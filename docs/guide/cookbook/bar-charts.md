---
description: "Draw bar charts in QAccelPlot with BarSeries: vertical and horizontal bars, grouped bars, custom baselines, colors by category, hover, and C++ data paths."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Bar charts

Use [`QAccelPlot.BarSeries`][bar-series] for bar charts. Each bar is a
(position, value) pair. The series uploads two floats per bar and builds the
bars in the shader, so changing `barWidth`, `barOffset`, or `baselineValue`
does not re-upload the data.

## Draw bars

```qml
QAccelPlot.Plot {
    id: plot
    xAxis: QAccelPlot.Axis { viewportMin: -0.5; viewportMax: 3.5 }
    yAxis: QAccelPlot.Axis { viewportMin: 0; viewportMax: 10 }

    QAccelPlot.BarSeries {
        name: "Sales"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        Component.onCompleted: setData([4, 7, 5, 9])
    }
}
```

`setData()` takes numbers, placed at positions 0, 1, 2, …, or objects with
`position`, `value`, and an optional `category`:

```qml
setData([{ position: 2020, value: 4 }, { position: 2021, value: 7 }])
```

| Property | Default | Effect |
| --- | --- | --- |
| `barWidth` | 0.8 | Bar width in position-axis data units |
| `barOffset` | 0 | Shift of every bar along the position axis |
| `baselineValue` | 0 | Value the bars start from |
| `minimumWidth` | 1 | Minimum drawn width in pixels |
| `orientation` | `Qt.Vertical` | `Qt.Horizontal` puts positions on the Y axis |

Values below `baselineValue` extend the bar the other way. Set
`baselineValue: -Infinity` to start the bars at the plot edge. On a
logarithmic value axis, a baseline at or below zero also starts at the plot
edge.

For category labels, use a [`TextTickLabelFormatter`](axis-formats.md) on the
position axis.

## Group bars

Use one series per group member. Split the group width between them and
offset each by its index:

```qml
Repeater {
    model: ["North", "South", "West"]

    QAccelPlot.BarSeries {
        required property int index
        required property string modelData

        name: modelData
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        barWidth: 0.8 / 3
        barOffset: (index - 1) * barWidth
        Component.onCompleted: setData(revenue[index])
    }
}
```

## Color bars by category

Give bars a `category` and list the colors in `categoryColors`. Bars without a
category, or with one outside the list, use `color`.

```qml
QAccelPlot.BarSeries {
    categoryColors: ["#4ade80", "#f87171"]
    Component.onCompleted: setData(profit.map((value, month) => ({
        position: month,
        value: value,
        category: value >= 0 ? 0 : 1
    })))
}
```

`setCategories()` replaces the categories without changing the data. Outlines
are set with `border.width` and `border.color`.

## Hover

`hoveredIndex` is the bar under the cursor, or -1. `barAt(index)` returns its
`position`, `value`, and `category`. Set `hoverColor` to highlight it.
`minimumWidth` also widens the hover area.

## Feed data from C++

Pass a vector of interleaved `(position, value)` pairs to `setData()`:

```cpp
// Three bars: 4 at position 0, 7 at position 1, 5 at position 2.
auto data = std::vector<double>{0.0, 4.0, 1.0, 7.0, 2.0, 5.0};
bars->setData(std::move(data), 3);
```

`setData()` keeps doubles and uploads them relative to an origin near the
data, so large positions such as epoch timestamps stay precise.

For the highest throughput, use the float paths (`setDataF()`,
`setDataFNoRange()`) and `postData()` from a worker thread; see the
[Performance guide](../performance.md).

A bar with a NaN or infinite position, or a NaN value, is not drawn. An
infinite value reaches the plot edge.

Complete source:

- [`examples/plot_types/bar_chart`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/plot_types/bar_chart):
  grouped and horizontal bars, a profit chart colored against a target
  baseline, and a hover tooltip.

[bar-series]: ../api/classQAccelPlot_1_1BarSeries.md
