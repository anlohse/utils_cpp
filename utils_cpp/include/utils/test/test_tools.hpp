/*
 * test_tools.hpp
 *
 *  Created on: 30/03/2010
 *      Author: alan.lohse
 */

#ifndef TEST_TOOLS_HPP_
#define TEST_TOOLS_HPP_

#include <utils/utils_defs.hpp>
#include <utils/time.hpp>
#include <utils/exception.hpp>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace utils {

namespace test {

class TestCase;

/**
 * One registered test method.
 *
 * The callable is a std::function taking the owning TestCase. It replaces a
 * hand-rolled bound_func hierarchy whose registration macro C-cast member
 * function pointers between unrelated classes -- undefined behaviour, and
 * outright broken on implementations where member pointer size varies with the
 * class layout.
 */
struct test_function {
	std::function<void(TestCase*)> address;
	const char* name;
	const char* exception_message;
	test_function(std::function<void(TestCase*)> _address, const char* _name, const char* _exception_message) :
		address(std::move(_address)),
		name(_name),
		exception_message(_exception_message) {
	}
};

/**
 * Registers a member function as a test method.
 * @param cl the TestCase subclass
 * @param fn the qualified member function name, e.g. Test_Hex::test_encode
 */
// The static_cast to void (cl::*)() picks the no-argument overload where a
// test class also declares a same-named helper taking parameters (Test_Base32
// and Test_Base64 both do). It is a checked conversion between compatible
// member pointer types, unlike the C-cast this replaces.
#define ADD_TEST_FUNCTION(cl,fn) \
	tests.push_back(utils::test::test_function( \
		[](utils::test::TestCase* _self) { (static_cast<cl*>(_self)->*static_cast<void (cl::*)()>(&fn))(); }, \
		#fn, nullptr))

/** As ADD_TEST_FUNCTION, but the test passes only if it throws with `excp`. */
#define ADD_TEST_FUNCTION_EX(cl,fn,excp) \
	tests.push_back(utils::test::test_function( \
		[](utils::test::TestCase* _self) { (static_cast<cl*>(_self)->*static_cast<void (cl::*)()>(&fn))(); }, \
		#fn, (const char*)excp))

class test_exception : public utils_exception {
public:
	test_exception() noexcept :
		utils_exception()
	{
	}
	test_exception(const std::string& cause) noexcept :
		utils_exception(cause)
	{
	}
};

#define ASSERT2(value,expected) assert_test(value == expected, "expected " #expected " but found " #value)
#define ASSERT(value,message) assert_test(value, message)
#define ASSERT3(value) assert_test(value, #value)

class TestCase {
protected:
	std::vector<test_function> tests;
	friend class TestSuit;
	void assert_test(bool tested, const std::string& message) {
		if (!tested) throw test_exception(message);
	}
public:
	virtual ~TestCase() {
	}
	virtual void prepare_test() = 0;
	virtual void close_test() = 0;
};

struct test_case {
	// shared_ptr rather than a raw pointer: test_case is copied into the
	// registry by value, and the old raw pointer was never deleted.
	std::shared_ptr<TestCase> tcase;
	const char* name;
	test_case(std::shared_ptr<TestCase> _tcase, const char* _name) :
		tcase(std::move(_tcase)),
		name(_name) {
	}
};

#define ADD_TEST_CASE(tc) \
	utils::test::TestSuit::add_test(utils::test::test_case(std::make_shared<tc>(), #tc))

class TestSuit {
private:
	static std::vector<test_case>& _tests() {
		// A function-local static, not a leaked heap vector. Initialisation is
		// thread-safe and the registry is destroyed at exit.
		static std::vector<test_case> tests;
		return tests;
	}
public:
	static void add_test(const test_case& tc) {
		_tests().push_back(tc);
	}

	/** @return the number of failed test methods. */
	static int run_tests() {
		int total_failed = 0;
		for (std::vector<test_case>::iterator it = _tests().begin(), end = _tests().end();
				it != end; ++it) {
			std::cout << "Running test case " << it->name << std::endl;
			int success_count = 0, failed_count = 0;
			const t_bigint start_all = Time::milliseconds();
			for (std::vector<test_function>::iterator it2 = it->tcase->tests.begin(), end2 = it->tcase->tests.end();
					it2 != end2; ++it2) {
				std::cout << "Test method: " << it2->name << " ... ";
				it->tcase->prepare_test();
				bool success = it2->exception_message == nullptr;
				std::string message = "ok";
				// t_bigint, not long: Time::nanoseconds() overflows a 32-bit long.
				const t_bigint start = Time::nanoseconds();
				try {
					it2->address(it->tcase.get());
				} catch(std::exception& exc) {
					success = it2->exception_message != nullptr && strcmp(it2->exception_message,exc.what()) == 0;
					message = exc.what();
				}
				const t_bigint finish = Time::nanoseconds();
				it->tcase->close_test();
				if (success) {
					std::cout << "SUCCESS";
					success_count++;
				} else {
					std::cout << "FAILURE: " << message;
					failed_count++;
				}
				std::cout << " in " << (finish-start) << " ns" << std::endl;
			}
			const t_bigint end_all = Time::milliseconds();
			std::cout << "Tests done: " << it->tcase->tests.size() << " in " << (end_all - start_all) << " ms " << std::endl;
			std::cout << "Successes: " << success_count << std::endl;
			std::cout << "Failures: " << failed_count << std::endl;
			std::cout << std::endl;
			total_failed += failed_count;
		}
		return total_failed;
	}
};

}

}

#endif /* TEST_TOOLS_HPP_ */
