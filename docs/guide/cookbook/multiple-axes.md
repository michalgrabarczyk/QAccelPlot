---
description: "Add secondary and extra axes to a QAccelPlot chart for multiple units or sampling domains sharing a single plot area."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Multiple axes

Each series independently chooses its X and Y axes. This permits several units
or sampling domains to share a plot area.

## Axis layout size

Each axis reserves 50 pixels of layout space by default. Set `layoutSize` to
choose the height of a horizontal axis or the width of a vertical axis:

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

Axis layout is intentionally explicit: label text, fonts, and formatters do not
change `layoutSize` automatically. Choose a value large enough for the configured
tick labels and axis title.

## Secondary axes

Use [`x2Axis`][x2-axis] or [`y2Axis`][y2-axis] for an axis on the opposite side.
The plot assigns sides automatically: `xAxis` is bottom, `x2Axis` is top,
`yAxis` is left, and `y2Axis` is right.

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

Use [`extraAxes`](../../api/classQAccelPlot_1_1QAccelPlot.html#a2aa8566edaee068e3517f5085fe9276b)
when more than two axes are needed in one orientation. They are laid out in
declaration order: extra horizontal axes stack below the primary X axis, while
extra vertical axes stack from the plot's left edge inward.

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

Coloring each axis to match its series reduces ambiguity. Disable or customize
the grid if ticks from different domains would imply a false correspondence.

Complete source: [`examples/custom_axis`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/custom_axis)

[x2-axis]: ../../api/classQAccelPlot_1_1QAccelPlot.html#a8fd3ea82835224ee95f3c4ec4bd78ff8
[y2-axis]: ../../api/classQAccelPlot_1_1QAccelPlot.html#ab940a26d0902b92808a8ee0885cb7455
