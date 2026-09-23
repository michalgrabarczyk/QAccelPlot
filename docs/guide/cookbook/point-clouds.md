---
description: "Render hundreds of thousands of unconnected scatter points in QAccelPlot with PointCloud: marker shapes, coloring points by value, logarithmic axes, hover picking, and streaming from worker threads."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Point clouds

Use [`QAccelPlot.PointCloud`][point-cloud] for scatter data whose points are
not connected: sensor returns, clustering results, particle positions, or
detector events. Every point is drawn as a GPU marker, and the series scales
to hundreds of thousands of points updated every frame.

A `LineCurve` with `NoLine` and markers can draw small scatter plots, but it
builds marker geometry on the CPU for every data change. `PointCloud` uploads
positions as a data texture instead, so a cloud of constant size costs one
texture upload per update.

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
        markerShape: QAccelPlot.PointCloud.Circle
        markerSize: 2

        Component.onCompleted: setData([Qt.point(-2, 1), Qt.point(3, 4), Qt.point(1, -3)])
    }
}
```

`markerSize` is the marker radius in pixels. Point clouds accept every
`LineCurve` marker shape; see the [shape table](styling.md#marker-shapes).
`Pixel` ignores `markerSize` and draws one pixel per point, which suits very
dense clouds.

Set `markerFilled: false` to draw closed shapes as outlines of
`markerStrokeWidth` pixels, exactly as on `LineCurve`. Hollow markers read well
on dense clouds, where filled shapes merge into a solid mass. Line shapes and
`Pixel` ignore both properties.

Translucent colors make dense regions read as density, because overlapping
markers blend. Points are drawn in data order, so put background points first
when they should appear underneath.

The default legend shows the marker shape and `color` for point clouds.

## Color points by value

Give each point a value and map it through a QML `Gradient`:

```qml
QAccelPlot.PointCloud {
    xAxis: plot.xAxis
    yAxis: plot.yAxis
    colormap: QAccelPlot.Colormap { preset: QAccelPlot.Colormap.Viridis }
}
```

A colormap is a color ramp plus the rule that places a value on it. Assigning one is what turns
on value coloring: without it, or without per-point values, every point uses `color`.

The built-in ramps are `Viridis`, `Plasma`, `Inferno`, `Magma`, `Turbo`, `Grayscale`, and
`Rainbow`. Prefer the perceptually uniform ones over `Rainbow`, whose uneven lightness invents
boundaries that are not in the data.

For a custom ramp, list the stops instead. The same stops can feed a color bar, so a swatch and
the points cannot drift apart:

```qml
Gradient {
    id: ramp
    GradientStop { position: 0.0; color: "#fde725" }
    GradientStop { position: 1.0; color: "#440154" }
}

QAccelPlot.PointCloud {
    colormap: QAccelPlot.Colormap { stops: ramp.stops }
}
Rectangle { gradient: ramp }   // the color bar
```

`min` and `max` bound the value range. Leave either unset and it is resolved from the data, so one
colormap can be shared between series that resolve different ranges. Set `norm` to
`Colormap.Log` when the values span decades, such as counts or intensities; values at or below
zero are then left unmapped and drawn with `color`.

A colormap has no geometry. Unlike `GradientFill` and `GradientStroke`, which paint a gradient
across the plot and so have a `direction`, a colormap is a one-dimensional ramp indexed by each
point's value, not by its position on screen. That is why there is no orientation here and no
radial form: only the stop colors are taken from the `Gradient` above.

From QML, call `setValues()` after `setData()` with one value per point. From
C++, pass values together with the positions:

```cpp
cloud->setDataF(std::move(xyInterleaved), std::move(values), pointCount);
```

`setData()` takes the same shape with a `std::vector<double>` of positions when
the coordinates need double precision. Values stay single precision either way,
because they only index the colormap.

With the default `DataRange` sources, the colormap spans the finite value
range of the data. The resolved bounds are exposed as `dataValueMin` and
`dataValueMax`, so a color bar can bind to them. Points with a non-finite value
fall back to `color`.

## Stream point data

`postData()` is thread-safe. It moves the buffers into a queued call, so a
worker thread can produce complete frames without copying them again:

```cpp
// Worker thread
cloud->postData(std::move(xy), std::move(values), pointCount);
```

Post a frame only after the window has presented the previous one, for
example by counting `QQuickWindow::frameSwapped`, so queued frames cannot pile
up when rendering falls behind. Use `setDataFNoRange()` when the application
manages `dataMin` and `dataMax` on the axes itself.

## Invalid points and logarithmic axes

Non-finite coordinates are kept in storage, so indices stay stable, but they
are not drawn, not picked, and do not contribute to axis data ranges. On a
logarithmic axis, non-positive coordinates are skipped the same way. You do not
need to filter them before handing data to the series.

## Hover and tooltips

`hoveredIndex` is the index of the valid point nearest to the pointer within
`hoverRadius` pixels, or `-1`. Combine it with `pointAt()`, `valueAt()`, and a
`QAccelPlot.DataAnchor` to show a tooltip:

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

The lookup uses a spatial index that is rebuilt on the first pointer move after
a data change. Only positions near a point count as inside the series, so
stacked series underneath still receive hover events elsewhere.

## Limits

- The GPU renders in single precision. `setData()` accepts doubles, from QML
  points or a `std::vector<double>`, and uploads positions relative to an
  origin taken from the first finite point, so values far from zero such as
  epoch timestamps keep their resolution. Logarithmic dimensions are never
  shifted. The `setDataF()` family takes floats and is not origin-shifted; use
  it when the data is already single precision.
- The number of drawable points is bounded by the GPU's maximum texture size:
  about 5.5 million points with values, or 8.3 million without, on a GPU with
  an 8192-pixel texture limit, and twice that with a 16384-pixel limit. Extra
  points are not drawn, and a warning is logged once.

Complete source:

- [`examples/point_cloud`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/point_cloud)

[point-cloud]: ../api/classQAccelPlot_1_1PointCloud.md
