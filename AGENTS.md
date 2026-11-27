# QAccelPlot – AI Agent Instructions

## Project Overview

QAccelPlot is a high-performance Qt Quick plotting library. It uses the Qt
Scene Graph (QSG) for hardware-accelerated rendering, exposing QML-friendly
components backed by C++ `QQuickItem` subclasses.

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
# Example build tests (link/run checks per example) also need -DQACCELPLOT_BUILD_EXAMPLES=ON

# API documentation (requires Doxygen)
cmake -S . -B build -DQACCELPLOT_BUILD_DOCS=ON
cmake --build build --target qaccelplot_docs

# User guide (MkDocs Material, source in docs/guide/)
mkdocs build --strict
```

### Qt version notes

The default optimized build uses `QuickPrivate` for in-place live-data texture
updates. Qt 6.11 and newer package it as a separately discoverable component;
earlier Qt releases expose `Qt6::QuickPrivate` automatically when the public
Quick component is loaded. Configure with
`-DQACCELPLOT_USE_QT_PRIVATE_API=OFF` for a public-Qt-only build.

## Architecture

Each visual element (`PlotView`, `LineCurve`, etc.) inherits from
`QQuickItem` and overrides `updatePaintNode` to build a QSG node tree.
Non-visual QML types (`Axis`, `Grid`, `AbstractPlotAnnotation`, etc.) inherit
from `QObject`. Shader files are pre-compiled to `.qsb` format (via Qt Shader
Tools) and embedded via `qt_add_resources`. Data is typically supplied from a
background thread; the main thread swaps in pre-computed buffers. Point buffers
are `std::vector<float>` with interleaved XY data.

New sources go in the matching subfolder of `src/QAccelPlot/`; the
`CONFIGURE_DEPENDS` glob in `QAccelPlot/CMakeLists.txt` picks them up.

### Source directory map

`QAccelPlot/src/` is the include root (`#include <QAccelPlot/...>`). The top
level of `src/QAccelPlot/` holds `PlotView` and its helpers (mouse events,
border, drag rectangle, rectangle zoom, logging).

| Folder | Contents |
|---|---|
| `annotations/` | QML items attached to data coordinates |
| `axis/` | Axes, ticks, axis labels, color bar |
| `data/` | Data preparation, such as histogram binning |
| `effects/` | Series effects (gradients, glow, fade trail) and `Colormap` |
| `export/` | Raster image export |
| `formatters/` | Tick label formatters |
| `grid/` | Grid lines |
| `inspection/` | Hover and selection queries over series data |
| `internal/` | Helpers shared across folders or used by `PlotView` |
| `legend/` | Legend model and per-series legend entries |
| `linestyles/` | Solid, dashed, and no-line styles |
| `materials/` | `QSGMaterial` subclasses (GPU side) |
| `renderers/` | Geometry builders for line curves and band edges |
| `series/` | All series types and their grouped properties |
| `theme/` | Color palettes and the `Colors` QML singleton |
| `transitions/` | Enter, update, and exit animations for series data |

Next to `src/` in `QAccelPlot/`: `qml/` (QML helper types such as `Plot` and
`Legend`), `shaders/` (GLSL compiled to `.qsb`), and `test/` (unit tests;
`consumer/` is a standalone app linking the library as a user project does).

### Other directories

- `examples/` — Complete runnable applications. `quickstart/` sits at the top;
  the rest are grouped by category: `plot_types/`, `styling/`, `axes/`,
  `data/`, `interaction/`, and `showcases/`. Each example directory is named
  after its target in snake_case (`QAccelPlotExampleTickFormats` →
  `axes/tick_formats/`). `common/` holds shared example code (`ExampleHeader`,
  `ExampleUtils`); `test/` holds example unit and visual tests.
- `docs/` — MkDocs guide source + Doxygen configuration
- `scripts/` — Build and CI helper scripts
- `cmake/` — CMake modules

## C++ Conventions

- Trailing `_` for all private member variables (e.g., `xAxis_`, `color_`).
- Declare Qt properties with the full
  `Q_PROPERTY(Type name READ getter WRITE setter NOTIFY signal)` pattern.
- Register QML elements with `QML_NAMED_ELEMENT(ClassName)` in the class body.
- Give a property whose default is computed or automatic a `RESET` function,
  so assigning `undefined` restores the default:
  - When the object knows the value in effect, the getter returns it and the
    change signal fires when it changes, as for `AxisLabel.rotation`. Use
    `Internal::ResettableValue` for the storage.
  - When it differs per consumer (a shared `Colormap`, an effect or
    transition used by several series) or reading it would force a data scan,
    the getter returns NaN, an invalid color, or `null` while unset. Document
    that readback.
  - Setters keep accepting NaN (not ±Inf), an invalid color, or `null` as a
    reset. Document only `undefined`.
- Inside `namespace QAccelPlot`, write library types unqualified, including in
  `Q_PROPERTY`, `Q_INVOKABLE`, and signal signatures (`Axis*`,
  `InspectionNS::Status`); moc and QML resolve them. Don't name a class
  `QAccelPlot`: it would hide the namespace and force `::QAccelPlot::` prefixes.
- Keep `.hpp` / `.cpp` pairs for every class; headers organized in
  subdirectories under `src/QAccelPlot/`. Define functions in the `.cpp` in
  the order the `.hpp` declares them. Include library headers with the
  `QAccelPlot/` prefix: `"QAccelPlot/series/LineCurve.hpp"` inside `QAccelPlot/`,
  `<QAccelPlot/series/LineCurve.hpp>` in examples.
- Put library-only helpers in `QAccelPlot::Internal`. A helper used by one
  folder lives in that folder's `internal/`; a helper shared across folders
  lives in the top-level `src/QAccelPlot/internal/`.
- Follow Qt's signal/slot naming: `fooChanged()` signal paired with
  `setFoo()` setter.
- Give every public C++ member a `/// \brief` documentation comment.
- When defining a variable in `.cpp`, prefer `auto v = double{};` over
  `double v{};`.
- Mark an unused parameter `[[maybe_unused]]` and keep its name; don't comment
  the name out or use `Q_UNUSED`. Remove the parameter instead when no
  override, signal, or shared signature requires it.
- Always run `clang-format` after making changes to `.cpp` or `.hpp` files.
  The Static analysis CI workflow checks them with clang-format 19.
- Write string and character literals in ASCII, with `\u` escapes for other
  characters (`QStringLiteral("\u00B5m")`); `scripts/check-source-text.py`
  enforces this in CI.
- Library sources must pass `clang-tidy` with the repository `.clang-tidy`
  (enforced by the Static analysis CI workflow).

## QML Conventions

- The library uses a namespaced import pattern:
  `import QAccelPlot as QAccelPlot`. This keeps generic type names (like
  `Colors`) out of the application's unqualified namespace.

## Language & Git

- Use American English spelling in all documentation, comments, and
  identifiers (e.g., "color" not "colour", "normalize" not "normalise").
- Name branches `fix/<name>` for bug fixes and `dev/<name>` for everything
  else; keep `<name>` short, lowercase, and hyphen-separated.

## Common Pitfalls

- **Software Qt Quick backend**: QAccelPlot's custom curve shaders require a
  hardware Scene Graph backend (OpenGL, Direct3D, Vulkan, or Metal). The
  software backend cannot render curves and emits a warning. Check
  `QSG_RHI_BACKEND` and `QT_QUICK_BACKEND` environment variables.
- **Threading**: Never write directly to a `QQuickItem` from a worker thread.
  `setDataF(std::vector<float>&&, int)` moves the buffer immediately and must
  be called on the curve's thread (normally the UI thread); from a worker
  thread, use `LineCurve::postData()`, which queues the handoff.
- **Data ranges**: Data setters do not scan for the data range. A series
  computes it when something reads it (`rescaleToData()`, an axis with
  `autoRescale`, `dataMin`/`dataMax`, a gradient with an unset bound). A series must
  call `invalidateDataRanges()` after every change that affects its extents.
  Data passed with `PlotSeries::DataBounds` is never scanned, so the bounds
  must cover the data.
- **Static QML module**: Applications link the umbrella
  `QAccelPlot::QAccelPlot` target and call `qt_import_qml_plugins()` before
  `qt_finalize_executable()`. C++-only tools can link `QAccelPlot::Core`.

## Where to read more

- Before reviewing a change, writing documentation or `///` comments, or adding
  or changing an example, read `CODING_STANDARDS.md`.
- Before tagging a release, read `docs/agents/releases.md`.
- Before opening a pull request, read `CONTRIBUTING.md`.

## Agent skills

### Issue tracker

Issues live in GitHub Issues for `michalgrabarczyk/QAccelPlot` (via the `gh` CLI). See `docs/agents/issue-tracker.md`.

### Domain docs

Single-context: one `CONTEXT.md` and `docs/adr/` at the repo root. See `docs/agents/domain.md`.
