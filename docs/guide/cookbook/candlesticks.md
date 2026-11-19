---
description: "Draw candlestick and OHLC charts in QAccelPlot with CandlestickSeries: open, high, low, and close data, candle width and spacing, trading-time axes without gaps, volume bars, live candles, hover, animated updates, and data from C++."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: MIT
-->

# Candlesticks

Use [`QAccelPlot.CandlestickSeries`][candlestick-series] to draw the open,
high, low, and close of each data point as a candle: a body from the open to
the close over a wick from the low to the high, or an OHLC bar.

Panning and zooming candles costs nothing on the CPU, whatever their number.

## Draw candles

```qml
QAccelPlot.Plot {
    id: plot
    xAxis: QAccelPlot.Axis { viewportMin: -1; viewportMax: 4 }
    yAxis: QAccelPlot.Axis { viewportMin: 9; viewportMax: 12 }

    QAccelPlot.CandlestickSeries {
        name: "Price"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        Component.onCompleted: setData([0, 1, 2, 3],
                                       [10.0, 11.0, 10.4, 10.2],  // open
                                       [11.2, 11.4, 10.9, 11.6],  // high
                                       [9.8, 10.3, 9.9, 10.0],    // low
                                       [11.0, 10.4, 10.2, 11.3])  // close
    }
}
```

A candle whose close is at or above its open is an up candle, drawn in
`upColor`; any other is a down candle, drawn in `downColor`. The wick takes the
candle's color.

## Set data

Each data point is `(x, open, high, low, close)`.

| Function | Takes |
| --- | --- |
| `setData(xs, opens, highs, lows, closes)` | Every candle; the shortest list sets the count |
| `appendData(x, open, high, low, close)` | One candle at the end |
| `replaceData(index, x, open, high, low, close)` | One candle, replacing candle `index` |
| `clearData()` | Nothing; removes every candle |

`candleAt(index)` returns `x`, `open`, `high`, `low`, and `close`.

## Choose a style

| `style` | Draws |
| --- | --- |
| `CandlestickSeries.Candlestick` (default) | A body from the open to the close over a wick from the low to the high |
| `CandlestickSeries.Ohlc` | A line from the low to the high, the open as a tick to the left and the close as a tick to the right |

Switching the style only changes a shader uniform.

## Candle width and spacing

A candle is `candleWidth` times `spacing` wide. Each OHLC tick is half that.

| Property | Default | Notes |
| --- | --- | --- |
| `candleWidth` | `0.8` | Fraction of `spacing`. |
| `spacing` | The smallest positive gap between consecutive x values | x data units. Assign a value to pin it; assign `undefined` to return to the default. |
| `minimumWidth` | `1` | Pixels. Candles never get narrower. |
| `lineWidth` | `1` | Pixels, for wicks, OHLC lines, and ticks. A body is at least this tall. |

- The default `spacing` ignores non-finite x values and takes the gaps in stored
  order. Without a positive gap, such as with one candle, it is 1.
- Reading `spacing` returns the spacing in effect. While the default applies,
  `spacingChanged` is emitted on every data change.
- Pin `spacing` when the x values have outliers or gaps, such as timestamps
  that skip weekends, or when a single streamed candle has no neighbor yet.

### Skip non-trading time

Use the candle's index as x, so nights, weekends, and holidays take no room, and
label the ticks with a formatter that maps the index to its time:

```qml
xAxis: QAccelPlot.Axis {
    ticker.tickLabelFormatter: QAccelPlot.NumericTickLabelFormatter {
        // times holds the start time of each candle.
        tickLabel: function (index) {
            const candle = Math.round(index);
            if (Math.abs(index - candle) > 1e-6 || candle < 0 || candle >= times.length) {
                return "";
            }
            return Qt.formatDateTime(times[candle], "d MMM HH:mm");
        }
    }
}
```

## Volume bars

Draw volume with a `BarSeries` on a second y axis, declared before the candles
so they are drawn and hovered on top. Bind its `barWidth` to the candle width:

```qml
QAccelPlot.BarSeries {
    xAxis: plot.xAxis
    yAxis: plot.y2Axis
    barWidth: candles.candleWidth * candles.spacing
    hoverEnabled: false
}

QAccelPlot.CandlestickSeries {
    id: candles
    xAxis: plot.xAxis
    yAxis: plot.yAxis
}
```

Set the volume axis' `viewportMax` to a few times the largest volume to keep
the bars in the lower part of the plot.

## Update a live candle

Replace the last candle as prices arrive, and append one when it closes:

```cpp
candles->replaceData(last, x, open, high, low, close);
candles->appendData(x + 1, close, close, close, close);
```

Both update the data ranges and the default `spacing` in place, except:

- Replacing a candle with another `x` rescans the x values for the default
  `spacing`.
- When the default `spacing` changes, or a replaced candle no longer reaches
  an edge of the data ranges that it reached before, the next read of the data
  ranges rescans the candles.

## Style

| Property | Default | Notes |
| --- | --- | --- |
| `upColor` | `Colors.dark.candleUp` | Up candles. |
| `downColor` | `Colors.dark.candleDown` | Down candles. |
| `hoverColor` | Invalid | Draws the hovered candle; invalid leaves it in its own color. |

A `Glow` effect surrounds each candle in its own color, unless `Glow.color` is set.
The legend shows an up candle next to a down candle.

## Feed data from C++

Data points are interleaved `(x, open, high, low, close)` quintuples:

```cpp
auto dataPoints = std::vector<double>{};
dataPoints.reserve(count * 5);
for (auto i = 0; i < count; ++i) {
    dataPoints.insert(dataPoints.end(), {x[i], open[i], high[i], low[i], close[i]});
}
candles->setData(std::move(dataPoints), count);
```

`setData()` also takes five `std::vector<double>` arguments. For float buffers,
worker-thread handoff, and data bounds, see
[Select the data path](../performance.md#select-the-data-path).

## Animate data changes

Set `enterTransition`, `updateTransition`, and `exitTransition`, as for a
[`LineCurve`](styling.md#animated-data-changes).

- `MorphTransition` moves each kept candle to the new x, open, high, low, and
  close of the same index.
- `GrowTransition` grows the high, low, and close from each candle's open; with
  a `baseline`, the open grows from it too. The x stays.
- `DrawTransition` shows the candles one after another.
- `count`, `candleAt()`, the data ranges, `spacing`, and inspection use the new
  data from the start of the animation. Hover picks the candles as drawn.
- `appendData()` and `replaceData()` show the new data at once; `clearData()`
  runs the exit transition.
- While a transition is enabled, the float paths store doubles.

## Invalid data points

- A candle is not drawn when its `x` is `NaN` or `±Inf`, or not strictly
  positive on a logarithmic axis, or when its open or close is `NaN`.
- A `NaN` high or low leaves out that half of the wick or OHLC line, which then
  ends at the body.
- An infinite value, and a value that is not strictly positive on a logarithmic
  axis, extends past the plot edge.
- Inconsistent values, such as a high below the body, are drawn as given.
- On a logarithmic x axis, a candle whose left edge would be at or below 0
  starts at its `x`.
- Auto-ranging counts every valid coordinate and widens x by half a candle on
  each side.

## Hover

`hoveredIndex` is the candle whose body, wick, line, or ticks lie within
`hoverRadius` pixels (default 4) of the mouse pointer; the nearest one wins, and
the highest index on a tie. Read it for a tooltip:

```qml
readonly property var hovered: candles.hoveredIndex >= 0 ? candles.candleAt(candles.hoveredIndex) : null
```

`inspection.recordAt(index)` and `inspection.recordAtPosition(position)`
return the same fields as `candleAt()`. `PlotInspector` does not inspect
candlestick series.

## Limits

At most 16,777,216 (2^24) candles are drawn, fewer on GPUs whose maximum
texture size is below 10240. Candles beyond the limit are not drawn or hovered,
and a warning is logged once.

Complete source: [`examples/plot_types/candlestick`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/plot_types/candlestick)

[candlestick-series]: ../api/classQAccelPlot_1_1CandlestickSeries.md
