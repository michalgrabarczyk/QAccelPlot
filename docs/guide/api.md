---
description: "QAccelPlot API reference — generated documentation for all public C++ and QML types, properties, methods, signals, and enums."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# API reference

The API reference documents public C++ and QML-facing types, properties,
methods, signals, and enums.

- [Class list](api/annotated.md)
- [Class index](api/classes.md)
- [Class hierarchy](api/hierarchy.md)
- [Namespaces](api/namespaces.md)
- [Files](api/files.md)

## Key types

- [`PlotView`](api/classQAccelPlot_1_1QAccelPlot.md) is the native plot canvas
  responsible for layout, navigation, coordinate conversion, the grid, and the
  series collection. The QML `Plot` component wraps it and is the recommended
  application-level entry point.
- [`Axis`](api/classQAccelPlot_1_1Axis.md) controls visible ranges, scaling,
  ticks, labels, panning, and zooming.
- [`LineCurve`](api/classQAccelPlot_1_1LineCurve.md) provides GPU-rendered
  lines, markers, data ingestion, effects, transitions, and hover handling.
- [`RectangleList`](api/classQAccelPlot_1_1RectangleList.md) renders
  collections of data-space rectangles.
- [`DataAnchor`](api/classQAccelPlot_1_1DataAnchor.md) positions QML overlays
  at data coordinates.
- [`PlotMouseEvent`](api/classQAccelPlot_1_1PlotMouseEvent.md) exposes mouse
  positions in plot and data coordinates.

## Feeding curve data

Choose the simplest API appropriate for the data source:

- Use `setData()` for QML data or conventional C++ point containers.
- Use the moved-buffer `setDataF()` overload for large interleaved
  `std::vector<float>` buffers produced on the curve's thread.
- Use [`postData()`](api/classQAccelPlot_1_1LineCurve.md#function-postdata) to
  transfer a completed buffer safely from a worker thread.
- Use `setDataFNoRange()` only when axis data ranges are maintained separately.

See the [performance guide](performance.md) for a detailed comparison of the
available data paths.

## Runtime requirements

QAccelPlot requires Qt 6.2 or newer and a hardware Qt Quick Scene Graph
backend, such as OpenGL, Direct3D, Vulkan, or Metal. The Qt Quick software
backend cannot render the custom curve shaders.

Never mutate a `QQuickItem` directly from a worker thread. Use
[`LineCurve::postData()`](api/classQAccelPlot_1_1LineCurve.md#function-postdata)
or queue the operation to the item's thread.

To preview the complete site locally, install the documentation dependencies
and run MkDocs:

```sh
python -m pip install -r docs/requirements.txt
python scripts/build-docs.py serve
```
