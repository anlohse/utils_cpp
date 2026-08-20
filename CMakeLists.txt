cmake_minimum_required(VERSION 3.20)

project(utils_cpp
        VERSION 1.0.0
        DESCRIPTION "A utility library for C++"
        LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Language level
#
# The library targets C++20. This is deliberate: concepts, ranges, <span> and
# constexpr algorithms are what let the legacy hand-rolled machinery in this
# codebase be replaced with something both smaller and safer.
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Export compile_commands.json for clangd / clang-tidy / IDE tooling.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(UTILS_BUILD_UI      "Build the utils_cpp.ui UI library" ON)
option(UTILS_BUILD_XML     "Build the utils_xml library"       ON)
option(UTILS_BUILD_TESTS   "Build the test executables"        ON)
option(UTILS_WERROR        "Treat warnings as errors"          OFF)
option(UTILS_SANITIZE      "Enable address/undefined sanitizers where supported" OFF)

include(cmake/UtilsCompileOptions.cmake)

# ---------------------------------------------------------------------------
# Subprojects
#
# enable_testing() must come before add_subdirectory(): add_test() calls in a
# subdirectory are ignored unless testing was already enabled at this point.
# ---------------------------------------------------------------------------
if(UTILS_BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(utils_cpp)

if(UTILS_BUILD_UI)
    add_subdirectory(utils_cpp.ui)
endif()

if(UTILS_BUILD_XML)
    add_subdirectory(utils_xml)
endif()

# ---------------------------------------------------------------------------
# Install / export
#
# Makes the library consumable from another project with:
#     find_package(utils_cpp REQUIRED)
#     target_link_libraries(myapp PRIVATE utils::cpp utils::ui)
# ---------------------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# utils_compile_options is linked PUBLIC by every target, so it has to travel
# with them in the export set.
set_target_properties(utils_compile_options PROPERTIES EXPORT_NAME compile_options)
install(TARGETS utils_compile_options EXPORT utils_cppTargets)

install(DIRECTORY utils_cpp/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inc")

if(UTILS_BUILD_UI)
    install(DIRECTORY utils_cpp.ui/include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.hpp")
endif()

if(UTILS_BUILD_XML)
    install(DIRECTORY utils_xml/include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.hpp")
endif()

install(EXPORT utils_cppTargets
        FILE utils_cppTargets.cmake
        NAMESPACE utils::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/utils_cpp)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/utils_cppConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/utils_cppConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/utils_cpp)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/utils_cppConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/utils_cppConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/utils_cppConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/utils_cpp)

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
message(STATUS "")
message(STATUS "utils_cpp ${PROJECT_VERSION}")
message(STATUS "  compiler    : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  standard    : C++${CMAKE_CXX_STANDARD}")
message(STATUS "  build type  : ${CMAKE_BUILD_TYPE}")
message(STATUS "  ui          : ${UTILS_BUILD_UI}")
message(STATUS "  xml         : ${UTILS_BUILD_XML}")
message(STATUS "  tests       : ${UTILS_BUILD_TESTS}")
message(STATUS "  werror      : ${UTILS_WERROR}")
message(STATUS "  sanitizers  : ${UTILS_SANITIZE}")
message(STATUS "")
