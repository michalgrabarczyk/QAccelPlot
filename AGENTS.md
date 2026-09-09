# QAccelPlot – AI Agent Instructions

## Project Overview

QAccelPlot is a high-performance Qt Quick plotting library. It uses the Qt
Scene Graph (QSG) for hardware-accelerated rendering, exposing QML-friendly
components backed by C++ `QQuickItem` subclasses. The library is dual-licensed
under GPLv3 with the Universal FOSS Exception, or a separate commercial license.

## Build & Toolchain

- **Language**: C++17
- **Build system**: CMake ≥ 3.16
- **Qt version**: Qt 6.2 or newer — Qt modules: Core, Gui, Quick, ShaderTools
- **QML module URI**: `QAccelPlot`
- **Library type**: Static library (shared not yet supported)

### Build commands

```sh
# Default build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# With examples
cmake -S . -B build -DQACCELPLOT_BUILD_EXAMPLES=ON
cmake --build build --config Release

# With tests
cmake -S . -B build -DQACCELPLOT_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure

# With benchmarks
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DQACCELPLOT_BUILD_BENCHMARKS=ON
cmake --build build --config Release

# API documentation (requires Doxygen)
cmake -S . -B build -DQACCELPLOT_BUILD_DOCS=ON
cmake --build build --target qaccelplot_docs
```

### Qt version notes

The default optimized build uses `QuickPrivate` for in-place live-data texture
updates. Qt 6.11 and newer package it as a separately discoverable component;
earlier Qt releases expose `Qt6::QuickPrivate` automatically when the public
Quick component is loaded. Configure with
`-DQACCELPLOT_USE_QT_PRIVATE_API=OFF` for a public-Qt-only build.

## Architecture

Each visual element (`QAccelPlot`, `LineCurve`, etc.) inherits from
`QQuickItem` and overrides `updatePaintNode` to build a QSG node tree.
Non-visual QML types (`Axis`, `Grid`, `AbstractPlotAnnotation`, etc.) inherit
from `QObject`. Shader files are pre-compiled to `.qsb` format (via Qt Shader
Tools) and embedded via `qt_add_resources`. Data is typically supplied from a
background thread; the main thread swaps in pre-computed buffers.

### Source directory map

```
QAccelPlot/
├── src/
│   ├── QAccelPlot.hpp/.cpp   — PlotView: main plot canvas item
│   ├── PlotMouseEvent.*      — Custom mouse event type
│   ├── PlotBorder.*          — Plot frame decoration
│   ├── MathUtils.hpp         — Math utilities
│   ├── QAccelPlotLogging.*   — Qt logging category
│   ├── annotations/          — DataAnchor (attach QML items to data coords)
│   ├── axis/                 — Axis, AxisTicker, AxisTickPainter
│   ├── effects/              — GradientFill, GradientStroke, LineCurveEffect
│   ├── formatters/           — DateTimeTickLabelFormatter, LogTickLabelFormatter,
│   │                           NumericTickLabelFormatter, TextTickLabelFormatter,
│   │                           TickLabelFormatter (base/JS callback)
│   ├── grid/                 — Grid, GridNode
│   ├── linestyles/           — SolidLine, DashLine, NoLine
│   ├── materials/            — QSGMaterial subclasses for GPU rendering
│   ├── renderers/            — LineCurveLineRenderer, LineCurvePointRenderer
│   ├── series/               — LineCurve, PlotSeries, LineCurveVertexCache,
│   │                           SpatialGrid (hover hit testing)
│   ├── shaders/              — GLSL shaders compiled to .qsb
│   ├── shapes/               — RectangleList
│   └── transitions/          — DataTransition, DrawTransition, MorphTransition
├── qml/                      — QML helper types (Plot, Legend, Colors)
├── test/                     — Unit tests
└── CMakeLists.txt            — Library build configuration
```

### Other directories

- `examples/` — Complete runnable applications (quickstart, realtime,
  annotations, axis_formats, custom_axis, interactive_tools,
  performance_showcase, styling_and_transitions)
- `benchmarks/` — Performance measurement scenarios and harness
- `docs/` — MkDocs guide source + Doxygen configuration
- `qml/` — Top-level QML types (Plot wrapper, Legend, Colors palette)
- `scripts/` — Build and CI helper scripts
- `cmake/` — CMake modules

## C++ Conventions

- `#pragma once` for all header include guards.
- Trailing `_` for all private member variables (e.g., `xAxis_`, `color_`).
- Declare Qt properties with the full
  `Q_PROPERTY(Type name READ getter WRITE setter NOTIFY signal)` pattern.
- Register QML elements with `QML_NAMED_ELEMENT(ClassName)` in the class body.
- Keep `.hpp` / `.cpp` pairs for every class; headers organized in
  subdirectories under `src/`.
- Follow Qt's signal/slot naming: `fooChanged()` signal paired with
  `setFoo()` setter.
- Prefer `qreal` for geometric values exposed to QML; use `float` for
  performance-critical internal buffers.
- Use `std::vector<float>` with interleaved XY data for plot point buffers.
- Mark variables and functions `const` whenever possible.
- When defining a variable in `.cpp`, prefer `auto v = double{};` over
  `double v{};`.
- Always use braces `{ }` after `if`, `for`, and `while` bodies, even for
  single-statement bodies.
- Always run `clang-format` after making changes to `.cpp` or `.hpp` files.

## Clean Code

- Keep functions short — if a function exceeds ~50 lines, extract smaller,
  well-named helper functions.
- Define functions in `.cpp` files in the same order they are declared in the
  corresponding `.hpp` file.
- Follow the Single Responsibility Principle: each class should have one clear
  reason to change.
- Split large classes into smaller, focused ones when responsibilities can be
  cleanly separated.

## QML Conventions

- QML files live in `qml/`.
- Use Qt Quick property bindings rather than imperative assignments where
  practical.
- The library uses a namespaced import pattern:
  `import QAccelPlot as QAccelPlot`. This keeps generic type names (like
  `Colors`) out of the application's unqualified namespace.

## Language

- Use American English spelling in all documentation, comments, and
  identifiers (e.g., "color" not "colour", "normalize" not "normalise").

## Git

- Never commit or push changes unless explicitly asked.
- Name branches according to the type of work:
  - Use `fix/<descriptive-name>` for bug fixes.
  - Use `dev/<descriptive-name>` for all other changes, including features,
    refactors, documentation, and maintenance.
- Keep branch names short, descriptive, lowercase, and hyphen-separated after
  the prefix.

## Releases

`CHANGELOG.md` is the source of truth for release notes. Before creating a
release tag:

1. Update the top-level CMake project version to the release version.
2. Move the relevant changelog entries from `[Unreleased]` into a dated section
   for that exact version, keeping `[Unreleased]` for future work.
3. Update migration or compatibility documentation for breaking changes.
4. Merge the release change into `main` and ensure normal CI passes.

Create and push the matching tag from `main`, for example:

```sh
git tag v0.2.0
git push origin v0.2.0
```

The release workflow requires the tag to match the CMake version and a
non-empty changelog section. Before 1.0, releases must increment `MINOR` and
keep `PATCH` at zero. The tagged commit must be reachable from `main`.

Published release tags are immutable. Never move or reuse a tag after its
GitHub Release has been created.

## Adding New Plot Elements

1. Create `src/MyElement.hpp` and `src/MyElement.cpp`.
2. Inherit from `QQuickItem` (visual) or `QObject` (non-visual).
3. Add `QML_NAMED_ELEMENT(MyElement)` and all `Q_PROPERTY` declarations.
4. Override `updatePaintNode` if the element needs custom rendering.
5. Register the new source files in `QAccelPlot/CMakeLists.txt`.

## Common Pitfalls

- **Software Qt Quick backend**: QAccelPlot's custom curve shaders require a
  hardware Scene Graph backend (OpenGL, Direct3D, Vulkan, or Metal). The
  software backend cannot render curves and emits a warning. Check
  `QSG_RHI_BACKEND` and `QT_QUICK_BACKEND` environment variables.
- **Buffer ownership**: `setDataF(std::vector<float>&&, int)` moves the buffer
  immediately and must be called on the curve's thread (normally the UI
  thread). For worker threads, use `postData()` which queues the handoff.
- **No-range APIs**: `setDataFNoRange()` skips data-range calculation. If you
  use it, you must maintain accurate `dataMin`/`dataMax` values on the axes
  yourself, or rescaling will be incorrect.
- **Static QML module**: Applications link the umbrella
  `QAccelPlot::QAccelPlot` target and call `qt_import_qml_plugins()` before
  `qt_finalize_executable()`. C++-only tools can link `QAccelPlot::Core`.
- **Thread safety**: Never write directly to a `QQuickItem` from a worker
  thread. Use `LineCurve::postData()` or queue the handoff to the UI thread.

## Testing

Tests are built with `-DQACCELPLOT_BUILD_TESTS=ON`. Run with:

```sh
ctest --test-dir build --output-on-failure
```

Example build tests (link/run checks for each example) require
`-DQACCELPLOT_BUILD_EXAMPLES=ON` as well.

## Documentation

- User guide: MkDocs Material, source in `docs/guide/`, built with
  `mkdocs build --strict`.
- API reference: Doxygen, source comments use `///` triple-slash style.
  `QT_AUTOBRIEF` is enabled so the first sentence of a `///` block is the
  brief description.
- Every public C++ member should have a `/// \brief` documentation comment.
