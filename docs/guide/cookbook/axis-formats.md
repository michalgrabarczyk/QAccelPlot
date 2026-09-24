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

    ticker.tickLabelFormatter: QAccelPlot.DateTimeTickLabelFormatter {
        dateTimeFormat: "hh:mm:ss.zzz"
    }
}
```

Supply epoch values through `setData()`, which keeps double precision. The `F`
APIs are faster but store floats, which are less precise.

## Categories

Map integer coordinates to labels with `QAccelPlot.TextTickLabelFormatter`:

```qml
xAxis: QAccelPlot.Axis {
    viewportMin: 0
    viewportMax: 6
    ticker.tickCount: 6
    ticker.subtickCount: 0
    ticker.tickLabelFormatter: QAccelPlot.TextTickLabelFormatter {
        labels: ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
    }
}
```

The data points still use X coordinates `0` through `6`.

## Logarithmic axes

Enable a base-10 logarithmic mapping:

```qml
xAxis: QAccelPlot.Axis {
    viewportMin: 20
    viewportMax: 20000
    dataMin: 20
    dataMax: 20000
    logScale: true
}
```

Logarithmic ranges must remain positive. Do not pass zero or negative bounds.
The default `NumericTickLabelFormatter` labels each decade tick as a power of
ten with a superscript exponent, such as `10⁻³`, `10²`, and `10³`.

## Application-specific labels

`QAccelPlot.NumericTickLabelFormatter` is the default numeric format. For custom
text, assign a JavaScript function to the formatter's
[`tickLabel`][tick-label] property. It runs for every tick label, so keep it
cheap:

```qml
yAxis: QAccelPlot.Axis {
    viewportMin: 18
    viewportMax: 26
    label: "Temperature"

    ticker.tickLabelFormatter: QAccelPlot.NumericTickLabelFormatter {
        tickLabel: function (temperature) {
            return temperature.toFixed(1) + " °C"
        }
    }
}
```

Complete source: [`examples/axes/tick_formats`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/axes/tick_formats)

[tick-label]: ../api/classQAccelPlot_1_1TickLabelFormatter.md#property-ticklabel-12
