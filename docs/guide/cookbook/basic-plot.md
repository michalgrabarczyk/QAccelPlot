---
description: "Create a basic static QAccelPlot chart with axes, multiple line curves, markers, dashed lines, and an automatic legend."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Basic static plot

A `Plot` needs axes and one or more series. Each series names the axes that map
its data and optionally supplies a `name` for the legend.

```qml
import QtQuick
import QAccelPlot as QAccelPlot

QAccelPlot.Plot {
    id: plot

    xAxis: QAccelPlot.Axis {
        viewportMin: 0
        viewportMax: 24
        label: "Time of day (h)"
    }

    yAxis: QAccelPlot.Axis {
        viewportMin: 16
        viewportMax: 27
        label: "Temperature (°C)"
    }

    QAccelPlot.LineCurve {
        name: "Measured"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "tomato"
        lineWidth: 3
        Component.onCompleted: setData([
            Qt.point(0, 18), Qt.point(6, 19.5),
            Qt.point(12, 25), Qt.point(18, 23.5), Qt.point(24, 18)
        ])
    }

    QAccelPlot.LineCurve {
        name: "Forecast"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "dodgerblue"
        lineWidth: 2
        lineStyle: QAccelPlot.DashLine { pattern: [8, 5] }
        marker.shape: QAccelPlot.LineCurve.Diamond
        marker.size: 7
    }
}
```

`Plot` shows a legend entry for every series. Set `legendVisible: false` to
hide it, or assign `legend` to replace it.

QML [`setData()`](../api/classQAccelPlot_1_1LineCurve.md#function-setdata-18)
suits small or rarely changing data. For large or frequent updates, see
[Select the data path](../performance.md#select-the-data-path).

Complete source: [`examples/quickstart`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/quickstart)
