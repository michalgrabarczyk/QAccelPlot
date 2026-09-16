---
description: "QAccelPlot documentation — GPU-rendered 2D plotting for Qt Quick that replaces and draws millions of points per frame with built-in interaction."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# QAccelPlot

QAccelPlot is a 2D plotting library for Qt Quick. Curves render through the Qt
Scene Graph with GPU shaders, and C++ data paths replace large data sets every
frame.

### Key capabilities

- **Throughput** — about 8M points replaced and drawn per frame at 60 FPS on the
  [reference system](performance-comparison.md), without downsampling
- **Data paths** — QML points, double vectors, moved `float` buffers, no-range
  updates, and worker-thread `postData()`
- **Interaction** — pan, cursor-centered and per-axis zoom, rescale, hover hit
  testing, and plot mouse events for custom tools
- **Invalid samples** — `NaN`, `±Inf`, and non-positive log values break or
  connect the curve consistently
- **Curves** — solid, dashed, and marker-only styles; gradient fills and
  strokes; morph and draw transitions
- **Layout** — secondary and extra axes, data-anchored QML overlays, and
  `RectangleList` for many regions

### Quick start

```qml
import QtQuick
import QAccelPlot as QAccelPlot

QAccelPlot.Plot {
    id: plot
    anchors.fill: parent
    xAxis: QAccelPlot.Axis { viewportMin: 0; viewportMax: 10 }
    yAxis: QAccelPlot.Axis { viewportMin: -1; viewportMax: 1 }
    QAccelPlot.LineCurve { xAxis: plot.xAxis; yAxis: plot.yAxis; color: "steelblue" }
}
```

See [Getting started](getting-started.md) for the full build and data setup.

## Explore the documentation

- **[Getting started](getting-started.md)** — CMake integration and a first plot.
- **[Concepts](concepts.md)** — Axes, series, gaps, overlays, and threading.
- **[Performance](performance.md)** — Data paths, worker handoff, and benchmarking.
- **[Cookbook](cookbook/index.md)** — Recipes for common tasks.
- **[FAQ](faq.md)** — Licensing, threading, and compatibility.
- **[API reference](api.md)** — Types, properties, methods, signals, and enums.

Each recipe links to a complete, runnable
[example](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples).

## Requirements

- CMake 3.16 or newer
- A C++17 compiler
- Qt 6.2 or newer with Core, Gui, Quick, and ShaderTools
- A hardware Qt Quick Scene Graph backend

When the matching `QuickPrivate` component is available, it is used for
in-place data texture updates. `-DQACCELPLOT_USE_QT_PRIVATE_API=OFF` forces a
public-Qt-only build. QAccelPlot builds as a static library.

## Licensing

GPLv3 with the Universal FOSS Exception, or a commercial license. See
[`LICENSING.md`](https://github.com/michalgrabarczyk/QAccelPlot/blob/main/LICENSING.md).
Qt is licensed separately.
