---
description: "QAccelPlot cookbook — recipes for static plots, real-time scrolling data, axis formats, multiple axes, styling, annotations, data inspection, point clouds, heatmaps, rectangle zoom, bar charts, histograms, bands, error bars, background data production, and exporting images."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Cookbook

Each recipe links to a complete runnable example.

| Use case | Covers |
| --- | --- |
| [Basic static plot](basic-plot.md) | Axes, several curves, markers, and a legend |
| [Real-time scrolling data](realtime.md) | A fixed window updated once per rendered frame, with dropouts as gaps |
| [Axis formats](axis-formats.md) | Date/time, categories, logarithmic scales, and custom labels |
| [Multiple axes](multiple-axes.md) | Secondary scales and signals with different domains |
| [Styling and transitions](styling.md) | Dashes, markers, gradients, fills, and animated updates |
| [Annotations and tools](annotations.md) | Data-attached labels, regions, measurement tools, and mouse handling |
| [Data inspection](data-inspection.md) | Crosshair, tooltip, nearest-data-point queries, region statistics, and selection |
| [Point clouds](point-clouds.md) | Large unconnected scatter data, coloring points by value, color bars, and hover picking |
| [Heatmaps](heatmaps.md) | Grids of values colored through a colormap, color bars, and reading cells |
| [Rectangle zoom](rectangle-zoom.md) | Drag a region to zoom all axes, or zoom from a custom tool |
| [Bar charts](bar-charts.md) | Vertical, horizontal, and grouped bars, baselines, category colors, and hover |
| [Histograms](histograms.md) | Equal and uneven bins, counts or density, from QML or C++ |
| [Bands](bands.md) | Confidence intervals and envelopes between a low and a high value, with edge lines |
| [Error bars](error-bars.md) | Symmetric and asymmetric errors, vertical and horizontal, on points and grouped bars |
| [Background data production](background-data.md) | Safe high-throughput worker-to-UI handoff |
| [Exporting images](export.md) | PNG, JPEG, and other raster files, the clipboard, and `QImage` capture |

For method-by-method detail, use the [API reference](../api.md). For choosing a
data path, use the [Performance guide](../performance.md).
