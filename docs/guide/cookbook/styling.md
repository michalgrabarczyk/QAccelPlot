---
description: "Style QAccelPlot curves with color palettes, dash patterns, point markers, gradient fills and strokes, and animated data transitions."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Styling and transitions

## Shared palette

The read-only `Colors` singleton exposes light and dark palettes. Import the
module with an alias to keep `Colors` out of the unqualified namespace:

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

Default plot, axis, grid, tick, hover, series, and legend colors come from
`Colors.dark`. Assign `Colors.light` tokens or any colors to restyle a plot.

## Lines and markers

```qml
QAccelPlot.LineCurve {
    color: "dodgerblue"
    lineWidth: 2
    lineStyle: QAccelPlot.DashLine {
        pattern: [14, 5, 3, 5]
    }
    marker.shape: QAccelPlot.LineCurve.Diamond
    marker.size: 5
    antialiasingEnabled: true
    antialiasingFeather: 1
}
```

Line styles are `QAccelPlot.SolidLine`, `QAccelPlot.DashLine`, and
`QAccelPlot.NoLine`. `NoLine` with a marker shape gives a marker-only series.
Marker settings are grouped under `marker`: `shape`, `size`, `filled`, and
`strokeWidth`.
A dash pattern lists alternating on/off lengths in pixels.

## Marker shapes

`marker.size` is the marker radius in pixels. Every shape fits within a square
of that half-width.

| Group | `MarkerShape` values |
| --- | --- |
| Filled shapes | `Circle`, `Square`, `Diamond`, `TriangleUp`, `TriangleDown`, `TriangleLeft`, `TriangleRight`, `Pentagon`, `Hexagon`, `Star` |
| Line shapes | `Cross` (+), `XCross` (×), `Asterisk`, `HLine`, `VLine` |
| Single pixel | `Pixel` |

Set `marker.filled: false` to draw filled shapes as outlines of
`marker.strokeWidth` pixels. The outline lies inside the shape's edge, so hollow
and filled markers have the same size. Line shapes and `Pixel` ignore both
properties.

```qml
QAccelPlot.LineCurve {
    lineStyle: QAccelPlot.NoLine {}
    marker.shape: QAccelPlot.LineCurve.Circle
    marker.size: 4
    marker.filled: false
    marker.strokeWidth: 1
}
```

`Pixel` draws each sample as one logical pixel and ignores `marker.size` and
anti-aliasing. Use it with a semi-transparent color for scatter plots with
hundreds of thousands of points, where overlapping points build up density.
Hover still detects a `Pixel` marker within 3 px.

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

Fixed gradient bounds keep colors stable while the viewport changes. Axis- or
data-derived bounds make colors follow the current range.

## Animated data changes

Assign a `QAccelPlot.MorphTransition` to [`transition`][transition] to
interpolate from old to new samples, or a `QAccelPlot.DrawTransition` to reveal
the new curve:

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

Transitions add work on every animated frame. Disable them for continuous
high-rate updates.

Complete source: [`examples/styling/styling_and_transitions`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/styling/styling_and_transitions)

[effects]: ../api/classQAccelPlot_1_1LineCurve.md#property-effects-12
[transition]: ../api/classQAccelPlot_1_1LineCurve.md#property-transition-12
