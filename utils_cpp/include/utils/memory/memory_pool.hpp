/*
 * memory_pool.hpp
 *
 *  Created on: 24/09/2009
 *      Author: alanlohse
 */

#ifndef MEMORY_POOL_HPP_
#define MEMORY_POOL_HPP_

#include <utils/utils_defs.hpp>
#include <algorithm>
#include <utils/funcs.hpp>
#include <utils/time.hpp>
#include <cstddef>
#include <memory>
#include <mutex>
#include <new>
#include <unordered_map>
#include <vector>

namespace utils {

namespace memory {

/**
 * Size bookkeeping and limits for memory_pool.
 *
 * get_fragmentation_slot_count() must equal
 * get_max_fragmented_size() - get_min_fragmented_size() + 1, because the pool
 * keeps one free list per exact byte size in that inclusive range. It
 * previously reported 4092 for the range [4, 4096], which is 4093 sizes, so an
 * allocation of exactly max_fragmented_size indexed one past the end of the
 * free-list array.
 */
struct default_growth_policy {
	size_t starter_memory_size;
	size_t min_block_size;
	size_t max_memory_size;
	size_t starter_fragmented_size;
	size_t total_size;
	size_t total_free;
	size_t total_alloc;
	default_growth_policy() {
		starter_memory_size = 262144;
		min_block_size = 262144;
		max_memory_size = 0;
		starter_fragmented_size = 4096;
		total_size = 0;
		total_free = 0;
		total_alloc = 0;
	}

	size_t get_min_fragmented_size() const {
		return 4;
	}
	size_t get_max_fragmented_size() const {
		return 4096;
	}
	size_t get_fragmentation_slot_count() const {
		return get_max_fragmented_size() - get_min_fragmented_size() + 1;
	}
	/** Bytes to request from the system when carving a new arena. */
	size_t get_arena_size() const {
		return 131072;
	}
	bool can_alloc(size_t size) const {
		return max_memory_size == 0 || max_memory_size >= total_size + size;
	}
	void add_mem(size_t size) {
		total_size += size;
	}
	void add_free_mem(size_t size) {
		total_free += size;
	}
	void sub_mem(size_t size) {
		total_size -= size;
	}
	void sub_free_mem(size_t size) {
		total_free -= size;
	}
	void add_alloc_mem(size_t size) {
		total_alloc += size;
	}
	void sub_alloc_mem(size_t size) {
		total_alloc -= size;
	}
	size_t get_total_size() const {
		return total_size;
	}
	size_t get_total_free() const {
		return total_free;
	}
	size_t get_total_alloc() const {
		return total_alloc;
	}
};

struct default_malloc {
	void* mem_alloc(size_t size) {
		return ::malloc(size);
	}
	void mem_free(void* mem) {
		return ::free(mem);
	}
};

/**
 * A size-classed pooling allocator.
 *
 * Allocations up to max_fragmented_size are served from per-size free lists
 * carved out of large arenas; larger ones get a free list per exact size.
 * Every arena is owned by the pool and released in the destructor.
 */
template<typename _MutexT = std::mutex, typename _GrowthPolicyT = default_growth_policy, typename _MallocT = default_malloc>
class memory_pool {
public:
	typedef memory_pool<_MutexT,_GrowthPolicyT,_MallocT> memory_pool_type;
	typedef _MutexT			mutex_type;
	typedef _GrowthPolicyT	growth_policy_type;
	typedef _MallocT		malloc_type;
private:

	struct memory_slot {
		memory_slot* next;
		memory_slot(memory_slot* _next): next(_next) {
		}
	};

	// Every slot is laid out as [header][payload]. The header is padded to the
	// platform's maximum fundamental alignment so the payload is correctly
	// aligned for any object the caller stores in it.
	static constexpr size_t k_align  = alignof(std::max_align_t);
	static constexpr size_t k_header = ((sizeof(memory_slot) + k_align - 1) / k_align) * k_align;

	static size_t align_up(size_t n) {
		return ((n + k_align - 1) / k_align) * k_align;
	}
	static void* slot_to_base(memory_slot* slot) {
		return reinterpret_cast<char*>(slot) + k_header;
	}
	static memory_slot* base_to_slot(void* base) {
		return reinterpret_cast<memory_slot*>(static_cast<char*>(base) - k_header);
	}

	/**
	 * One system block carved into fixed-stride slots.
	 *
	 * The stride is the padded [header][payload] size. The previous version
	 * stored memory_slot* and advanced with cur++, stepping sizeof(memory_slot)
	 * bytes -- 8 -- regardless of the payload size, so every slot overlapped
	 * its neighbours and the computed end pointer was far short of the block.
	 */
	struct arena {
		char* memory;
		size_t stride;
		size_t capacity;
		size_t used;
		arena(char* _memory, size_t _stride, size_t _capacity) :
			memory(_memory), stride(_stride), capacity(_capacity), used(0) {
		}
		bool has_next() const {
			return used < capacity;
		}
		memory_slot* next() {
			if (!has_next()) return nullptr;
			return reinterpret_cast<memory_slot*>(memory + (used++) * stride);
		}
	};

	struct slot_group {
		size_t size;
		slot_group* next;
		memory_slot* free_slots;
		slot_group(size_t _size, slot_group* _next, memory_slot* _free_slots):
			size(_size),
			next(_next),
			free_slots(_free_slots) {
		}
		memory_slot* pop_free() {
			if (free_slots) {
				memory_slot* slot = free_slots;
				free_slots = slot->next;
				slot->next = nullptr;
				return slot;
			}
			return nullptr;
		}
		void push_free(memory_slot* slot) {
			slot->next = free_slots;
			free_slots = slot;
		}
	};

	static const size_t k_map_buckets = 2027;

	struct memory_map {
		slot_group* groups[k_map_buckets];
		memory_map() {
			for (size_t i = 0; i < k_map_buckets; i++)
				groups[i] = nullptr;
		}
		~memory_map() {
			for (size_t i = 0; i < k_map_buckets; i++) {
				slot_group* cur = groups[i];
				while (cur) {
					slot_group* next = cur->next;
					delete cur;
					cur = next;
				}
				groups[i] = nullptr;
			}
		}
		memory_map(const memory_map&) = delete;
		memory_map& operator = (const memory_map&) = delete;

		void put(slot_group* group) {
			const size_t p = group->size % k_map_buckets;
			group->next = groups[p];
			groups[p] = group;
		}
		slot_group* get(size_t size) const {
			const size_t p = size % k_map_buckets;
			for (slot_group* cur = groups[p]; cur; cur = cur->next)
				if (cur->size == size)
					return cur;
			return nullptr;
		}
	};

	std::vector<memory_slot*> free_slots;   // one free list per exact small size
	memory_map big_slots;
	mutex_type mutex;
	growth_policy_type growth_policy;
	malloc_type mem_alloc;
	// Arenas, keyed by stride, so the destructor can hand every block back.
	std::unordered_map<size_t, std::vector<arena> > arenas;

	memory_pool() :
		free_slots(),
		big_slots(),
		mutex(),
		growth_policy(),
		mem_alloc(),
		arenas() {
		free_slots.assign(growth_policy.get_fragmentation_slot_count(), nullptr);
	}

	/** Rounds a request into its size class. */
	size_t size_class(size_t size) const {
		const size_t min_size = growth_policy.get_min_fragmented_size();
		return size < min_size ? min_size : size;
	}

	bool is_small(size_t size) const {
		return size <= growth_policy.get_max_fragmented_size();
	}

	size_t small_index(size_t size) const {
		return size - growth_policy.get_min_fragmented_size();
	}

	/** Carves a fresh slot of the given payload size out of an arena. */
	memory_slot* alloc_slot(size_t size) {
		const size_t stride = align_up(size + k_header);
		if (!growth_policy.can_alloc(stride))
			return nullptr;

		std::vector<arena>& list = arenas[stride];
		if (!list.empty() && list.back().has_next()) {
			growth_policy.add_mem(stride);
			return list.back().next();
		}

		const size_t per_arena = growth_policy.get_arena_size() / stride;
		const size_t capacity  = per_arena ? per_arena : 1;
		const size_t bytes     = capacity * stride;

		char* block = static_cast<char*>(mem_alloc.mem_alloc(bytes));
		if (!block)
			return nullptr;   // out of memory: report failure instead of writing through null

		list.push_back(arena(block, stride, capacity));
		growth_policy.add_mem(stride);
		return list.back().next();
	}

public:
	~memory_pool() {
		// Hand every arena back. The previous destructor was empty, so the pool
		// leaked every block it had ever requested.
		for (auto& entry : arenas) {
			for (arena& a : entry.second)
				mem_alloc.mem_free(a.memory);
			entry.second.clear();
		}
		arenas.clear();
	}

	memory_pool(const memory_pool&) = delete;
	memory_pool& operator = (const memory_pool&) = delete;

	/**
	 * @return a block of at least `size` bytes, or null if the pool cannot
	 *         satisfy the request.
	 */
	void* malloc(size_t size) {
		// Requests below the minimum class are rounded up rather than refused;
		// returning null for every allocation under 4 bytes made the pool
		// unusable as a general allocator.
		const size_t sz = size_class(size);
		std::lock_guard<mutex_type> lk(mutex);

		if (!growth_policy.can_alloc(sz))
			return nullptr;

		memory_slot* slot = nullptr;
		if (is_small(sz)) {
			const size_t p = small_index(sz);
			slot = free_slots[p];
			if (slot) {
				free_slots[p] = slot->next;
				slot->next = nullptr;
				growth_policy.sub_free_mem(sz);
			} else {
				slot = alloc_slot(sz);
			}
		} else {
			slot_group* group = big_slots.get(sz);
			if (!group) {
				group = new slot_group(sz, nullptr, nullptr);
				big_slots.put(group);
			}
			slot = group->pop_free();
			if (slot)
				growth_policy.sub_free_mem(sz);
			else
				slot = alloc_slot(sz);
		}

		if (!slot)
			return nullptr;
		growth_policy.add_alloc_mem(sz);
		return slot_to_base(slot);
	}

	/**
	 * Returns a block to the pool. `size` must be the size passed to malloc.
	 */
	void free(void* base, size_t size) {
		if (!base) return;
		const size_t sz = size_class(size);
		std::lock_guard<mutex_type> lk(mutex);

		memory_slot* slot = base_to_slot(base);
		if (is_small(sz)) {
			const size_t p = small_index(sz);
			slot->next = free_slots[p];
			free_slots[p] = slot;
		} else {
			slot_group* group = big_slots.get(sz);
			if (!group) {
				// Freeing a size the pool has no group for used to dereference
				// null. Create the group and keep the block for reuse.
				group = new slot_group(sz, nullptr, nullptr);
				big_slots.put(group);
			}
			group->push_free(slot);
		}
		growth_policy.sub_alloc_mem(sz);
		growth_policy.add_free_mem(sz);
	}

	size_t get_free_memory() {
		std::lock_guard<mutex_type> lk(mutex);
		return growth_policy.get_total_free();
	}

	size_t get_used_memory() {
		std::lock_guard<mutex_type> lk(mutex);
		return growth_policy.get_total_alloc();
	}

	size_t get_total_memory() {
		std::lock_guard<mutex_type> lk(mutex);
		return growth_policy.get_total_size();
	}

	static memory_pool_type& get_instance() {
		static memory_pool_type instance;
		return instance;
	}

};

/**
 * A std-conforming allocator backed by memory_pool.
 */
template<typename _Tp, typename _MutexT = std::mutex, typename _GrowthPolicyT = default_growth_policy, typename _MallocT = default_malloc>
class pooled_allocator {
	typedef memory_pool<_MutexT,_GrowthPolicyT,_MallocT> memory_pool_type;
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef _Tp* pointer;
	typedef const _Tp* const_pointer;
	typedef _Tp& reference;
	typedef const _Tp& const_reference;
	typedef _Tp value_type;

	typedef std::true_type is_always_equal;
	typedef std::true_type propagate_on_container_move_assignment;

	/**
	 * Rebinding must carry the mutex, growth and malloc policies across.
	 * Dropping them, as the previous definition did, produced an allocator
	 * bound to a *different* memory_pool singleton, so memory obtained from one
	 * pool was returned to another.
	 */
	template<typename _Tp1>
	struct rebind {
		typedef pooled_allocator<_Tp1,_MutexT,_GrowthPolicyT,_MallocT> other;
	};

	pooled_allocator() noexcept {
	}

	pooled_allocator(const pooled_allocator&) noexcept {
	}

	template<typename _Tp1>
	pooled_allocator(const pooled_allocator<_Tp1,_MutexT,_GrowthPolicyT,_MallocT>&) noexcept {
	}

	~pooled_allocator() noexcept {
	}

	pooled_allocator& operator = (const pooled_allocator&) noexcept {
		return *this;
	}

	pointer address(reference __x) const noexcept {
		return &__x;
	}

	const_pointer address(const_reference __x) const noexcept {
		return &__x;
	}

	pointer allocate(size_type __n) {
		if (__n > max_size())
			throw std::bad_alloc();
		void* mem = memory_pool_type::get_instance().malloc(__n * sizeof(_Tp));
		// An allocator must throw rather than return null; containers do not
		// check the result.
		if (!mem)
			throw std::bad_alloc();
		return static_cast<_Tp*> (mem);
	}

	void deallocate(pointer __p, size_type __n) {
		memory_pool_type::get_instance().free(__p,__n * sizeof(_Tp));
	}

	size_type max_size() const noexcept {
		return size_t(-1) / sizeof(_Tp);
	}

	template<typename _Up, typename... _Args>
	void construct(_Up* __p, _Args&&... __args) {
		::new (static_cast<void*>(__p)) _Up(std::forward<_Args>(__args)...);
	}

	template<typename _Up>
	void destroy(_Up* __p) {
		__p->~_Up();
	}

};

template<typename _Tp,  typename _MutexT, typename _GrowthPolicyT, typename _MallocT>
inline bool operator==(const pooled_allocator<_Tp,_MutexT,_GrowthPolicyT,_MallocT>&,
		const pooled_allocator<_Tp,_MutexT,_GrowthPolicyT,_MallocT>&) {
	return true;
}

template<typename _Tp, typename _MutexT, typename _GrowthPolicyT, typename _MallocT>
inline bool operator!=(const pooled_allocator<_Tp,_MutexT,_GrowthPolicyT,_MallocT>&,
		const pooled_allocator<_Tp,_MutexT,_GrowthPolicyT,_MallocT>&) {
	return false;
}

struct null_growth_policy {
	null_growth_policy() {
	}

	size_t get_min_fragmented_size() const {
		return 4;
	}
	size_t get_max_fragmented_size() const {
		return 4096;
	}
	size_t get_fragmentation_slot_count() const {
		return get_max_fragmented_size() - get_min_fragmented_size() + 1;
	}
	size_t get_arena_size() const {
		return 131072;
	}
	bool can_alloc(size_t) const {
		return true;
	}
	void add_mem(size_t) {
	}
	void add_free_mem(size_t) {
	}
	void sub_mem(size_t) {
	}
	void sub_free_mem(size_t) {
	}
	void add_alloc_mem(size_t) {
	}
	void sub_alloc_mem(size_t) {
	}
	size_t get_total_size() const {
		return 0;
	}
	size_t get_total_free() const {
		return 0;
	}
	size_t get_total_alloc() const {
		return 0;
	}
};

struct new_malloc {
	void* mem_alloc(size_t size) {
		return ::operator new(size, std::nothrow);
	}
	void mem_free(void* mem) {
		return ::operator delete(mem);
	}
};

}

}

#endif /* MEMORY_POOL_HPP_ */
