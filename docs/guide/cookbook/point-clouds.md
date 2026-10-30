---
description: "Render hundreds of thousands of unconnected scatter points in QAccelPlot with PointCloud: marker shapes, coloring points by value, color bars, logarithmic axes, hover picking, and streaming from worker threads."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Point clouds

Use [`QAccelPlot.PointCloud`][point-cloud] for scatter data whose points are
not connected. It draws each point as a GPU marker and scales to hundreds of
thousands of points updated every frame. A `LineCurve` with `NoLine` and
markers suits small scatter plots only.

## Draw a scatter plot

```qml
QAccelPlot.Plot {
    id: plot
    xAxis: QAccelPlot.Axis { viewportMin: -10; viewportMax: 10 }
    yAxis: QAccelPlot.Axis { viewportMin: -8; viewportMax: 8 }

    QAccelPlot.PointCloud {
        id: samples
        name: "Samples"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "#8065b5ff"
        marker.shape: QAccelPlot.MarkerShape.Circle
        marker.size: 2

        Component.onCompleted: setData([Qt.point(-2, 1), Qt.point(3, 4), Qt.point(1, -3)])
    }
}
```

`marker` takes the same `shape`, `size`, `filled`, and `strokeWidth` as on a
`LineCurve`, except the shape `None`; see
[Marker shapes](styling.md#marker-shapes).

Overlapping translucent markers blend separately, so dense regions read as
density. Points are drawn in data order.

The default legend shows the marker shape and `color`.

## Color points by value

Give each point a value and assign a [`Colormap`][colormap]:

```qml
QAccelPlot.PointCloud {
    xAxis: plot.xAxis
    yAxis: plot.yAxis
    colormap: QAccelPlot.Colormap { preset: QAccelPlot.Colormap.Viridis }
}
```

Without a colormap, or without per-point values, every point uses `color`.

The presets are `Viridis`, `Plasma`, `Inferno`, `Magma`, `Grayscale`, and
`Rainbow`; `reversed: true` runs a ramp from its last color to its first. For
a custom ramp, list `GradientStop`s in `stops`. The ramp follows bindings on
their `position` and `color`:

```qml
QAccelPlot.PointCloud {
    colormap: QAccelPlot.Colormap {
        stops: [
            GradientStop { position: 0.0; color: "#fde725" },
            GradientStop { position: 1.0; color: QAccelPlot.Colors.dark.seriesPrimary }
        ]
    }
}
```

`min` and `max` bound the value range. An unset bound is resolved from the
data, so series sharing one colormap can resolve different ranges.
`setRange(min, max)` sets both with one update. Set `norm` to `Colormap.Log`
when the values span decades; values at or below zero are then drawn with
`color`.

From QML, call `setValues()` after `setData()` with one value per point. From
C++, pass values together with the positions:

```cpp
cloud->setDataF(std::move(xyInterleaved), std::move(values), pointCount);
```

`setData()` takes the same arguments with a `std::vector<double>` of positions.
Values are single precision either way.

`dataValueMin` and `dataValueMax` expose the resolved bounds. Points with a
non-finite value use `color`.

## Show a color bar

[`QAccelPlot.ColorBar`][color-bar] draws the series' colormap with ticks for
the resolved value range:

```qml
QAccelPlot.Plot {
    id: plot

    QAccelPlot.PointCloud {
        id: cloud
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        colormap: QAccelPlot.Colormap { preset: QAccelPlot.Colormap.Plasma }
    }

    QAccelPlot.ColorBar {
        series: cloud
        label: "Intensity"
        x: plot.plotRect.left + 8
        y: plot.plotRect.top + 8
    }
}
```

- The bar is an ordinary item: position it with `x`, `y`, or anchors. It does
  not reserve layout space, and the default legend sits at the top right of
  the plot area.
- The default bar is vertical, with its minimum at the bottom.
  `orientation: Qt.Horizontal` puts the minimum on the left.
- Set `height` on a vertical bar, or `width` on a horizontal one, to change its
  length. Raise `tickLabelWidth` when the labels of a vertical bar are clipped.
- `ticker` configures the ticks, as on `Axis.ticker`.
- `interactive: true` lets the user pan the range with a left drag, zoom it
  with the wheel, and resolve it from the data again with a double-click. The
  gestures set the colormap's `min` and `max`, so leave both unbound.

## Stream point data

From a worker thread, hand over complete frames with `postData()`, which
moves the buffers:

```cpp
// Worker thread
cloud->postData(std::move(xy), std::move(values), pointCount);
```

Post no faster than frames are presented; see
[Background data production](background-data.md).

## Animate data changes

Assign a `MorphTransition` or a `DrawTransition` to `transition`, as for a
[`LineCurve`](styling.md#animated-data-changes).

- `MorphTransition` moves each point to the new position of the same index.
  Values change with it when the old and the new points both have them, so
  the colors blend through the colormap.
- `DrawTransition` shows the new points one after another.
- New points appear, and removed points disappear, at once.
- `count`, `pointAt()`, `valueAt()`, the data and value ranges, and inspection
  use the new data from the start of the animation. Hover picks the points as
  drawn.
- `setValues()` animates too. Called after `setData()`, it keeps that
  animation, with the points in their new colors.
- `clearData()` shows the new data at once.
- While a transition is enabled, the float paths store doubles.

## Invalid points

Non-finite coordinates, and non-positive ones on a logarithmic axis, are not
drawn, not picked, and do not contribute to axis data ranges. They stay in
storage, so indices are stable.

## Hover

`hoveredIndex` is the index of the valid point nearest to the mouse pointer
within `hoverRadius` pixels, or `-1`. Combine it with `pointAt()`, `valueAt()`,
and a `QAccelPlot.DataAnchor` to show a tooltip:

```qml
QAccelPlot.DataAnchor {
    readonly property point hovered: samples.hoveredIndex >= 0 ? samples.pointAt(samples.hoveredIndex) : Qt.point(0, 0)
    visible: samples.hoveredIndex >= 0
    xAxis: plot.xAxis
    yAxis: plot.yAxis
    plotRect: Qt.rect(0, 0, overlay.width, overlay.height)
    dataX1: hovered.x
    dataY1: hovered.y
    dataX2: hovered.x
    dataY2: hovered.y

    Text { text: "#" + samples.hoveredIndex }
}
```

Only positions near a point count as inside the series, so stacked series
underneath still receive hover events elsewhere. For the lookup cost, see
[Hover interactions](../performance.md#hover-interactions).

## Limits

- `setData()` keeps doubles and uploads positions relative to an origin near
  the data, so values far from zero such as epoch timestamps stay precise.
  `setDataF()` stores floats.
- The number of drawable points is bounded by the GPU's maximum texture size:
  about 22 million points with values, or 33 million without, on a GPU with
  an 8192-pixel texture limit, and twice that with a 16384-pixel limit. Extra
  points are not drawn, and a warning is logged once.

Complete source: [`examples/plot_types/point_cloud`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/plot_types/point_cloud)

[point-cloud]: ../api/classQAccelPlot_1_1PointCloud.md
[colormap]: ../api/classQAccelPlot_1_1Colormap.md
[color-bar]: ../api/classQAccelPlot_1_1ColorBar.md
