# utils_cpp

A C++20 utility library: arbitrary-precision arithmetic, cryptographic hashes,
codecs, geometry, signal transforms, sockets, logging, an XML parser, and a
retained-mode Win32 UI framework.

Three libraries, usable independently:

| Target       | Contents                                                                |
| ------------ | ----------------------------------------------------------------------- |
| `utils::cpp` | Core: math, crypt, codec, net, log, io, memory, test harness             |
| `utils::ui`  | Retained-mode UI framework with a Win32/GDI backend                     |
| `utils::xml` | SAX-style XML parser and object binding                                 |

## Requirements

- A C++20 compiler — MSVC 19.30+, GCC 11+, or Clang 14+
- CMake 3.20+

`utils::ui` is Windows-only today. On other platforms configure with
`-DUTILS_BUILD_UI=OFF`.

## Build

```bash
cmake --preset msvc && cmake --build --preset msvc && ctest --preset msvc
```

Available presets: `msvc`, `msvc-strict` (warnings as errors), `msvc-asan`,
`mingw`, `mingw-strict`, `linux`, `linux-asan`.

Without presets:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
```

### Options

| Option               | Default | Effect                                    |
| -------------------- | ------- | ----------------------------------------- |
| `UTILS_BUILD_UI`     | `ON`    | Build `utils::ui` (requires Windows)      |
| `UTILS_BUILD_XML`    | `ON`    | Build `utils::xml`                        |
| `UTILS_BUILD_TESTS`  | `ON`    | Build the test executables                |
| `UTILS_WERROR`       | `OFF`   | Treat warnings as errors                  |
| `UTILS_SANITIZE`     | `OFF`   | AddressSanitizer (+UBSan on GCC/Clang)    |

## Install and consume

```bash
cmake --install build --prefix /your/prefix
```

Then from another project:

```cmake
find_package(utils_cpp REQUIRED)
target_link_libraries(myapp PRIVATE utils::cpp utils::xml utils::ui)
```

```cpp
#include <utils/math/giantint.hpp>
#include <iostream>

int main() {
    utils::math::giantint a("123456789012345678901234567890");
    utils::math::giantint b("987654321098765432109876543210");
    std::cout << (a * b).to_string() << "\n";
}
```

## What's in it

**Math** — `giantint` (arbitrary-precision integers), `decimal` (fixed-point
with scale and rounding modes), fixed-size `vector`/`matrix` geometry,
`errored_number` (value with error propagation), FFT and interpolation,
statistics, fuzzy logic, a perceptron.

**Crypt & codec** — MD5, SHA-1, SHA-256/384/512; Hex, Base32, Base64.

**Net** — sockets with an iostream interface, URL parsing, the beginnings of
HTTP.

**Log** — logger, appenders, pattern layout.

**Memory** — `memory_pool`, a size-classed pool allocator, plus
`pooled_allocator` which adapts it to the standard allocator interface.

**UI** — `Component`/`Container`/`Graphics`/`Event`/`UIController`, backed by
Win32 and GDI. Incomplete: see below.

**XML** — SAX-style parser with entity and CDATA handling.

## Testing

```bash
ctest --preset msvc --output-on-failure
```

19 test cases, 66 test methods across the core library, plus the XML parser
suite. Both executables return the number of failures as their exit code.

## Notes for existing code

This library was recently modernised, and several subsystems that predated the
standard equivalents were removed rather than repaired. If you have code
against the older version:

| Removed                       | Use instead                                   |
| ----------------------------- | --------------------------------------------- |
| `utils/thread/*`              | `<thread>`, `<mutex>`, `<atomic>`             |
| `utils/containers/*`          | `<vector>`, `<list>`, `<unordered_map>`, ...  |
| `utils/functions/bound_funcs` | `<functional>`                                |
| `utils/random/*`              | `<random>`                                    |
| `Wrapper`, `SmartPointer`     | `std::optional`, `std::shared_ptr`            |
| `MAX(a,b)` / `MIN(a,b)`       | `std::max` / `std::min`                       |

The `t_*` typedefs in `utils_defs.hpp` are now fixed-width aliases of
`<cstdint>`. `t_int` and `t_dword` previously expanded to `long int` and
`unsigned long int`, which are 64 bits on LP64 platforms — code that relied on
them being 32 bits was silently wrong off Windows.

## Known gaps

- **The UI is unfinished.** The Win32 backend draws and dispatches events, but
  layout is a stub (`BasicLayout::apply` does nothing), and the widget set is
  limited to `Button`. There is no non-Windows backend.
- **Reference counting in the UI is manual** (`add_reference`/`rem_reference`
  and the `SAFE_DELETE` macro). It is now atomic, but ownership is not RAII.
- **`http_request` is a data structure without a parser** — the class is empty.
- The build emits conversion warnings (`C4244`/`C4242`) from the math and codec
  code where narrowing is intentional but not spelled out with explicit casts.

## Licence

See [LICENSE](LICENSE).
