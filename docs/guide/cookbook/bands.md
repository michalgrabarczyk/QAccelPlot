---
description: "Draw confidence intervals, forecast ranges, and min/max or mean ± σ envelopes in QAccelPlot with BandSeries: fills between a low and a high value, edge lines, hover, and streaming millions of samples."
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

Samples live in a GPU data texture, and the vertex buffer holds only sample
indices. Panning and zooming change shader uniforms only, so a band of millions
of samples costs nothing on the CPU per frame. Dashed edge lines are the
exception: zooming recomputes their dash positions, one pass over the samples.

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

At each sample the band spans from the smaller to the larger of `low` and
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

The edge lines read the band's data texture; the samples are uploaded once.

## Feed data from C++

Samples are interleaved `(x, low, high)` triples. The data API matches
`LineCurve` with three values per sample:

```cpp
auto samples = std::vector<double>{};
samples.reserve(count * 3);
for (auto i = 0; i < count; ++i) {
    samples.insert(samples.end(), {x[i], mean[i] - 2 * sigma[i], mean[i] + 2 * sigma[i]});
}
band->setData(std::move(samples), count);
```

For float buffers, worker-thread handoff, and skipping range scans, see
[Select the data path](../performance.md#select-the-data-path).

- `setData()` keeps doubles and uploads them relative to an origin near the
  viewport, so epoch timestamps stay precise. `setDataF()` stores floats.
- `postData()` hands a buffer over from any thread without copying it.
- `setData(xs, lows, highs)` takes three separate arrays, from QML or C++.
- `appendData(x, low, high)` adds one sample.
- `NoRange` variants leave the axes' data ranges to you.

## Invalid samples

A sample is invalid when `x`, `low`, or `high` is `NaN` or `±Inf`, or is not
strictly positive on a logarithmic axis. The fill and the edge lines leave a
gap there. Auto-ranging skips invalid values one by one.

## Hover

`hovered` is `true` while the cursor is inside the band or on an edge line.
Only the topmost series under the cursor is hovered, so nested bands highlight
one at a time. Set `hoverRadius: 0` on a `LineCurve` drawn over a band to leave
hover to the band:

```qml
QAccelPlot.BandSeries {
    color: hovered ? "#8065b5ff" : "#4065b5ff"
    edges.width: hovered ? 1 : 0
}
```

## Limits

At most 16,777,216 (2^24) samples are drawn, fewer on GPUs whose maximum
texture size is below 6144. Samples beyond the limit are not drawn, and a
warning is logged once.

Complete source:

- [`examples/plot_types/bands`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/plot_types/bands):
  a forecast with nested prediction intervals, and a rolling mean ± 2σ band
  over 10,000 samples of a simulated sensor.

[band-series]: ../api/classQAccelPlot_1_1BandSeries.md
