# utils_cpp

A C++20 utility library: arbitrary-precision arithmetic, cryptographic hashes,
codecs, geometry, signal transforms, sockets, logging, an XML parser, and a
retained-mode Win32 UI framework.

Three libraries, usable independently:

| Target       | Contents                                                                |
| ------------ | ----------------------------------------------------------------------- |
| `utils::cpp` | Core: math, crypt, codec, net, log, io, memory, test harness             |
| `utils::ui`  | Retained-mode UI framework, Canvas-2D drawing, GDI + GDI+ backends      |
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

**UI** — `Component`/`Container`/`Graphics`/`Event`/`UIController`. The
drawing API follows the HTML5 Canvas 2D context: `save`/`restore`, an affine
transform stack, `beginPath`/`moveTo`/`lineTo`/`bezierCurveTo`/`arc`/`arcTo`,
`fill`/`stroke`/`clip`, gradients, patterns, `globalAlpha`, and the three
`drawImage` overloads.

Two backends are selectable at runtime:

```cpp
Graphics::setGraphicsImplementation(GRAPHICS_GDIPLUS);  // antialiased
Graphics::setGraphicsImplementation(GRAPHICS_GDI);      // hard-edged, default
```

GDI has no coverage-based rasteriser and cannot antialias geometry at all;
GDI+ can. Unlike Canvas, antialiasing is opt-in per `Graphics`, because a
widget toolkit needs crisp 1px borders as often as it needs smooth curves:

```cpp
g->setAntialias(false);        // hard pixel edges
g->setImageSmoothing(false);   // nearest-neighbour image scaling
```

Both are part of the `save()`/`restore()` state. `getAntialias()` reports
false on GDI however it is set, rather than promising smoothing the backend
cannot deliver.

**Layout** — `BasicLayout` is a border layout: `TOP` and `BOTTOM` span the
full width at their preferred height, `LEFT` and `RIGHT` take the band between
them, and `CENTER` fills the remainder. One child per region, with configurable
vertical and horizontal padding, and nested containers are arranged too.

```cpp
Container* root = new Container(0, 0, 400, 300);
root->setLayout(new BasicLayout(8, 8));   // vpadding, hpadding
root->addChild(toolbar, BasicLayout::TOP);
root->addChild(sidebar, BasicLayout::LEFT);
root->addChild(canvas,  BasicLayout::CENTER);
root->layout();
```

**Demo** — `utils_ui_tests --window` opens an interactive showcase: a window
laid out into header, sidebar, canvas and status bar, every region painted
through the Graphics API, with antialiasing toggled live and mouse events
driving repaints. Source: `utils_cpp.ui/test_src/demo_window.cpp`.

**XML** — SAX-style parser with entity and CDATA handling.

## Testing

```bash
ctest --preset msvc --output-on-failure
```

Three suites: the core library (19 test cases, 66 test methods), the XML
parser, and the UI suite. The UI suite covers three things:

- `Ref<T>` itself -- retain/release, copy, move, self-assignment, assigning an
  alias of the same object, replacement, two handles over one raw pointer,
  detach, and converting construction.

- a graphics-backend comparison that renders one reference scene through GDI
  and GDI+ and asserts that only GDI+ produces partial-coverage edge pixels —
  and that `setAntialias(false)` removes them again. It writes `gdi.bmp`,
  `gdiplus.bmp` and `gdiplus_noaa.bmp` for eyeballing.
- `BasicLayout` geometry, which creates real windows and reads the resulting
  rectangles back, so the arrangement is checked against actual Win32 output
  rather than a model of it.

The interactive window demo is off by default because it needs a desktop:

```bash
utils_ui_tests --window
```

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

- **The widget set is thin.** `Button` is the only control beyond
  `Component`/`Container`/`Window`, and there is no non-Windows backend.
- **`BasicLayout` is the only layout.** A flow, grid or box layout would each
  be a small `Layout` subclass, but none exist yet.
- **Canvas fidelity is partial, by design.** `CompositeOperation` is still the
  GDI raster-op set rather than Canvas's `source-over`/`multiply`/... family,
  and GDI+ only supports two of them. Canvas's two-circle radial gradient has
  no GDI+ equivalent and is approximated with a `PathGradientBrush`. Both want
  Direct2D. Also absent: `ellipse`, `setLineDash`, `miterLimit`, shadows,
  `isPointInPath`, `textAlign`/`textBaseline`.
- **Ownership is now RAII**, via `Ref<T>` in `utils/ui/ref.hpp` -- an owning
  handle over the intrusive count that `UIObject` already carried. Every
  owning member in the UI holds one, `SAFE_DELETE` is gone, and the two
  remaining hand-written reference calls are in `Image::deleteImage`, which
  is a release-one-reference API by design. The count being intrusive means
  two handles over the same raw pointer are safe, which `shared_ptr` could
  not offer here.
- **`Component` background painting is unreliable.** The default background is
  a `GdiColorBrush` wrapping `(HBRUSH)(COLOR_BTNFACE+1)`, which is a system
  colour constant valid only in `WNDCLASS.hbrBackground`, not a real brush
  handle. Components that need a background should paint it themselves.
- **Constructor-parented components are invisible to layout.** `new
  Component(parent, ...)` makes a Win32 child but does not add it to the
  container's list, and `addChild` then refuses it because the parent is
  already set. Use `new Component(NULL, ...)` followed by `addChild`.
- **`http_request` is a data structure without a parser** — the class is empty.
- The build emits conversion warnings (`C4244`/`C4242`) from the math and codec
  code where narrowing is intentional but not spelled out with explicit casts.

## Licence

See [LICENSE](LICENSE).
