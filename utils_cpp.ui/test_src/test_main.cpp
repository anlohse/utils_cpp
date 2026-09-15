/*
 * test_main.cpp
 *
 * Entry point for the UI tests.
 *
 * By default this runs only the headless checks, so the target is usable from
 * ctest. Pass --window to also open the interactive demo, which needs a
 * desktop and does not return until the window closes.
 *
 * The exit code is the number of failed checks.
 */

#include <cstring>
#include <iostream>

int  test_graphics_backends();
int  test_layout();
void test_ui();
void demo_window();

int main(int argc, char** argv) {
	bool window = false;
	for (int i = 1; i < argc; ++i)
		if (std::strcmp(argv[i], "--window") == 0)
			window = true;

	int failures = 0;
	try {
		failures += test_graphics_backends();
		std::cout << std::endl;
		failures += test_layout();
		if (window)
			demo_window();
	} catch (const std::exception& exc) {
		std::cerr << "ui tests failed: " << exc.what() << std::endl;
		return 1;
	}

	std::cout << std::endl << (failures == 0 ? "ALL UI TESTS PASSED" : "UI TESTS FAILED")
	          << " (" << failures << " failure(s))" << std::endl;
	return failures;
}
