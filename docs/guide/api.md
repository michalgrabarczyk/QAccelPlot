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
- [`PlotRectangleZoom`](api/classQAccelPlot_1_1PlotRectangleZoom.md) — gesture
  configuration, selection styling, and read-only selection state.
- [`LineCurve`](api/classQAccelPlot_1_1LineCurve.md) — GPU-rendered lines and
  markers, data ingestion, effects, transitions, gaps, and hover.
- [`PointCloud`](api/classQAccelPlot_1_1PointCloud.md) — large sets of
  unconnected points as markers, colored uniformly or by per-point values.
- [`ColorBar`](api/classQAccelPlot_1_1ColorBar.md) — continuous key for a
  point cloud's colormap and value range.
- [`BandSeries`](api/classQAccelPlot_1_1BandSeries.md) — filled area between a
  low and a high value at each X, with optional edge lines.
- [`RectangleSeries`](api/classQAccelPlot_1_1RectangleSeries.md) — many data-space
  rectangles in one item: spans, bands, and state timelines.
- [`BarSeries`](api/classQAccelPlot_1_1BarSeries.md) — vertical, horizontal, and
  grouped bar charts, and ranged bars such as histogram bins.
- [`Histogram`](api/classQAccelPlot_1_1Histogram.md) — samples counted into
  bins, as data for a `BarSeries`.
- [`DataAnchor`](api/classQAccelPlot_1_1DataAnchor.md) — QML overlays at data
  coordinates.
- [`PlotMouseEvent`](api/classQAccelPlot_1_1PlotMouseEvent.md) — mouse
  positions in plot and data coordinates.
- [`SeriesInspection`](api/classQAccelPlot_1_1SeriesInspection.md) — per-series
  data queries: nearest samples, brackets, region statistics, and index pages.
- [`PlotInspector`](api/classQAccelPlot_1_1PlotInspector.md) — cursor state and
  one model row per series for crosshairs and tooltips.
- [`SelectionTool`](api/classQAccelPlot_1_1SelectionTool.md) — data-space box
  and range selection with per-series statistics.

See [Data inspection](cookbook/data-inspection.md) for usage, query contracts, and readiness.

## Feeding curve data

- `setData()` — QML data or double-precision C++ containers.
- `setDataF(std::vector<float>&&, count)` — large interleaved buffers on the
  curve's thread.
- [`postData()`](api/classQAccelPlot_1_1LineCurve.md#function-postdata) —
  completed buffers from a worker thread.
- `PlotSeries::DataBounds` — passed with the data when its extents are already known.
- `NaN` marks a missing sample; `gaps.nanMode` breaks or connects the curve.
  See [Invalid samples and gaps](concepts.md#invalid-samples-and-gaps).

See [Select the data path](performance.md#select-the-data-path).
