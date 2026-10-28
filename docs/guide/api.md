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
- [`HeatmapSeries`](api/classQAccelPlot_1_1HeatmapSeries.md) — a uniform
  grid of values as cells colored through a colormap.
- [`ColorBar`](api/classQAccelPlot_1_1ColorBar.md) — continuous key for the
  colormap and value range of a point cloud or heatmap.
- [`BandSeries`](api/classQAccelPlot_1_1BandSeries.md) — filled area between a
  low and a high value at each X, with optional edge lines.
- [`ErrorBarSeries`](api/classQAccelPlot_1_1ErrorBarSeries.md) — capped lines
  from a low to a high bound at each data point, vertical or horizontal.
- [`RectangleSeries`](api/classQAccelPlot_1_1RectangleSeries.md) — many data-space
  rectangles in one item: spans and state timelines.
- [`BarSeries`](api/classQAccelPlot_1_1BarSeries.md) — vertical, horizontal, and
  grouped bar charts, and ranged bars such as histogram bins.
- [`Histogram`](api/classQAccelPlot_1_1Histogram.md) — samples counted into
  bins, as data for a `BarSeries`.
- [`DataAnchor`](api/classQAccelPlot_1_1DataAnchor.md) — QML overlays at data
  coordinates.
- [`PlotMouseEvent`](api/classQAccelPlot_1_1PlotMouseEvent.md) — the mouse
  event of `PlotView`'s mouse signals, in item pixels; `accept()` skips the
  built-in pan, rectangle zoom, or rescale.
- [`LegendModel`](api/classQAccelPlot_1_1LegendModel.md) — legend entries of the
  visible series that have one, each a [`LegendEntry`](api/structQAccelPlot_1_1LegendEntry.md).
- [`SeriesInspection`](api/classQAccelPlot_1_1SeriesInspection.md) — per-series
  data queries: nearest data points, brackets, region statistics, and index pages.
- [`PlotInspector`](api/classQAccelPlot_1_1PlotInspector.md) — cursor state and
  one model row per series for crosshairs and tooltips.
- [`SelectionTool`](api/classQAccelPlot_1_1SelectionTool.md) — data-space box
  and range selection with per-series statistics.

See [Data inspection](cookbook/data-inspection.md) for usage and readiness.

## Feeding curve data

- `setData()` — QML data or double-precision C++ containers.
- `setDataF(std::vector<float>&&, count)` — large interleaved buffers on the
  curve's thread.
- [`postData()`](api/classQAccelPlot_1_1LineCurve.md#function-postdata-22) —
  completed buffers from a worker thread.
- `PlotSeries::DataBounds` — passed with the data when its extents are already known.
- `NaN` marks a missing data point; `gaps.nanMode` breaks or connects the curve.
  See [Invalid data points and gaps](concepts.md#invalid-data-points-and-gaps).

See [Select the data path](performance.md#select-the-data-path).
