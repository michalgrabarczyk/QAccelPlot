---
description: "Add data-attached annotations, event regions, measurement tools, and custom mouse interaction to QAccelPlot charts using DataAnchor and RectangleList."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Annotations and tools

## Attach QML items to data

Create a clipped layer matching [`plotRect`][plot-rect], then place `QAccelPlot.DataAnchor`
items inside it. Because the layer's origin is the plot area's top-left, pass a
local plot rectangle to each anchor:

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

[`QAccelPlot.DataAnchor`][data-anchor] maps a data-space point, line, or rectangular extent
to its item geometry. It can contain any QML content—such as a `Rectangle`,
`Canvas`, `Text`, or interaction handlers—which then follows panning and
zooming. Set matching X and/or Y coordinates for point or line anchors; use
all four bounds for a rectangle or range. This approach suits a modest number
of rich, individually interactive annotations: each `QAccelPlot.DataAnchor`
and its QML content adds `QQuickItem`s to the scene. For thousands of simple
regions, prefer [`QAccelPlot.RectangleList`][rectangle-list], which represents the collection
with one `QQuickItem`.

## Highlight many data regions

Use [`QAccelPlot.RectangleList`][rectangle-list] for many data-space rectangles, such as event
windows or maintenance intervals, instead of creating one QML item per region.
Its [`setData()`][rectangle-list-set-data] method accepts rectangle bounds, and
[`hoveredIndex`][hovered-index] identifies the region under the pointer for a
tooltip or selection:

```qml
QAccelPlot.RectangleList {
    xAxis: plot.xAxis
    yAxis: plot.yAxis
    color: "goldenrod"

    Component.onCompleted: setData([
        { x1: 8, y1: -1.2, x2: 12, y2: 1.2 },
        { x1: 19, y1: -1.2, x2: 22, y2: 1.2 }
    ])
}
```

The hover lookup uses a spatial grid after data changes, so it remains suitable
for large collections. See [Hover interactions](../performance.md#hover-interactions)
for the performance characteristics.

## Build interactive tools

The plot exposes both conversion directions:

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

Use [`dataToPixelX()`][data-to-pixel-x] and [`dataToPixelY()`][data-to-pixel-y]
to position handles. Use [`pixelToDataX()`][pixel-to-data-x] and
[`pixelToDataY()`][pixel-to-data-y] while dragging them. Accept a plot event
when the tool has consumed it so the same gesture does not start default
panning.

When a tool lives in a layer translated to [`plotRect`][plot-rect], convert between plot and
layer coordinates by adding or subtracting the plot rectangle origin. The
complete interactive example wraps those conversions in a small layer API.

Complete sources:

- [`examples/annotations`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/annotations)
- [`examples/interactive_tools`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/interactive_tools)

[plot-rect]: ../api/classQAccelPlot_1_1QAccelPlot.md#property-plotrect-12
[data-to-pixel-x]: ../api/classQAccelPlot_1_1QAccelPlot.md#function-datatopixelx
[data-to-pixel-y]: ../api/classQAccelPlot_1_1QAccelPlot.md#function-datatopixely
[pixel-to-data-x]: ../api/classQAccelPlot_1_1QAccelPlot.md#function-pixeltodatax
[pixel-to-data-y]: ../api/classQAccelPlot_1_1QAccelPlot.md#function-pixeltodatay
[data-anchor]: ../api/classQAccelPlot_1_1DataAnchor.md
[rectangle-list]: ../api/classQAccelPlot_1_1RectangleList.md
[rectangle-list-set-data]: ../api/classQAccelPlot_1_1RectangleList.md#function-setdata
[hovered-index]: ../api/classQAccelPlot_1_1RectangleList.md#property-hoveredindex-12
