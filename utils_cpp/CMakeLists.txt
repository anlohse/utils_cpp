# ---------------------------------------------------------------------------
# utils_cpp -- the core library
# ---------------------------------------------------------------------------

# NOTE: globbing is deliberate while the library is being reworked; files are
# being added and removed continuously. Switch to explicit lists once the
# refactor settles. CONFIGURE_DEPENDS makes the build re-glob on change.
file(GLOB UTILS_CPP_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

if(WIN32)
    file(GLOB UTILS_CPP_PLATFORM_SOURCES CONFIGURE_DEPENDS "windows/*.cpp")
else()
    file(GLOB UTILS_CPP_PLATFORM_SOURCES CONFIGURE_DEPENDS "linux/*.cpp")
endif()

file(GLOB_RECURSE UTILS_CPP_HEADERS CONFIGURE_DEPENDS "include/*.hpp" "include/*.inc")

add_library(utils_cpp STATIC
    ${UTILS_CPP_SOURCES}
    ${UTILS_CPP_PLATFORM_SOURCES}
    ${UTILS_CPP_HEADERS}
)
add_library(utils::cpp ALIAS utils_cpp)

target_include_directories(utils_cpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(utils_cpp PUBLIC utils::compile_options)

if(WIN32)
    # ws2_32: Winsock. The net/ subtree calls into it directly.
    target_link_libraries(utils_cpp PUBLIC ws2_32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(utils_cpp PUBLIC Threads::Threads)
endif()

# EXPORT_NAME so the installed target is utils::cpp, matching the ALIAS used in
# the build tree. Without it the export namespace would yield utils::utils_cpp.
set_target_properties(utils_cpp PROPERTIES EXPORT_NAME cpp)

install(TARGETS utils_cpp
        EXPORT utils_cppTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Headers listed in the target for IDE grouping only -- they are not compiled.
set_source_files_properties(${UTILS_CPP_HEADERS} PROPERTIES HEADER_FILE_ONLY ON)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
    ${UTILS_CPP_SOURCES} ${UTILS_CPP_PLATFORM_SOURCES} ${UTILS_CPP_HEADERS})

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(UTILS_BUILD_TESTS)
    file(GLOB UTILS_CPP_TEST_SOURCES CONFIGURE_DEPENDS "test_src/*.cpp")

    add_executable(utils_cpp_tests ${UTILS_CPP_TEST_SOURCES})
    target_link_libraries(utils_cpp_tests PRIVATE utils::cpp)

    add_test(NAME utils_cpp_tests COMMAND utils_cpp_tests)
endif()
