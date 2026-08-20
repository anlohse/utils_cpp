# ---------------------------------------------------------------------------
# utils_compile_options
#
# One INTERFACE target carrying the warning set and language conformance flags
# that every target in this project links against. Keeping it here rather than
# in each CMakeLists means MSVC and GCC/MinGW stay in sync as flags evolve.
# ---------------------------------------------------------------------------

add_library(utils_compile_options INTERFACE)
add_library(utils::compile_options ALIAS utils_compile_options)

if(MSVC)
    target_compile_options(utils_compile_options INTERFACE
        /W4                 # high warning level
        /permissive-        # standard conformance; disables MS extensions
        /Zc:__cplusplus     # report the real __cplusplus value, not 199711
        /Zc:preprocessor    # conforming preprocessor
        /Zc:inline          # drop unreferenced COMDATs
        /Zc:throwingNew     # operator new never returns null
        /EHsc               # standard C++ exception model
        /utf-8              # source and execution charset
        /bigobj             # heavily-templated TUs exceed the default section limit
        /diagnostics:caret  # point at the offending column
    )

    # Warnings that matter for this codebase and are off by default at /W4.
    target_compile_options(utils_compile_options INTERFACE
        /w14242 /w14254 /w14263 /w14265 /w14287 /we4289 /w14296
        /w14311 /w14545 /w14546 /w14547 /w14549 /w14555 /w14619
        /w14640 /w14826 /w14905 /w14906 /w14928
    )
    #  4242/4254 lossy conversion            4263/4265 virtual override / non-virtual dtor
    #  4287/4289 unsigned mismatch, loop var  4296 always-true comparison
    #  4311      pointer truncation           4545-4555 malformed expressions
    #  4619      bogus #pragma warning        4640 non-thread-safe local static
    #  4826      sign-extending cast          4905/4906 string cast  4928 double conversion

    target_compile_definitions(utils_compile_options INTERFACE
        WIN32_LEAN_AND_MEAN
        NOMINMAX            # windows.h min/max macros collide with std::min/max
        UNICODE _UNICODE
    )

    if(UTILS_WERROR)
        target_compile_options(utils_compile_options INTERFACE /WX)
    endif()

else()
    # GCC (MinGW-w64) and Clang
    target_compile_options(utils_compile_options INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow                # locals shadowing members bite hard in this codebase
        -Wnon-virtual-dtor      # base classes here are deleted through base pointers
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
    )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(utils_compile_options INTERFACE
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
            -Wuseless-cast
        )
    endif()

    if(WIN32)
        target_compile_definitions(utils_compile_options INTERFACE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
        )
        # MinGW needs an explicit Windows version floor for the Win32 UI backend.
        target_compile_definitions(utils_compile_options INTERFACE _WIN32_WINNT=0x0601)
    endif()

    if(UTILS_WERROR)
        target_compile_options(utils_compile_options INTERFACE -Werror)
    endif()

    if(UTILS_SANITIZE)
        # MinGW has no ASan runtime; this is for Clang and for Linux builds.
        if(NOT MINGW)
            target_compile_options(utils_compile_options INTERFACE
                -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(utils_compile_options INTERFACE
                -fsanitize=address,undefined)
        else()
            message(WARNING "UTILS_SANITIZE requested but MinGW has no sanitizer runtime; ignoring.")
        endif()
    endif()
endif()

if(MSVC AND UTILS_SANITIZE)
    # MSVC ships AddressSanitizer; it is incompatible with /RTC and edit-and-continue.
    target_compile_options(utils_compile_options INTERFACE /fsanitize=address)
endif()

# ---------------------------------------------------------------------------
# Platform identification
#
# config.hpp currently derives this from _WIN32/_LINUX itself. Defining it here
# too means the build system is the single source of truth once config.hpp is
# cleaned up, and it makes cross-compiling explicit rather than accidental.
# ---------------------------------------------------------------------------
if(WIN32)
    target_compile_definitions(utils_compile_options INTERFACE UTILS_WINDOWS)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(utils_compile_options INTERFACE UTILS_LINUX _LINUX)
elseif(APPLE)
    target_compile_definitions(utils_compile_options INTERFACE UTILS_MACOS)
endif()
