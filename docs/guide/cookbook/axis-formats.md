---
description: "Configure QAccelPlot axis tick labels — date/time formatting, categorical text labels, logarithmic scales, and custom JavaScript formatters."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Axis formats

Axis values remain numeric. A formatter changes only the displayed tick text.

## Date and time

Use milliseconds since the Unix epoch as X values and attach a
`QAccelPlot.DateTimeTickLabelFormatter`:

```qml
xAxis: QAccelPlot.Axis {
    viewportMin: traceStart
    viewportMax: traceEnd
    side: QAccelPlot.Axis.Bottom

    ticker.tickLabelFormatter: QAccelPlot.DateTimeTickLabelFormatter {
        dateTimeFormat: "hh:mm:ss.zzz"
    }
}
```

## Categories

Map integer coordinates to labels with `QAccelPlot.TextTickLabelFormatter`:

```qml
xAxis: QAccelPlot.Axis {
    viewportMin: 0
    viewportMax: 6
    side: QAccelPlot.Axis.Bottom
    ticker.tickCount: 6
    ticker.subtickCount: 0
    ticker.tickLabelFormatter: QAccelPlot.TextTickLabelFormatter {
        labels: ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
    }
}
```

The data points still use X coordinates `0` through `6`.

## Logarithmic axes

Enable a base-10 logarithmic mapping and use logarithmic labels:

```qml
QAccelPlot.LogTickLabelFormatter { id: logLabels }

xAxis: QAccelPlot.Axis {
    viewportMin: 20
    viewportMax: 20000
    dataMin: 20
    dataMax: 20000
    logScale: true
    side: QAccelPlot.Axis.Bottom
    ticker.tickLabelFormatter: logLabels
}
```

Logarithmic ranges must remain positive. Do not pass zero or negative bounds.
Power-of-ten ticks use superscript exponents, such as `10²`, `10³`, and `10⁻³`.

## Application-specific labels

`QAccelPlot.NumericTickLabelFormatter` provides the normal numeric presentation. For
specialized formatting, assign a JavaScript callback to a
`QAccelPlot.TickLabelFormatter`'s [`tickLabel`][tick-label] property; keep the function inexpensive
because it runs for every generated tick label.

For example, a temperature-monitoring plot can append units while retaining numeric
data values:

```qml
yAxis: QAccelPlot.Axis {
    viewportMin: 18
    viewportMax: 26
    side: QAccelPlot.Axis.Left
    label: "Temperature"

    ticker.tickLabelFormatter: QAccelPlot.NumericTickLabelFormatter {
        tickLabel: function (temperature) {
            return temperature.toFixed(1) + " °C"
        }
    }
}
```

Complete source: [`examples/axis_formats`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/axis_formats)

[tick-label]: ../api/classQAccelPlot_1_1TickLabelFormatter.md#property-ticklabel-12
