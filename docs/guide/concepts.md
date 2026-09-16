---
description: "QAccelPlot architecture explained — how Plot, PlotView, Axes, LineCurve, RectangleList, DataAnchor, and the Qt Scene Graph rendering pipeline work together."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Concepts and architecture

![QAccelPlot architecture](assets/architecture.svg)

## Plot and PlotView

`PlotView` is the C++ plot item. It lays out axes, converts between data and
pixel coordinates, handles pan and zoom, and collects child series.

`Plot` is the QML wrapper most applications use. It adds a `Legend` and
`legendVisible` to the full `PlotView` interface.

[`plotRect`][plot-rect] is the data area, excluding axes and padding. Use it to
place clipped overlays and custom tools.

## Axes and ranges

An `Axis` has two related ranges:

- The **viewport range** ([`viewportMin`][viewport-min], [`viewportMax`][viewport-max]) is visible.
- The **data range** ([`dataMin`][data-min], [`dataMax`][data-max]) is the extent reported by
  attached series.

Panning and zooming change the viewport. [`rescaleToData()`][rescale-to-data]
sets the viewport to fit the data range. A series reports its extent to the
axes set in its [`xAxis`][series-x-axis] and [`yAxis`][series-y-axis] properties.

| Plot property | Side |
| --- | --- |
| [`xAxis`][plot-x-axis] | Bottom |
| [`x2Axis`][plot-x2-axis] | Top |
| [`yAxis`][plot-y-axis] | Left |
| [`y2Axis`][plot-y2-axis] | Right |
| [`extraAxes`][extra-axes] | Set by each axis's `side` |

Each series selects its own axes.

`AxisTicker` controls tick count, subticks, lengths, colors, fonts, rotation,
and label formatting. Built-in formatters cover numeric, date/time,
logarithmic, and categorical labels; `TickLabelFormatter` also accepts a
JavaScript callback.

## Series

`PlotSeries` holds properties shared by all series: name, axes, plot rectangle,
and legend symbol.

`LineCurve` renders lines, markers, line styles, gradients, and fills. It
stores points as interleaved floats and accepts data from QML or C++ buffers.

`RectangleList` renders many data-space rectangles in one item and reports the
hovered rectangle index.

## Invalid samples and gaps

`LineCurve` applies one invalid-sample contract to rendering, gradient fills,
dash patterns, transitions, auto-ranging, and hover hit testing.

A sample is **invalid** when its X or Y coordinate is `NaN` or `±Inf`, or when a
coordinate is zero or negative on a logarithmic axis. Mark missing data by
inserting a sample with a `NaN` coordinate. JavaScript `null` and `undefined`
are not gap markers.

Gap rendering is configured through the curve's [`gaps`][line-curve] grouped
property. `gaps.nanMode` selects how the line and fill treat invalid samples:

```qml
QAccelPlot.LineCurve {
    gaps.nanMode: QAccelPlot.NanGapMode.Connect
}
```

| `gaps.nanMode` | Behavior |
| --- | --- |
| `NanGapMode.Break` (default) | No segment or fill is drawn to or from an invalid sample. Valid neighbors end squarely, and a valid sample isolated between two invalid samples draws no line. |
| `NanGapMode.Connect` | Invalid samples are skipped and the line, fill, and hit test join the nearest valid samples. |

Despite its name, `nanMode` applies to every invalid sample, including `±Inf`
and non-positive log-axis values.

In both modes:

- **Markers** are never drawn for invalid samples.
- **Auto-ranging** judges each coordinate on its own. A sample with a valid X
  and an invalid Y still extends the X range, so a dropout keeps its timestamp.
  If a dimension has no valid coordinate, the series clears that range.
- **Dash patterns** continue across a gap; the gap contributes no dash length.
- **`MorphTransition`** shows a sample that becomes invalid as a gap
  immediately, and a sample that becomes valid jumps to its target instead of
  animating from `NaN`.
- **Hover** never reports a hit on an invalid sample or across a break.
- The `NoRange` data APIs still render gaps; they only skip range calculation.

## Composition and overlays

Ordinary QML items can be placed above the grid and series. `DataAnchor` maps
one or two data coordinates to a QML item's geometry, so labels, event lines,
regions, and drag handles follow the data while panning and zooming.

For custom tools, `PlotView` provides [`dataToPixelX()`][data-to-pixel-x],
[`dataToPixelY()`][data-to-pixel-y], [`pixelToDataX()`][pixel-to-data-x],
[`pixelToDataY()`][pixel-to-data-y], and [`isInsidePlotArea()`][is-inside-plot-area].
Accepting a plot mouse event suppresses the default pan.

## Rendering and threads

QML objects and setters run on the UI thread; Qt synchronizes their state to
the Scene Graph render thread. Curve renderers keep persistent Scene Graph
nodes and use precompiled shaders for the active graphics API. The software
backend cannot run these shaders.

Produce data on a worker thread if needed, but never mutate UI objects from it.
Use [`LineCurve::postData()`][post-data] or a queued call. See
[Background data production](cookbook/background-data.md).

[plot-rect]: api/classQAccelPlot_1_1QAccelPlot.md#property-plotrect-12
[viewport-min]: api/classQAccelPlot_1_1Axis.md#property-viewportmin-12
[viewport-max]: api/classQAccelPlot_1_1Axis.md#property-viewportmax-12
[data-min]: api/classQAccelPlot_1_1Axis.md#property-datamin-12
[data-max]: api/classQAccelPlot_1_1Axis.md#property-datamax-12
[rescale-to-data]: api/classQAccelPlot_1_1Axis.md#function-rescaletodata
[series-x-axis]: api/classQAccelPlot_1_1PlotSeries.md#property-xaxis-12
[series-y-axis]: api/classQAccelPlot_1_1PlotSeries.md#property-yaxis-12
[plot-x-axis]: api/classQAccelPlot_1_1QAccelPlot.md#property-xaxis-12
[plot-y-axis]: api/classQAccelPlot_1_1QAccelPlot.md#property-yaxis-12
[plot-x2-axis]: api/classQAccelPlot_1_1QAccelPlot.md#property-x2axis-12
[plot-y2-axis]: api/classQAccelPlot_1_1QAccelPlot.md#property-y2axis-12
[extra-axes]: api/classQAccelPlot_1_1QAccelPlot.md#property-extraaxes-12
[data-to-pixel-x]: api/classQAccelPlot_1_1QAccelPlot.md#function-datatopixelx
[data-to-pixel-y]: api/classQAccelPlot_1_1QAccelPlot.md#function-datatopixely
[pixel-to-data-x]: api/classQAccelPlot_1_1QAccelPlot.md#function-pixeltodatax
[pixel-to-data-y]: api/classQAccelPlot_1_1QAccelPlot.md#function-pixeltodatay
[is-inside-plot-area]: api/classQAccelPlot_1_1QAccelPlot.md#function-isinsideplotarea
[post-data]: api/classQAccelPlot_1_1LineCurve.md#function-postdata
[line-curve]: api/classQAccelPlot_1_1LineCurve.md
