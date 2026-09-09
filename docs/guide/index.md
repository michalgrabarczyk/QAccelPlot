---
description: "QAccelPlot documentation — a high-performance hardware-accelerated plotting library for Qt Quick with GPU-rendered 2D line plots, real-time data visualization, and QML-friendly components."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# QAccelPlot

QAccelPlot is a hardware-accelerated plotting library for Qt Quick. It combines
QML-friendly plot composition with C++ data paths designed for large and
frequently changing data sets, rendering through the Qt Scene Graph using GPU
shaders for real-time 2D data visualization.

### Key capabilities

- **GPU-rendered line curves** with solid, dashed, and marker-only styles
- **Gradient fills and strokes** for visual effects on curves
- **Multiple data-ingestion APIs** from convenience QML to efficient C++ buffers
- **Real-time performance** — hundreds of thousands of points at display refresh rate
- **Interactive** — built-in pan, zoom, hover detection, and custom tool support
- **Multiple axes** — primary, secondary, and extra axes for different scales
- **Annotations** — data-attached labels, regions, and measurement overlays
- **Animated transitions** — morph and draw transitions for data updates

### Quick start

```qml
import QtQuick
import QAccelPlot as QAccelPlot

QAccelPlot.Plot {
    id: plot
    anchors.fill: parent
    xAxis: QAccelPlot.Axis { viewportMin: 0; viewportMax: 10; side: QAccelPlot.Axis.Bottom }
    yAxis: QAccelPlot.Axis { viewportMin: -1; viewportMax: 1; side: QAccelPlot.Axis.Left }
    QAccelPlot.LineCurve { xAxis: plot.xAxis; yAxis: plot.yAxis; color: "steelblue" }
}
```

See [Getting started](getting-started.md) for the full build and data setup.

## Explore the documentation

- **[Getting started](getting-started.md)** — Build your first QAccelPlot application.
- **[Architecture](concepts.md)** — See how plots, axes, series, and the Qt
  Scene Graph fit together.
- **[Performance](performance.md)** — Choose the right data-ingestion path and
  measure rendering throughput.
- **[Cookbook](cookbook/index.md)** — Solve common plotting tasks with focused
  recipes.
- **[FAQ](faq.md)** — Answers to common questions about licensing, performance,
  threading, and compatibility.
- **[API reference](api.md)** — Look up types, properties, methods, signals,
  and enums.

The runnable [`examples`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples)
are the authoritative demonstrations. The guide explains the decisions behind
them and links back to complete source files.

## Requirements

- CMake 3.16 or newer
- A C++17 compiler
- Qt 6.2 or newer with Core, Gui, Quick, and ShaderTools
- A hardware Qt Quick Scene Graph backend for curve rendering

The default optimized mode also uses the matching `QuickPrivate` component for
in-place live-data texture updates. A completely public-Qt-only build is
available with `-DQACCELPLOT_USE_QT_PRIVATE_API=OFF`. QAccelPlot is currently
built and distributed as a static library.

## Licensing

QAccelPlot is offered under GPLv3 with the Universal FOSS Exception or under a
separate commercial license. Read the
[`LICENSING.md`](https://github.com/michalgrabarczyk/QAccelPlot/blob/main/LICENSING.md)
overview before distributing an application. Your use of Qt is governed
separately by the license for your Qt installation.
