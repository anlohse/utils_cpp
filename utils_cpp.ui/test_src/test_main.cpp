/*
 * test_main.cpp
 *
 * Entry point for the UI demo. test_ui() had no caller, so the target could
 * never link.
 */

#include <iostream>

void test_ui();

int main() {
	try {
		test_ui();
	} catch (const std::exception& exc) {
		std::cerr << "test_ui failed: " << exc.what() << std::endl;
		return 1;
	}
	return 0;
}
