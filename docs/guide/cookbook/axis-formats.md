---
description: "Configure QAccelPlot axis tick labels — date/time formatting, categorical text labels, logarithmic scales, units with SI prefixes, and custom JavaScript formatters."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: MIT
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

Supply epoch values through `setData()`, which keeps double precision; the `F`
APIs store floats.

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

The data points still use X coordinates `0` through `6`. Zoomed in, the axis
still ticks only at whole numbers, so each category is labeled once.

## Logarithmic axes

Enable a base-10 logarithmic mapping:

```qml
xAxis: QAccelPlot.Axis {
    viewportMin: 20
    viewportMax: 20000
    logScale: true
}
```

Logarithmic ranges must stay positive. The default
`NumericTickLabelFormatter` labels each decade tick as a power of ten, such as
`10⁻³` and `10²`.

## Units and SI prefixes

[`QAccelPlot.UnitTickLabelFormatter`][unit-formatter] writes values in a unit
with an SI unit prefix that follows the visible range. The data stays in the
base unit:

```qml
xAxis: QAccelPlot.Axis {
    viewportMin: 0
    viewportMax: 0.004
    label.text: "Distance"

    ticker.tickLabelFormatter: QAccelPlot.UnitTickLabelFormatter {
        unit: "m"
    }
}
```

The tick labels read `0` to `4` and the axis label `Distance (mm)`. Zoomed in
to 0–0.000004, they switch to µm.

| Property | Effect |
| --- | --- |
| `unit` | Base unit symbol of the values, such as `"m"`. |
| `unitPosition` | `AxisLabel` (default): the axis label shows the prefixed unit. `TickLabels`: each tick label carries it, as in `2 mm`. |
| `prefixMode` | `LargestMagnitude` (default): the largest prefix that keeps the largest visible tick at 1 or more, shared by all tick labels. `Fixed`: `fixedPrefix` at every zoom level. `Custom`: the symbol returned by `customPrefix(min, max, ticks)`. `PerTick`: each tick label picks its own prefix and carries the unit, as in `500 mm`, `1.0 m`. |
| `allowedPrefixes` | Prefix symbols to choose from, with `""` for the bare unit. Default: every power-of-1000 prefix from `q` to `Q`, and `""`. `[]` turns prefixing off. Assign `undefined` to restore the default. |

On a logarithmic axis, a shared prefix keeps the decade ticks as powers of ten
in the prefixed unit; `PerTick` writes each decade as a plain number, such as
`1 kHz`, `10 kHz`, `100 kHz`, `1 MHz`.

The axis label combines `label.text` and the read-only `label.unitText` with
`label.unitFormat`, `"%1 (%2)"` by default: `"%1 [%2]"` paints
`Distance [mm]`. A `label.delegate` binds `label.unitText` to show the unit.
Color bars work the same way. `Axis.formatValue()`, used by tooltips, the
crosshair, and the inspector, appends the prefixed unit: `12.3 mm`.

Complete source: [`examples/axes/unit_tick_labels`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/axes/unit_tick_labels)

## Application-specific labels

For custom text, assign a JavaScript function to a formatter's
[`tickLabel`][tick-label] property. It runs for every tick label, so keep it
cheap:

```qml
yAxis: QAccelPlot.Axis {
    viewportMin: 18
    viewportMax: 26
    label.text: "Temperature"

    ticker.tickLabelFormatter: QAccelPlot.NumericTickLabelFormatter {
        tickLabel: function (temperature) {
            return temperature.toFixed(1) + " °C"
        }
    }
}
```

Complete source: [`examples/axes/tick_formats`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/axes/tick_formats)

[tick-label]: ../api/classQAccelPlot_1_1TickLabelFormatter.md#property-ticklabel-12
[unit-formatter]: ../api/classQAccelPlot_1_1UnitTickLabelFormatter.md
