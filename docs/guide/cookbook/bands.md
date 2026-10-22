---
description: "Draw confidence intervals, forecast ranges, and min/max or mean ± σ envelopes in QAccelPlot with BandSeries: fills between a low and a high value, edge lines, hover, and streaming millions of data points."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Bands

Use [`QAccelPlot.BandSeries`][band-series] to fill the area between a low and a
high value at each X: confidence and prediction intervals, tolerance masks, and
min/max or mean ± σ envelopes. Draw the center line with a separate
`LineCurve`.

Panning and zooming a band costs nothing on the CPU, whatever its size. Dashed
edge lines are the exception: zooming recomputes their dash positions in one
pass over the data points.

## Draw a band around a line

```qml
QAccelPlot.Plot {
    id: plot
    xAxis: QAccelPlot.Axis { viewportMin: 0; viewportMax: 10 }
    yAxis: QAccelPlot.Axis { viewportMin: 0; viewportMax: 10 }

    QAccelPlot.BandSeries {
        name: "90% interval"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "#4065b5ff"
        Component.onCompleted: setData([0, 5, 10], [2, 4, 3], [4, 7, 8])
    }

    QAccelPlot.LineCurve {
        name: "Forecast"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "#65b5ff"
        Component.onCompleted: setData([Qt.point(0, 3), Qt.point(5, 5.5), Qt.point(10, 5.5)])
    }
}
```

Declare the band before the line so the line is drawn on top. Nested intervals
are separate bands, widest first.

At each data point the band spans from the smaller to the larger of `low` and
`high`, so swapped values still fill correctly.

The default legend shows a `Fill` swatch in the band's `color`.

## Edge lines

Set `edges.width` to draw lines along the lower and upper edges:

```qml
QAccelPlot.BandSeries {
    edges.width: 1
    edges.color: "#b065b5ff"
    edges.lineStyle: QAccelPlot.DashLine { pattern: [4, 3] }
}
```

| Property | Default | Notes |
| --- | --- | --- |
| `edges.width` | `0` | No edge lines at 0. |
| `edges.color` | invalid | Uses the band's `color` at full opacity. |
| `edges.lineStyle` | `SolidLine` | `DashLine` and `NoLine` also work. |

## Feed data from C++

Data points are interleaved `(x, low, high)` triples. The data API matches
`LineCurve` with three values per data point:

```cpp
auto dataPoints = std::vector<double>{};
dataPoints.reserve(count * 3);
for (auto i = 0; i < count; ++i) {
    dataPoints.insert(dataPoints.end(), {x[i], mean[i] - 2 * sigma[i], mean[i] + 2 * sigma[i]});
}
band->setData(std::move(dataPoints), count);
```

For float buffers, worker-thread handoff, and data bounds, see
[Select the data path](../performance.md#select-the-data-path).

- `setData()` keeps doubles and uploads them relative to an origin near the
  viewport, so epoch timestamps stay precise. `setDataF()` stores floats.
- `postData()` hands a buffer over from any thread without copying it.
- `setData(xs, lows, highs)` takes three separate arrays, from QML or C++.
- `appendData(x, low, high)` adds one data point.

## Invalid data points

A data point is invalid when `x`, `low`, or `high` is `NaN` or `±Inf`, or is not
strictly positive on a logarithmic axis. The fill and the edge lines leave a
gap there. Auto-ranging skips invalid values one by one.

## Hover

`hovered` is `true` while the mouse pointer is inside the band or on an edge
line. Only the topmost series under the mouse pointer is hovered, so nested
bands highlight one at a time. Set `hoverEnabled: false` on a series drawn over
a band to leave hover to the band:

```qml
QAccelPlot.BandSeries {
    color: hovered ? "#8065b5ff" : "#4065b5ff"
    edges.width: hovered ? 1 : 0
}
```

## Limits

At most 16,777,216 (2^24) data points are drawn, fewer on GPUs whose maximum
texture size is below 6144. Data points beyond the limit are not drawn, and a
warning is logged once.

Complete source: [`examples/plot_types/bands`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/plot_types/bands)

[band-series]: ../api/classQAccelPlot_1_1BandSeries.md
