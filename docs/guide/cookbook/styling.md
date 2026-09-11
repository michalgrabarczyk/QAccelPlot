---
description: "Style QAccelPlot curves with color palettes, dash patterns, point markers, gradient fills and strokes, and animated data transitions."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Styling and transitions

## Shared palette

QAccelPlot exposes its light and dark palettes as a read-only
singleton. Importing the module with an alias keeps the generic `Colors` name
out of the application's unqualified type namespace:

```qml
import QtQuick
import QAccelPlot as QAccelPlot

QAccelPlot.Plot {
    plotAreaColor: QAccelPlot.Colors.dark.plotArea
    axesAreaColor: QAccelPlot.Colors.dark.axesArea

    QAccelPlot.LineCurve {
        color: QAccelPlot.Colors.dark.seriesPrimary
    }
}
```

The palette is opt-in; importing QAccelPlot does not automatically change plot
or application colors.

## Lines and markers

```qml
QAccelPlot.LineCurve {
    color: "dodgerblue"
    lineWidth: 2
    lineStyle: QAccelPlot.DashLine {
        pattern: [14, 5, 3, 5]
    }
    markerShape: QAccelPlot.LineCurve.Diamond
    markerSize: 5
    antialiasingEnabled: true
    antialiasingFeather: 1
}
```

Use `QAccelPlot.SolidLine`, `QAccelPlot.DashLine`, or `QAccelPlot.NoLine`.
`QAccelPlot.NoLine` combined with a marker shape creates a marker-only series.
A dash pattern alternates on/off lengths in pixels.

## Gradient stroke and fill

Effects are attached through the curve's [`effects`][effects] property:

```qml
QAccelPlot.LineCurve {
    effects: [
        QAccelPlot.GradientFill {
            direction: QAccelPlot.GradientDirection.Vertical
            baseline: QAccelPlot.GradientFillBaseline.Value
            baselineValue: 0
            gradientValueMinSource: QAccelPlot.GradientValueSource.Fixed
            gradientValueMin: -4
            gradientValueMaxSource: QAccelPlot.GradientValueSource.Fixed
            gradientValueMax: 8
            opacity: 0.65
            gradient: Gradient {
                GradientStop { position: 0; color: "royalblue" }
                GradientStop { position: 1; color: "goldenrod" }
            }
        }
    ]
}
```

Explicit gradient bounds make the visual mapping stable while the viewport
changes. Axis- or data-derived bounds are useful when the colors should track
the current domain.

## Animated data changes

Assign a `QAccelPlot.MorphTransition` to [`transition`][transition] when old and new samples should
interpolate, or a `QAccelPlot.DrawTransition` to reveal the new curve:

```qml
QAccelPlot.MorphTransition {
    id: morph
    duration: 500
    easing.type: Easing.InOutCubic
}

QAccelPlot.LineCurve {
    transition: morph
}
```

Transitions create work on intermediate frames. Disable them for sustained
high-frequency updates unless animation is part of the intended result.

Complete source: [`examples/styling_and_transitions`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/styling_and_transitions)

[effects]: ../api/classQAccelPlot_1_1LineCurve.md#property-effects-12
[transition]: ../api/classQAccelPlot_1_1LineCurve.md#property-transition-12
