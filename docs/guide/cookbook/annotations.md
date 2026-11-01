---
description: "Add data-attached annotations, event regions, state timelines, measurement tools, and custom mouse interaction to QAccelPlot charts using DataAnchor and RectangleSeries."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Annotations and tools

## Attach QML items to data

Place `QAccelPlot.DataAnchor` items in a clipped layer that matches
[`plotRect`][plot-rect]. The layer's origin is the plot area's top-left, so pass
each anchor a local rectangle:

```qml
Item {
    id: annotationLayer
    z: 1
    x: plot.plotRect.x
    y: plot.plotRect.y
    width: plot.plotRect.width
    height: plot.plotRect.height
    clip: true

    QAccelPlot.DataAnchor {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        plotRect: Qt.rect(0, 0, annotationLayer.width, annotationLayer.height)
        dataX1: 42
        dataY1: plot.yAxis.viewportMin
        dataX2: 42
        dataY2: plot.yAxis.viewportMax

        Rectangle {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.left
            width: 3
            color: "tomato"
        }
    }
}
```

[`DataAnchor`][data-anchor] maps a data-space point, line, or rectangle to its
item geometry. Its children (`Rectangle`, `Text`, `Canvas`, input handlers)
follow panning and zooming. Set matching X and/or Y coordinates for a point or
line; set all four bounds for a rectangle.

Each `DataAnchor` and its children add `QQuickItem`s, so use it for a moderate
number of rich annotations. For thousands of simple regions, use
[`RectangleSeries`][rectangle-series], which is a single item.

## Highlight many data regions

[`RectangleSeries`][rectangle-series] draws many data-space rectangles, such as
event windows or machine states. [`setData()`][rectangle-series-set-data] takes
rectangle bounds. An omitted or infinite edge reaches the plot edge, so
`{ x1: 8, x2: 12 }` is a full-height span:

```qml
QAccelPlot.RectangleSeries {
    xAxis: plot.xAxis
    yAxis: plot.yAxis
    color: "goldenrod"

    Component.onCompleted: setData([
        { x1: 8, x2: 12 },
        { x1: 19, x2: 22 }
    ])
}
```

Infinite edges don't change the axes' data ranges.

| Property | Use |
| :--- | :--- |
| `categories`, `category` | Per-rectangle color and name: a rectangle's `category` picks the entry with that `value`; others use `color` |
| `minimumWidth`, `minimumHeight` | Minimum drawn size in pixels (default 1), so short events stay visible |
| `border.width`, `border.color` | Outline inside each rectangle, e.g. to separate adjacent states |
| `hoverColor` | Fill of the hovered rectangle |
| [`hoveredIndex`][hovered-index], `rectangleAt()` | The rectangle under the mouse pointer and its bounds, for tooltips |

A state timeline puts one lane per row, colored by category:

```qml
QAccelPlot.RectangleSeries {
    xAxis: plot.xAxis
    yAxis: plot.yAxis
    categories: [
        QAccelPlot.Category { value: 0; name: "Running"; color: "seagreen" },
        QAccelPlot.Category { value: 1; name: "Idle"; color: "slategray" },
        QAccelPlot.Category { value: 2; name: "Alarm"; color: "firebrick" }
    ]
    border.width: 1

    Component.onCompleted: setData([
        { x1: 0, x2: 40, y1: -0.4, y2: 0.4, category: "Running" },
        { x1: 40, x2: 43, y1: -0.4, y2: 0.4, category: "Alarm" },
        { x1: 0, x2: 25, y1: 0.6, y2: 1.4, category: 1 }
    ])
}
```

- Each `Category` has an integer `value` and an optional `name` and `color`.
  Values need not be consecutive, so they can be C++ enum values. Edits to a
  `Category`, such as a theme color, apply at once.
- In QML data, `category` is a `value` or a `name`. Names resolve when the data
  is set; an unknown name or a non-whole number is ignored with a warning.
- Rectangles without a category, or whose category has no color or is not
  listed, use `color`.
- `setDataPointCategories()` replaces the rectangles' categories without
  changing their bounds.
- `rectangleAt()` returns `category` and, for a named category, `categoryName`.
- The legend shows one row per category.

Label the lanes with a [`TextTickLabelFormatter`](axis-formats.md#categories) on the
Y axis. From C++, `setData(std::vector<double>&&, std::vector<int>&&, int)`
moves bounds and category values in without copying; `postData()` hands them
off from a worker thread.

RectangleSeries only accepts hover over its rectangles, so series underneath
still receive hover elsewhere. Hover lookup uses a spatial grid, so it stays
fast for large collections unless many rectangles are very wide or tall; see
[Hover interactions](../performance.md#hover-interactions).

## Build interactive tools

Plot mouse events carry pixel positions; convert them to data coordinates:

```qml
onMousePressed: event => {
    if (event.button === Qt.RightButton) {
        const dataX = pixelToDataX(event.x)
        const dataY = pixelToDataY(event.y)
        createToolAt(dataX, dataY)
        event.accept()
    }
}
```

Position handles with [`dataToPixelX()`][data-to-pixel-x] and
[`dataToPixelY()`][data-to-pixel-y]; convert drag positions back with
[`pixelToDataX()`][pixel-to-data-x] and [`pixelToDataY()`][pixel-to-data-y].
Call `event.accept()` when a tool handles an event so the gesture does not also
pan the plot or start rectangle zoom.

In a layer positioned at [`plotRect`][plot-rect], add or subtract the plot
rectangle origin to convert between plot and layer coordinates.

Complete sources:

- [`examples/interaction/annotations`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/interaction/annotations)
- [`examples/interaction/measurement_tools`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/interaction/measurement_tools)
- [`examples/plot_types/state_timeline`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/plot_types/state_timeline)

[plot-rect]: ../api/classQAccelPlot_1_1PlotView.md#property-plotrect-12
[data-to-pixel-x]: ../api/classQAccelPlot_1_1PlotView.md#function-datatopixelx
[data-to-pixel-y]: ../api/classQAccelPlot_1_1PlotView.md#function-datatopixely
[pixel-to-data-x]: ../api/classQAccelPlot_1_1PlotView.md#function-pixeltodatax
[pixel-to-data-y]: ../api/classQAccelPlot_1_1PlotView.md#function-pixeltodatay
[data-anchor]: ../api/classQAccelPlot_1_1DataAnchor.md
[rectangle-series]: ../api/classQAccelPlot_1_1RectangleSeries.md
[rectangle-series-set-data]: ../api/classQAccelPlot_1_1RectangleSeries.md#function-setdata-18
[hovered-index]: ../api/classQAccelPlot_1_1RectangleSeries.md#property-hoveredindex-12
