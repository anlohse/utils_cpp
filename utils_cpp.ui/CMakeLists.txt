# ---------------------------------------------------------------------------
# utils_ui -- the UI framework
#
# The platform backend lives in its own directory and is selected here rather
# than by #ifdef inside the sources. Keeping that seam at the build level is
# what makes a future X11/Cocoa backend a matter of adding a directory.
# ---------------------------------------------------------------------------

file(GLOB UTILS_UI_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

if(WIN32)
    file(GLOB UTILS_UI_BACKEND_SOURCES CONFIGURE_DEPENDS "windows/*.cpp")
    file(GLOB UTILS_UI_BACKEND_HEADERS CONFIGURE_DEPENDS "windows/*.h")
else()
    message(FATAL_ERROR
        "utils_cpp.ui currently has a Win32 backend only. "
        "Configure with -DUTILS_BUILD_UI=OFF on this platform.")
endif()

file(GLOB_RECURSE UTILS_UI_HEADERS CONFIGURE_DEPENDS "include/*.hpp")

add_library(utils_ui STATIC
    ${UTILS_UI_SOURCES}
    ${UTILS_UI_BACKEND_SOURCES}
    ${UTILS_UI_BACKEND_HEADERS}
    ${UTILS_UI_HEADERS}
)
add_library(utils::ui ALIAS utils_ui)

target_include_directories(utils_ui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        # Backend-private headers (gdi_graphics.h, image_w32.h, ...) must not
        # leak into the public interface.
        $<$<BOOL:${WIN32}>:${CMAKE_CURRENT_SOURCE_DIR}/windows>
)

target_link_libraries(utils_ui PUBLIC utils::cpp)

if(WIN32)
    target_link_libraries(utils_ui PRIVATE
        user32      # windows, messages, cursors
        gdi32       # the GDI drawing backend
        comctl32    # common controls
        shell32
    )
endif()

set_target_properties(utils_ui PROPERTIES EXPORT_NAME ui)

install(TARGETS utils_ui
        EXPORT utils_cppTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

set_source_files_properties(${UTILS_UI_HEADERS} PROPERTIES HEADER_FILE_ONLY ON)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
    ${UTILS_UI_SOURCES} ${UTILS_UI_BACKEND_SOURCES}
    ${UTILS_UI_BACKEND_HEADERS} ${UTILS_UI_HEADERS})

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(UTILS_BUILD_TESTS)
    file(GLOB UTILS_UI_TEST_SOURCES CONFIGURE_DEPENDS "test_src/*.cpp")

    if(UTILS_UI_TEST_SOURCES)
        # A console main(), not WinMain: the demo prints diagnostics to stdout.
        add_executable(utils_ui_tests ${UTILS_UI_TEST_SOURCES})
        target_link_libraries(utils_ui_tests PRIVATE utils::ui)
    endif()
endif()
