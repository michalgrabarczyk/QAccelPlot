---
description: "Add secondary and extra axes to a QAccelPlot chart for multiple units or sampling domains sharing a single plot area."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Multiple axes

Each series chooses its own X and Y axes, so several units or sampling domains
can share one plot area.

## Axis layout size

Each axis reserves 50 pixels by default. `layoutSize` sets the height of a
horizontal axis or the width of a vertical axis:

```qml
yAxis: QAccelPlot.Axis {
    layoutSize: 90
    label: "Temperature"
}

xAxis: QAccelPlot.Axis {
    layoutSize: 70
    label: "Time"
}
```

`layoutSize` does not grow with label text, fonts, or formatters. Choose a value
that fits the tick labels and axis title.

## Secondary axes

[`x2Axis`][x2-axis] is placed at the top and [`y2Axis`][y2-axis] on the right.

```qml
import QtQuick
import QAccelPlot as QAccelPlot

QAccelPlot.Plot {
    id: plot

    xAxis: QAccelPlot.Axis {}
    yAxis: QAccelPlot.Axis {}
    y2Axis: QAccelPlot.Axis {
        viewportMin: 0
        viewportMax: 100
        label: "Load (%)"
    }

    QAccelPlot.LineCurve {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
    }

    QAccelPlot.LineCurve {
        xAxis: plot.xAxis
        yAxis: plot.y2Axis
    }
}
```

## Extra axes

Use [`extraAxes`](../api/classQAccelPlot_1_1QAccelPlot.md#property-extraaxes-12)
for more than two axes in one orientation. They are laid out in declaration
order: horizontal axes stack below the primary X axis, vertical axes stack from
the plot's left edge inward.

```qml
import QtQuick
import QAccelPlot as QAccelPlot

QAccelPlot.Plot {
    id: plot

    xAxis: QAccelPlot.Axis {
        viewportMin: 0
        viewportMax: 10
        label: "Slow time (s)"
    }

    extraAxes: [
        QAccelPlot.Axis {
            id: fastAxis
            orientation: QAccelPlot.Axis.Horizontal
            side: QAccelPlot.Axis.Bottom
            viewportMin: 0
            viewportMax: 0.5
            label: "Fast time (s)"
        }
    ]

    QAccelPlot.LineCurve {
        xAxis: fastAxis
        yAxis: plot.yAxis
    }
}
```

Complete source: [`examples/axes/multiple_axes`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/axes/multiple_axes)

[x2-axis]: ../api/classQAccelPlot_1_1QAccelPlot.md#property-x2axis-12
[y2-axis]: ../api/classQAccelPlot_1_1QAccelPlot.md#property-y2axis-12
