---
description: "Draw error bars in QAccelPlot with ErrorBarSeries: symmetric and asymmetric errors, vertical and horizontal error bars, caps, error bars on grouped bars, hover, and data from C++."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Error bars

Use [`QAccelPlot.ErrorBarSeries`][error-bar-series] to draw a line from a low
to a high bound at each data point, capped at both ends. It does not draw the
values: draw them with a `PointCloud`, `LineCurve`, or `BarSeries`.

Panning and zooming error bars costs nothing on the CPU, whatever their number.

## Draw error bars under points

```qml
QAccelPlot.Plot {
    id: plot
    xAxis: QAccelPlot.Axis { viewportMin: 0; viewportMax: 4 }
    yAxis: QAccelPlot.Axis { viewportMin: 0; viewportMax: 10 }

    QAccelPlot.ErrorBarSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        Component.onCompleted: setErrorData([1, 2, 3], [4.2, 6.1, 5.7], [0.5, 0.8, 0.4])
    }

    QAccelPlot.PointCloud {
        name: "Measurements"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        Component.onCompleted: setData([Qt.point(1, 4.2), Qt.point(2, 6.1), Qt.point(3, 5.7)])
    }
}
```

Declare the error bars before the points, so the markers are drawn and hovered
on top. Error bars have no legend entry; name the series that draws the values.

## Set errors or bounds

Each data point is `(x, y, low, high)`. `low` and `high` are absolute bounds.

| Function | Takes |
| --- | --- |
| `setErrorData(xs, ys, errors)` | One error on both sides of each value |
| `setErrorData(xs, ys, minuses, pluses)` | An error below and one above each value |
| `setData(xs, ys, lows, highs)` | The bounds |
| `appendErrorData(x, y, error)`, `appendErrorData(x, y, minus, plus)`, `appendData(x, y, low, high)` | One data point |

- The value is `y` for vertical and `x` for horizontal error bars.
- The signs of the errors are ignored.
- `errorBarAt(index)` returns `x`, `y`, `low`, and `high`.

## Horizontal error bars

`orientation: Qt.Horizontal` places the bounds on the X axis. The data points
are still `(x, y, low, high)`, so the same `xs` and `ys` can feed a vertical and
a horizontal series:

```qml
QAccelPlot.ErrorBarSeries {
    orientation: Qt.Horizontal
    Component.onCompleted: setErrorData(xs, ys, xErrors)
}

QAccelPlot.ErrorBarSeries {
    Component.onCompleted: setErrorData(xs, ys, yErrorsBelow, yErrorsAbove)
}
```

Changing `orientation` keeps the stored bounds. Set the data again when the
bounds belong to the other axis.

## Style

| Property | Default | Notes |
| --- | --- | --- |
| `color` | `Colors.dark.seriesPrimary` | |
| `lineWidth` | `1.5` | Pixels, for the line and the caps. |
| `capLength` | `8` | Pixels across the line. `0` hides the caps. |
| `offset` | `0` | Data units across the bounds. |

## Error bars on grouped bars

Give each `ErrorBarSeries` the `barOffset` of its `BarSeries`:

```qml
QAccelPlot.BarSeries {
    id: lowDose
    barWidth: 0.25
    barOffset: 0.25
    Component.onCompleted: setData(means)
}

QAccelPlot.ErrorBarSeries {
    offset: lowDose.barOffset
    color: "white"
    Component.onCompleted: setErrorData([0, 1, 2, 3], means, deviations)
}
```

Auto-ranging ignores `offset`.

## Feed data from C++

Data points are interleaved `(x, y, low, high)` quadruples:

```cpp
auto dataPoints = std::vector<double>{};
dataPoints.reserve(count * 4);
for (auto i = 0; i < count; ++i) {
    dataPoints.insert(dataPoints.end(), {x[i], mean[i], mean[i] - sigma[i], mean[i] + sigma[i]});
}
errorBars->setData(std::move(dataPoints), count);
```

`setErrorData()` also takes `std::vector<double>` arguments. For float buffers,
worker-thread handoff, and data bounds, see
[Select the data path](../performance.md#select-the-data-path).

## Invalid data points

- An error bar is not drawn when its position (`x` for vertical, `y` for
  horizontal error bars) is `NaN` or `±Inf`, or not strictly positive on a
  logarithmic axis, or when its value is `NaN`.
- A `NaN` bound leaves out that half: the line runs from the value to the other
  bound. A `NaN` error gives a `NaN` bound.
- An infinite bound, and a bound that is not strictly positive on a
  logarithmic axis, extends the line past the plot edge without a cap.
- Auto-ranging counts every valid coordinate on its own.

## Hover

`hoveredIndex` is the error bar within `hoverRadius` pixels (default 6) of the
mouse pointer, and `hoverColor` draws it. Only the topmost series under the
mouse pointer is hovered: over a marker, the series drawn on top takes hover.
Set `hoverEnabled: false` to leave hover to the series beneath.

Error bars and the series drawn on top usually share indices, so one tooltip
can read both:

```qml
readonly property int hovered: points.hoveredIndex >= 0 ? points.hoveredIndex : errorBars.hoveredIndex
readonly property var errorBar: hovered >= 0 ? errorBars.errorBarAt(hovered) : null
```

`inspection.recordAt(index)` and `inspection.recordAtPosition(position)`
return the same fields as `errorBarAt()`.

## Limits

At most 16,777,216 (2^24) error bars are drawn, fewer on GPUs whose maximum
texture size is below 8192. Error bars beyond the limit are not drawn or
hovered, and a warning is logged once.

Complete source: [`examples/plot_types/error_bars`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/plot_types/error_bars)

[error-bar-series]: ../api/classQAccelPlot_1_1ErrorBarSeries.md
