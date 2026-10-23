---
description: "Style QAccelPlot curves with color palettes, dash patterns, point markers, step lines, gradient fills and strokes, and animated data transitions."
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

## Step lines

[`stepMode`][step-mode] draws the line as horizontal steps joined by vertical
risers:

```qml
QAccelPlot.LineCurve {
    stepMode: QAccelPlot.StepMode.After
}
```

| `StepMode` | Each value holds |
| --- | --- |
| `None` (default) | Nowhere: straight segments join the data points. |
| `Before` | From the previous data point's X up to its own X. |
| `After` | From its own X up to the next data point's X. |
| `Center` | Between the midpoints to its neighbors, halfway on screen (the geometric mean on a log axis). |

- The line starts at the first data point and ends at the last one. To hold the
  last value until now, append a data point at the current X.
- The gradient fill, dash pattern, and hover follow the steps. Markers stay on
  the data points.
- With [`PlotInspector.interpolate`](data-inspection.md), a crosshair between
  two data points reads the value the step holds, and `index` is the data point
  holding it.
- The line follows the data points in order; X is not sorted.

To outline a histogram, repeat the last value at the last edge and draw it with
`After`:

```qml
QAccelPlot.LineCurve {
    stepMode: QAccelPlot.StepMode.After
    Component.onCompleted: {
        const histogram = QAccelPlot.Histogram.fromSamples(samples, 40);
        const values = histogram.values;
        setData(histogram.edges.map((edge, i) => Qt.point(edge, values[Math.min(i, values.length - 1)])));
    }
}
```

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

`Pixel` draws each data point as one logical pixel and ignores `marker.size` and
anti-aliasing. Use it with a semi-transparent color for scatter plots with
hundreds of thousands of points, where overlapping points build up density.
Hover still detects a `Pixel` marker within 3 px.

## Gradient stroke and fill

Effects are attached through the curve's [`effects`][effects] property:

```qml
QAccelPlot.LineCurve {
    effects: [
        QAccelPlot.GradientFill {
            direction: Qt.Vertical
            baseline: QAccelPlot.GradientFillBaseline.Value
            baselineValue: 0
            valueMinSource: QAccelPlot.GradientValueSource.Fixed
            valueMin: -4
            valueMaxSource: QAccelPlot.GradientValueSource.Fixed
            valueMax: 8
            opacity: 0.65
            colormap: QAccelPlot.Colormap { preset: QAccelPlot.Colormap.Viridis }
        }
    ]
}
```

Both effects take their color stops from one of two properties:

- `colormap`: a [`Colormap`](point-clouds.md#color-points-by-value) preset or
  custom ramp. Overrides `gradient` when both are set. Only the ramp is used;
  the colormap's `min`, `max`, and `norm` are ignored.
- `gradient`: a Qt Quick `Gradient`.

```qml
QAccelPlot.GradientStroke {
    gradient: Gradient {
        GradientStop { position: 0; color: "royalblue" }
        GradientStop { position: 1; color: "goldenrod" }
    }
}
```

Fixed gradient bounds keep colors stable while the viewport changes. Axis- or
data-derived bounds make colors follow the current range.

## Animated data changes

Assign a `QAccelPlot.MorphTransition` to [`transition`][transition] to
interpolate from old to new data points, or a `QAccelPlot.DrawTransition` to reveal
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

Several curves can share one transition; each animates to its own data.
Replacing, clearing, or cancelling `transition`, or calling `appendData()`,
during an animation shows the new data at once.

`BarSeries` and `PointCloud` take the same transitions; see
[bar charts](bar-charts.md#animate-data-changes) and
[point clouds](point-clouds.md#animate-data-changes).

Transitions add work on every animated frame. Disable them for continuous
high-rate updates.

Complete sources:

- [`examples/styling/theming`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/styling/theming)
- [`examples/styling/line_styles`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/styling/line_styles)
- [`examples/styling/markers`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/styling/markers)
- [`examples/styling/gradients`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/styling/gradients)
- [`examples/styling/transitions`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/styling/transitions)
- [`examples/plot_types/step_lines`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/plot_types/step_lines)

[effects]: ../api/classQAccelPlot_1_1LineCurve.md#property-effects-12
[step-mode]: ../api/classQAccelPlot_1_1LineCurve.md#property-stepmode-12
[transition]: ../api/classQAccelPlot_1_1LineCurve.md#property-transition-12
