# QAccelPlot

<p align="left">
  <a href="https://github.com/michalgrabarczyk/QAccelPlot/actions/workflows/ci.yml"><img src="https://github.com/michalgrabarczyk/QAccelPlot/actions/workflows/ci.yml/badge.svg" alt="Build" /></a>
  <a href="https://michalgrabarczyk.github.io/QAccelPlot/coverage/"><img src="https://img.shields.io/endpoint?url=https%3A%2F%2Fmichalgrabarczyk.github.io%2FQAccelPlot%2Fcoverage%2Fbadge.json" alt="Code coverage" /></a>
  <a href="https://michalgrabarczyk.github.io/QAccelPlot/"><img src="https://github.com/michalgrabarczyk/QAccelPlot/actions/workflows/docs.yml/badge.svg" alt="Documentation" /></a>
  <img src="https://img.shields.io/badge/Qt-6.2%2B-41CD52?logo=qt&logoColor=white" alt="Qt 6.2+" />
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white" alt="C++17" />
  <a href="#licensing"><img src="https://img.shields.io/badge/License-GPLv3%20%2F%20Commercial-blue.svg" alt="GPLv3 / Commercial License" /></a>
</p>

**QAccelPlot** is a high-performance, interactive 2D plotting library for
**Qt Quick (QML & C++)**. It renders through the Qt Scene Graph with GPU shaders and replaces
and draws **millions of points per frame at 60 FPS** without downsampling or
level-of-detail (LOD) reduction.

---

<div align="center">
  <img src="docs/guide/assets/pulsar_resonance_portrait.gif" alt="Portrait Cosmic Pulsar CP 1919 animated 3D waterfall rendered by QAccelPlot" height="851" />
  <br />
  <sub><a href="examples/showcases/pulsar/">Explore the Cosmic Pulsar example</a></sub>
</div>

## ⚡ Highlights

- **GPU rendering**: Lines, markers, dashes, and gradients are drawn by shaders on OpenGL, Direct3D 11/12, Vulkan, and Metal.
- **Interactive at full data size**: Pan, cursor-centered zoom, per-axis zoom, and double-click rescale work out of the box. Hover hit testing uses cached block bounds, so it stays responsive on large curves.
- **Fast data paths**: Move interleaved `float` buffers into a curve, skip range scans, or hand buffers off from worker threads with `postData()`.
- **QML-native**: Plots, axes, and series are `QQuickItem`/`QObject` types that compose with Qt Quick layouts and bindings.
- **Invalid samples**: `NaN`, `±Inf`, and non-positive log-axis values render as gaps or are connected across.

---

## 📊 Performance at a Glance

Maximum points each library can replace and render every frame while
sustaining 60 FPS on the same system:

![Performance comparison](docs/guide/assets/performance_comparison.svg)

<sub>Measured on NVIDIA GeForce GTX 970 / AMD Ryzen 9 5900X / Windows 10 Pro with Qt 6.11.2 and OpenGL. Each frame replaces the complete dataset. [Read full benchmark setup & methodology →](https://michalgrabarczyk.github.io/QAccelPlot/performance-comparison/)</sub>

### Architectural Comparison

| Feature / Metric | **QAccelPlot** | **Qt Graphs** | **QCustomPlot** |
| :--- | :--- | :--- | :--- |
| **Rendering Backend** | **Qt Scene Graph** | Qt Scene Graph | QWidget paint buffers (optional OpenGL) |
| **Qt Quick / QML Native** | **Yes** — Native `QQuickItem` | Yes | No (QWidget-based) |
| **60 FPS Dataset Capacity**\* | **~8M points** | ~13K points | ~2.75M (OpenGL) |
| **Bulk Data Input** | **QML points, double vectors, moved or copied `float` buffers, prebuilt vertex cache** | `QList<QPointF>` replace | Key/value vector copies |

<sub>* Point capacity varies with hardware. Figures shown reflect measurements on the [benchmark reference system](https://michalgrabarczyk.github.io/QAccelPlot/performance-comparison/).</sub>

---

## 🚀 Using QAccelPlot

### 1. Declare the Plot in QML

```qml
import QtQuick
import QAccelPlot as QAccelPlot

QAccelPlot.Plot {
    id: plot
    anchors.fill: parent

    xAxis: QAccelPlot.Axis {
        viewportMin: 0; viewportMax: 10
        label: "Time (s)"
    }

    yAxis: QAccelPlot.Axis {
        viewportMin: -1.2; viewportMax: 1.2
        label: "Amplitude"
    }

    QAccelPlot.LineCurve {
        objectName: "curve"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "darkorange"
        lineWidth: 2
    }
}
```

Panning, zooming, and rescaling need no extra code.

### 2. Set Data from C++

```cpp
auto* curve = root->findChild<QAccelPlot::LineCurve*>("curve");
const std::vector<double> times { 0, 2.5, 5, 7.5, 10 };
const std::vector<double> amplitudes { 0, 0.8, -0.6, 0.4, 0 };
curve->setData(times, amplitudes);
```

`setData()` suits small data sets. For large or streaming data, see the
[Performance guide](https://michalgrabarczyk.github.io/QAccelPlot/performance/).
For CMake integration, see
[Getting Started](https://michalgrabarczyk.github.io/QAccelPlot/getting-started/).

> [!NOTE]
> **Pre-1.0 (v0.2):** The API may change between minor releases until 1.0.

---

## 🎨 Example Showcase

Explore runnable applications in the [`examples/`](examples/) directory:

| Category | Example | Description |
| :--- | :--- | :--- |
| Showcases | **[Cosmic Pulsar](examples/showcases/pulsar/)** | Animated 80-ridge CP 1919 waterfall; dense antialiased lines fed from a worker thread. |
| Showcases | **[Performance](examples/showcases/performance/)** | Stress test up to 10M points with display FPS and data update rate. |
| Start | **[Quickstart](examples/quickstart/)** | Minimal plot setup. |
| Plot types | **[Parametric Curves](examples/plot_types/parametric_curves/)** | Spiral, Lissajous figure, spirograph, and Van der Pol phase portrait: curves that loop and cross themselves. |
| Plot types | **[Point Cloud](examples/plot_types/point_cloud/)** | 200K-point scatter colored by value through a Colormap preset, with a hover tooltip. |
| Styling | **[Line Styles](examples/styling/line_styles/)** | Dash patterns, line weights, and antialiasing. |
| Styling | **[Markers](examples/styling/markers/)** | Every marker shape, filled and hollow, and dense Pixel scatter. |
| Styling | **[Gradients](examples/styling/gradients/)** | Gradient area fills and strokes with presets, direction, and baseline. |
| Styling | **[Transitions](examples/styling/transitions/)** | Morph and draw animations between datasets. |
| Styling | **[Theming](examples/styling/theming/)** | Plot, axis, grid, legend, and series colors bound to the dark or light palette. |
| Axes | **[Tick Formats](examples/axes/tick_formats/)** | Category, logarithmic, and date/time tick labels. |
| Axes | **[Multiple Axes](examples/axes/multiple_axes/)** | Dual wavelength/frequency scale, multi-rate time axes, and ECG panels sharing one time axis. |
| Data | **[Realtime](examples/data/realtime/)** | Scrolling vibration signal in a 20-second window, updated every frame. |
| Data | **[Missing Data](examples/data/missing_data/)** | NaN, ±Inf, and log-invalid samples drawn as gaps or connected. |
| Interaction | **[Annotations](examples/interaction/annotations/)** | QML overlays anchored to data coordinates. |
| Interaction | **[Measurement Tools](examples/interaction/measurement_tools/)** | Distance ruler, angle tool, rectangular selection, and point markers. |

---

## 🧪 Testing

- **[Unit tests](.github/workflows/ci.yml)**: plotting behavior, data
  handling, axes, formatting, styling, interaction, and transitions, built
  with GCC and Clang.
- **[Code coverage](https://michalgrabarczyk.github.io/QAccelPlot/coverage/)**:
  percentage of library C++ source lines exercised by unit tests, updated by
  CI on `main`.
- **[Performance benchmarks](.github/workflows/benchmark.yml)**: data-ingestion
  and rendering regressions.
- **[Visual acceptance tests](.github/workflows/ai-regression.yml)**: every
  example rendered on Qt 6.2, 6.8, and 6.11 with OpenGL, OpenGL ES, Vulkan,
  Direct3D 11/12, and Metal, then checked against per-page visual contracts by
  AI inspection.

---

## 🛠️ Building from Source

### Requirements

- **CMake** 3.16 or newer
- **C++17** compatible compiler
- **Qt 6.2 or newer** with `Core`, `Gui`, `Quick`, and `ShaderTools`
- Optional: the matching `QuickPrivate` component for faster live-data texture
  updates
- A hardware Qt Quick Scene Graph backend (OpenGL, Direct3D 11/12, Vulkan, or Metal)

QAccelPlot builds as a static library.

```sh
# Configure with examples enabled
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DQACCELPLOT_BUILD_EXAMPLES=ON

# Build library and examples
cmake --build build --config Release

# Build every compiled test executable
cmake --build build --config Release --target QAccelPlotTests

# Run tests
ctest --test-dir build --output-on-failure
```

| Option | Default | Effect |
| :--- | :--- | :--- |
| `QACCELPLOT_USE_QT_PRIVATE_API` | `ON` if `QuickPrivate` is found | In-place data texture updates; `OFF` uses public Qt API only. See [texture upload modes](docs/guide/performance.md#select-the-texture-upload-mode). |
| `QACCELPLOT_BUILD_VISUAL_TESTS` | `OFF` | Screenshot and visual validation tests. Requires examples and tests. |
| `QACCELPLOT_BUILD_AI_VISUAL_TESTS` | `OFF` | Paid AI inspection tests. Requires visual tests. |

---

## 📖 Documentation

- 🚀 **[Getting Started](https://michalgrabarczyk.github.io/QAccelPlot/getting-started/)** — CMake integration and a first plot.
- 🏗️ **[Concepts](https://michalgrabarczyk.github.io/QAccelPlot/concepts/)** — Axes, series, gaps, overlays, and threading.
- ⚡ **[Performance Guide](https://michalgrabarczyk.github.io/QAccelPlot/performance/)** — Data paths, worker handoff, and benchmarking.
- 🍳 **[Cookbook](https://michalgrabarczyk.github.io/QAccelPlot/cookbook/)** — Real-time data, multiple axes, styling, and tools.
- ❓ **[FAQ](https://michalgrabarczyk.github.io/QAccelPlot/faq/)** — Threading, licensing, and backends.
- 📚 **[API Reference](https://michalgrabarczyk.github.io/QAccelPlot/api/)** — Classes, properties, and methods.

---

## 🗺️ Road to 1.0

Planned before 1.0:

- Point-cloud series for large sets of unconnected points.
- Bar charts.
- Heatmaps.
- SVG and PNG export.
- Box zoom and per-axis pan and zoom limits.
- Touch panning and pinch zoom.

---

## 🤝 Contributing & Feedback

Report bugs, feature requests, and compatibility notes through
[GitHub Issues](https://github.com/michalgrabarczyk/QAccelPlot/issues). Pull
requests are not accepted; see [CONTRIBUTING.md](CONTRIBUTING.md).

---

## 📄 Licensing

QAccelPlot is available under a **dual-licensing model**:

1. **Open-Source License:** GNU General Public License v3.0 only with the **Universal FOSS Exception**. Qualifying applications distributed under an OSI-approved or FSF-free software license (e.g. MIT, BSD, Apache 2.0, LGPL) may link with QAccelPlot while retaining their own open-source license.
2. **Commercial License:** Separate commercial terms for embedding QAccelPlot in proprietary or closed-source commercial applications without GPL requirements.

📧 For commercial licensing inquiries, contact: **[qaccelplot@gmail.com](mailto:qaccelplot@gmail.com)**

| Intended Use Case | Open-Source (GPLv3 + FOSS Exception) | Commercial License |
| :--- | :--- | :--- |
| **GPLv3 Application** | ✅ Permitted under GPLv3 | Not required |
| **Qualifying FOSS (MIT, BSD, Apache, LGPL, etc.)** | ✅ Permitted under Universal FOSS Exception | Not required |
| **Commercial FOSS Product (Charging fee for FOSS)** | ✅ Permitted (Charging does not make software proprietary) | Not required |
| **Proprietary / Closed-Source Distributed Product** | ❌ Not permitted | ✅ **Required** |
| **Private / Internal Enterprise Use (No distribution)** | ✅ Permitted under GPLv3 terms | Not required |
| **Server-Side / SaaS Use (No client distribution)** | ✅ Permitted (GPLv3 has no AGPL network-copyleft clause) | Not required |

For licensing details and links to the full license texts, see
[LICENSING.md](LICENSING.md).
