/*
 * test_ref.cpp
 *
 * Tests for Ref<T>, the owning handle for intrusively counted UI objects.
 *
 * These use a local counted type rather than a real UIObject so destruction
 * can be observed directly, and so the tests do not depend on the UI
 * allocator being initialised.
 */

#include <utils/ui/ref.hpp>
#include <cstdio>
#include <utility>

using namespace utils::ui;

namespace {

int failures = 0;

void check(const char* what, bool ok, const char* detail) {
	std::printf("  %-48s %s  %s\n", what, ok ? "[ok]    " : "[FAIL]  ", detail);
	if (!ok) ++failures;
}

void checkInt(const char* what, int got, int expected) {
	char buf[96];
	if (got == expected)
		std::snprintf(buf, sizeof buf, "%d", got);
	else
		std::snprintf(buf, sizeof buf, "expected %d, got %d", expected, got);
	check(what, got == expected, buf);
}

/** A minimal intrusively counted type that records its own destruction. */
struct Counted {
	static int live;
	int refs;
	Counted() : refs(0) { ++live; }
	~Counted() { --live; }
	Counted* add_reference() { ++refs; return this; }
	Counted* rem_reference() { --refs; return this; }
	int get_references() const { return refs; }
};
int Counted::live = 0;

struct Derived : Counted { };

// ---------------------------------------------------------------------------

void testRetainRelease() {
	std::printf("\n Ref: construction retains, destruction releases\n");
	Counted::live = 0;
	{
		Ref<Counted> r(new Counted());
		checkInt("one reference after construction", r.use_count(), 1);
		checkInt("object is live", Counted::live, 1);
	}
	checkInt("destroyed when the handle goes", Counted::live, 0);
}

void testCopy() {
	std::printf("\n Ref: copying shares ownership\n");
	Counted::live = 0;
	{
		Ref<Counted> a(new Counted());
		{
			Ref<Counted> b = a;
			checkInt("two references after copy", a.use_count(), 2);
			check("both name the same object", a.get() == b.get(), "same pointer");
		}
		checkInt("back to one when the copy dies", a.use_count(), 1);
		checkInt("object still live", Counted::live, 1);
	}
	checkInt("destroyed when the last handle goes", Counted::live, 0);
}

void testMove() {
	std::printf("\n Ref: moving transfers without touching the count\n");
	Counted::live = 0;
	{
		Ref<Counted> a(new Counted());
		Ref<Counted> b = std::move(a);
		checkInt("count unchanged by the move", b.use_count(), 1);
		check("source is empty", a.get() == nullptr, "moved-from is null");
		checkInt("object still live", Counted::live, 1);
	}
	checkInt("destroyed once", Counted::live, 0);
}

void testSelfAssignment() {
	std::printf("\n Ref: self-assignment does not destroy the object\n");
	Counted::live = 0;
	{
		Ref<Counted> a(new Counted());
		// The naive release-then-retain order drops the last reference here
		// and leaves the handle pointing at freed memory.
		a = a;
		checkInt("still exactly one reference", a.use_count(), 1);
		checkInt("still live", Counted::live, 1);
	}
	checkInt("destroyed once", Counted::live, 0);
}

void testAliasAssignment() {
	std::printf("\n Ref: assigning an alias of the same object is safe\n");
	Counted::live = 0;
	{
		Ref<Counted> a(new Counted());
		Ref<Counted> b = a;          // count 2
		a = b;                       // must stay at 2, not dip to 0
		checkInt("count unchanged", a.use_count(), 2);
		checkInt("still live", Counted::live, 1);
	}
	checkInt("destroyed once", Counted::live, 0);
}

void testReplacement() {
	std::printf("\n Ref: assignment releases what it replaces\n");
	Counted::live = 0;
	{
		Ref<Counted> r(new Counted());
		checkInt("one object live", Counted::live, 1);
		r = new Counted();           // the first must be destroyed here
		checkInt("first released on replacement", Counted::live, 1);
		checkInt("new object has one reference", r.use_count(), 1);
	}
	checkInt("second destroyed too", Counted::live, 0);
}

void testTwoRefsOverOneRawPointer() {
	std::printf("\n Ref: two handles over one raw pointer (intrusive count)\n");
	Counted::live = 0;
	{
		Counted* raw = new Counted();
		Ref<Counted> a(raw);
		Ref<Counted> b(raw);
		// An intrusive count makes this correct, where shared_ptr would build
		// two control blocks and double-free.
		checkInt("both references counted", a.use_count(), 2);
		checkInt("object live", Counted::live, 1);
	}
	checkInt("destroyed exactly once", Counted::live, 0);
}

void testNullHandling() {
	std::printf("\n Ref: null is a valid, inert state\n");
	Counted::live = 0;
	Ref<Counted> r;
	check("default constructs empty", r.get() == nullptr, "null");
	check("converts to false", !static_cast<bool>(r), "falsy");
	checkInt("use_count of null is zero", r.use_count(), 0);
	r = nullptr;                  // must not crash
	r.reset();                    // must not crash
	check("still empty", r.get() == nullptr, "null");
}

void testReset() {
	std::printf("\n Ref: reset and detach\n");
	Counted::live = 0;
	{
		Ref<Counted> r(new Counted());
		r.reset();
		checkInt("reset destroys the object", Counted::live, 0);
	}
	{
		Ref<Counted> r(new Counted());
		Counted* raw = r.detach();
		check("detach empties the handle", r.get() == nullptr, "null");
		checkInt("object survives detach", Counted::live, 1);
		checkInt("caller holds the reference", raw->get_references(), 1);
		// The caller now owns it, exactly as before Ref existed.
		if (raw->rem_reference()->get_references() < 1) delete raw;
		checkInt("destroyed by the caller", Counted::live, 0);
	}
}

void testConvertingConstruction() {
	std::printf("\n Ref: a Ref<Derived> initialises a Ref<Base>\n");
	Counted::live = 0;
	{
		Ref<Derived> d(new Derived());
		Ref<Counted> b = d;
		checkInt("two references", d.use_count(), 2);
		check("same object", b.get() == d.get(), "same pointer");
	}
	checkInt("destroyed once", Counted::live, 0);
}

void testContainerUse() {
	std::printf("\n Ref: survives being stored and copied in bulk\n");
	Counted::live = 0;
	{
		Ref<Counted> src(new Counted());
		Ref<Counted> stack[8];
		for (int i = 0; i < 8; ++i)
			stack[i] = src;
		checkInt("nine references", src.use_count(), 9);
		for (int i = 0; i < 8; ++i)
			stack[i] = nullptr;
		checkInt("back to one", src.use_count(), 1);
		checkInt("still live", Counted::live, 1);
	}
	checkInt("destroyed once", Counted::live, 0);
}

} // namespace

int test_ref() {
	std::printf("--- Ref<T> ownership handle ---\n");
	failures = 0;

	testRetainRelease();
	testCopy();
	testMove();
	testSelfAssignment();
	testAliasAssignment();
	testReplacement();
	testTwoRefsOverOneRawPointer();
	testNullHandling();
	testReset();
	testConvertingConstruction();
	testContainerUse();

	std::printf("\n  %d Ref failure(s).\n", failures);
	return failures;
}
