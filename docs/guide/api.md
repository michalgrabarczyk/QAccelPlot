---
description: "QAccelPlot API reference — generated documentation for all public C++ and QML types, properties, methods, signals, and enums."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# API reference

- [Class list](api/annotated.md)
- [Class index](api/classes.md)
- [Class hierarchy](api/hierarchy.md)
- [Namespaces](api/namespaces.md)
- [Files](api/files.md)

## Key types

- [`PlotView`](api/classQAccelPlot_1_1QAccelPlot.md) — plot item: layout,
  pan and zoom, coordinate conversion, grid, and series collection. QML
  applications use its `Plot` wrapper.
- [`Axis`](api/classQAccelPlot_1_1Axis.md) — ranges, log scale, ticks, labels,
  and per-axis pan and zoom.
- [`LineCurve`](api/classQAccelPlot_1_1LineCurve.md) — GPU-rendered lines and
  markers, data ingestion, effects, transitions, gaps, and hover.
- [`PointCloud`](api/classQAccelPlot_1_1PointCloud.md) — large sets of
  unconnected points as markers, colored uniformly or by per-point values.
- [`RectangleList`](api/classQAccelPlot_1_1RectangleList.md) — many data-space
  rectangles in one item.
- [`DataAnchor`](api/classQAccelPlot_1_1DataAnchor.md) — QML overlays at data
  coordinates.
- [`PlotMouseEvent`](api/classQAccelPlot_1_1PlotMouseEvent.md) — mouse
  positions in plot and data coordinates.

## Feeding curve data

- `setData()` — QML data or double-precision C++ containers.
- `setDataF(std::vector<float>&&, count)` — large interleaved buffers on the
  curve's thread.
- [`postData()`](api/classQAccelPlot_1_1LineCurve.md#function-postdata) —
  completed buffers from a worker thread.
- `setDataFNoRange()` — only when you maintain axis data ranges yourself.
- `NaN` marks a missing sample; `gaps.nanMode` breaks or connects the curve.
  See [Invalid samples and gaps](concepts.md#invalid-samples-and-gaps).

See [Select the data path](performance.md#select-the-data-path).
