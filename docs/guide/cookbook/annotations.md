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

[plot-rect]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_q_accel_plot.html#ad8106ed7a0613158e29ed0137edbd248
[data-to-pixel-x]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_q_accel_plot.html#a64aa564d1fae3a901d22897b8c70d514
[data-to-pixel-y]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_q_accel_plot.html#af6e974159b9a04649636768ff923d797
[pixel-to-data-x]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_q_accel_plot.html#a484d049dd7188040e0c450bb9590ac0c
[pixel-to-data-y]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_q_accel_plot.html#abd81402f2c90d2287bb1ca41f1a35e49
[data-anchor]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_data_anchor.html
[rectangle-list]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_rectangle_list.html
[rectangle-list-set-data]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_rectangle_list.html#aaead3aa540e3521e1e6a365bdc313d6c
[hovered-index]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_rectangle_list.html#afb85815f91d34b3a35bbf957e9aa5e09
