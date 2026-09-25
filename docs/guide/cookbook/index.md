---
description: "QAccelPlot cookbook — recipes for static plots, real-time scrolling data, axis formats, multiple axes, styling, annotations, point clouds, and background data production."
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
| [Point clouds](point-clouds.md) | Large unconnected scatter data, coloring points by value, color bars, and hover picking |
| [Background data production](background-data.md) | Safe high-throughput worker-to-UI handoff |

For method-by-method detail, use the [API reference](../api.md). For choosing a
data path, use the [Performance guide](../performance.md).
