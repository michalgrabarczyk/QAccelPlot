---
description: "QAccelPlot architecture explained — how Plot, PlotView, Axes, LineCurve, RectangleList, DataAnchor, and the Qt Scene Graph rendering pipeline work together."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Concepts and architecture

QAccelPlot separates plot composition, coordinate mapping, data ownership, and
rendering. Understanding those boundaries makes both QML composition and
high-rate C++ updates straightforward.

![QAccelPlot architecture](assets/architecture.svg)

## Plot and PlotView

`PlotView` is the C++ plot container. It owns the plot area, lays out primary,
secondary, and extra axes, exposes coordinate conversion helpers, handles the
default pan and zoom behavior, and discovers series added as child items.

`Plot` is the convenient QML wrapper used by most applications. It adds a
`Legend` and the `legendVisible` property while retaining the complete
`PlotView` interface.

The plot rectangle is available as [`plotRect`][plot-rect]. It excludes the space reserved
for axes and padding, and is useful when placing clipped overlays or custom
tools above the data.

## Axes and ranges

An `Axis` has two related ranges:

- The **viewport range** ([`viewportMin`][viewport-min], [`viewportMax`][viewport-max]) is currently visible.
- The **data range** ([`dataMin`][data-min], [`dataMax`][data-max]) describes the extent reported by
  attached series.

Panning and zooming change the viewport. Calling [`rescaleToData()`][rescale-to-data] copies the
current data range into the viewport. A series reports its extent through the
axes assigned to its [`xAxis`][series-x-axis] and [`yAxis`][series-y-axis] properties.

The primary axes are [`xAxis`][plot-x-axis] and [`yAxis`][plot-y-axis].
[`x2Axis`][plot-x2-axis] and [`y2Axis`][plot-y2-axis] provide the opposite
sides, while [`extraAxes`][extra-axes] supports additional independently scaled
axes. Assigning a named axis automatically selects its side: `xAxis` is bottom,
`x2Axis` is top, `yAxis` is left, and `y2Axis` is right. Extra axes specify
their own `side`. Each series explicitly selects the axes it uses.

`AxisTicker` controls tick count, subticks, lengths, colors, fonts, rotation,
and label formatting. Built-in formatters cover numeric, date/time,
logarithmic, and categorical labels. A `TickLabelFormatter` can also invoke a
JavaScript callback for application-specific labels.

## Series

`PlotSeries` contains the properties shared by data-bearing items: its name,
axes, plot rectangle, and legend symbol.

`LineCurve` renders a line, optional point markers, line styles, gradients, and
fills. It stores points internally as interleaved floats. Data can arrive from
QML for convenience or through C++ buffer APIs for high throughput.

`RectangleList` renders a set of data-space rectangles and reports its hovered
rectangle index. It is useful for ranges, events, bars, or large collections of
simple rectangular shapes.

## Composition and overlays

The grid and series render within the plot, while ordinary QML items can be
placed above them. `DataAnchor` maps one or two data coordinates into a QML
item's geometry, allowing labels, event lines, regions, and drag handles to
remain attached to the data while the user pans or zooms.

For more specialized interaction, `PlotView` provides [`dataToPixelX()`][data-to-pixel-x],
[`dataToPixelY()`][data-to-pixel-y], [`pixelToDataX()`][pixel-to-data-x],
[`pixelToDataY()`][pixel-to-data-y], and [`isInsidePlotArea()`][is-inside-plot-area].
Plot mouse events can be accepted by a custom tool to
prevent the default pan behavior.

## Rendering and threads

Qt Quick owns the scene. QML objects and public setters normally run on the UI
thread, while Qt synchronizes scene state to the Scene Graph render thread.
QAccelPlot curve renderers create persistent Scene Graph nodes and use
precompiled shaders for the active graphics API.

The software Qt Quick backend cannot execute QAccelPlot's custom curve shaders.
Data production may happen on a worker thread, but UI objects must not be
mutated directly from that worker. Use [`LineCurve::postData()`][post-data] or queue the
handoff to the UI thread. See [Background data production](cookbook/background-data.md).

## Where to find detail

- Use this guide for workflows and design decisions.
- Use the [Cookbook](cookbook/index.md) for complete patterns.
- Use the [API reference](api.md) for individual properties, methods, and enums.
- Use the repository's
  [`examples`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples)
  for runnable applications.

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
