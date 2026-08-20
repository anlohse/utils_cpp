/*
 * test_pool.cpp
 *
 *  Created on: 25/03/2010
 *      Author: alan.lohse
 */

//============================================================================
// Name        : teste.cpp
// Author      : Alan
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <cstdint>
#include <random>
#include <utils/time.hpp>
#include <utils/memory/memory_pool.hpp>
// #include <ext/pool_allocator.h>
#include <iostream>

using namespace std;
using namespace utils;
using namespace utils::memory;

struct malloc_malloc {
	void* malloc(size_t size) {
		return ::malloc(size);
	}
	void free(void* mem, size_t size) {
		::free(mem);
	}
	const char* get_name() {
		return "normal malloc";
	}
	void status() {

	}
};

struct pool_malloc {
	typedef memory_pool<null_mutex,null_growth_policy,new_malloc> memory_pool_type;
	void* malloc(size_t size) {
		return memory_pool_type::get_instance().malloc(size);
	}
	void free(void* mem, size_t size) {
		memory_pool_type::get_instance().free(mem,size);
	}
	const char* get_name() {
		return "memory_pool malloc";
	}
	void status() {
//		cout << "Total memory in pool: " << memory_pool_type::get_instance().get_total_memory() <<endl;
//		cout << "Memory free in pool: " << memory_pool_type::get_instance().get_free_memory() <<endl;
//		cout << "Memory used: " << memory_pool_type::get_instance().get_used_memory() <<endl;
	}
};

/*
struct gnu_pool_malloc {
	__gnu_cxx::__pool_alloc<char> pool;
	void* malloc(size_t size) {
		return pool.allocate(size);
	}
	void free(void* mem, size_t size) {
		pool.deallocate((char*)mem,size);
	}
	const char* get_name() {
		return "gnu pool allocator";
	}
	void status() {

	}
};
*/
struct std_allocator_malloc {
	std::allocator<char> pool;
	void* malloc(size_t size) {
		return pool.allocate(size);
	}
	void free(void* mem, size_t size) {
		pool.deallocate((char*)mem,size);
	}
	const char* get_name() {
		return "std::allocator allocator";
	}
	void status() {

	}
};

#define COUNT 1000000
#define MAX_ALLOC_SIZE 256

struct data_t {
	void* mem;
	size_t size;
};

typedef std::linear_congruential_engine<std::uint_fast32_t,34567,76543,0x7fffffff> random_type;

template <typename _Malloc>
void test_malloc() {
	_Malloc mallocfn;
	random_type rnd(static_cast<random_type::result_type>(Time::nanoseconds()));
	data_t* testdata = (data_t*)mallocfn.malloc(COUNT*sizeof(data_t));

	cout << "Testing " << mallocfn.get_name() << endl;
	data_t* curdata = testdata;
	data_t* end_of_data = testdata+COUNT;
	t_bigint start,end;
	cout << "  malloc in ";
	start = Time::nanoseconds();
	for (; curdata < end_of_data; curdata++) {
		size_t sz = rnd()%MAX_ALLOC_SIZE + 4;
		curdata->size = sz;
		curdata->mem = mallocfn.malloc(sz);
	}
	end = Time::nanoseconds();
	cout << (end-start) << " ns" << endl;

	cout << "  freeing half to cause fragmentation in ";
	start = Time::nanoseconds();
	for (curdata = testdata; curdata < end_of_data; curdata+=2) {
		 mallocfn.free(curdata->mem,curdata->size);
	}
	end = Time::nanoseconds();
	cout << (end-start) << " ns" << endl;

	cout << "  malloc with double size in ";
	start = Time::nanoseconds();
	for (curdata = testdata; curdata < end_of_data; curdata+=2) {
		size_t sz = (rnd()%MAX_ALLOC_SIZE + 4)*2;
		curdata->size = sz;
		curdata->mem = mallocfn.malloc(sz);
	}
	end = Time::nanoseconds();
	cout << (end-start) << " ns" << endl;

	cout << "  freeing half to cause fragmentation in ";
	start = Time::nanoseconds();
	for (curdata = testdata; curdata < end_of_data; curdata+=2) {
		 mallocfn.free(curdata->mem,curdata->size);
	}
	end = Time::nanoseconds();
	cout << (end-start) << " ns" << endl;

	cout << "  malloc with double size in ";
	start = Time::nanoseconds();
	for (curdata = testdata; curdata < end_of_data; curdata+=2) {
		size_t sz = (rnd()%MAX_ALLOC_SIZE + 4);
		curdata->size = sz;
		curdata->mem = mallocfn.malloc(sz);
	}
	end = Time::nanoseconds();
	cout << (end-start) << " ns" << endl;

	mallocfn.status();
	cout << "  freeing all in ";
	start = Time::nanoseconds();
	for (curdata = testdata; curdata < end_of_data; curdata++) {
		 mallocfn.free(curdata->mem,curdata->size);
	}
	end = Time::nanoseconds();
	cout << (end-start) << " ns" << endl;

	mallocfn.status();
	mallocfn.free(testdata,COUNT*sizeof(data_t));
}

template <typename _Malloc>
void test_malloc_complete() {
	t_bigint start,end;
	start = Time::nanoseconds();
	test_malloc<_Malloc>();
	end = Time::nanoseconds();
	cout << "  Total time: " << (end-start) << " ns" << endl;
	cout << endl;
}

void test_memory_pool() {
	test_malloc_complete<malloc_malloc>();
	test_malloc_complete<pool_malloc>();
	//test_malloc_complete<gnu_pool_malloc>();
	test_malloc_complete<std_allocator_malloc>();
}


