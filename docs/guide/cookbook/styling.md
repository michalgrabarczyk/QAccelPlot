---
description: "Style QAccelPlot curves with color palettes, dash patterns, point markers, step lines, gradient fills and strokes, glow and fading trails, and animated data transitions."
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
    marker.shape: QAccelPlot.MarkerShape.Diamond
    marker.size: 5
}
```

Line styles are `QAccelPlot.SolidLine`, `QAccelPlot.DashLine`, and
`QAccelPlot.NoLine`. `NoLine` with a marker shape gives a marker-only series.
Marker settings are grouped under `marker`: `shape`, `size`, `filled`, and
`strokeWidth`.
A dash pattern lists alternating on/off lengths in pixels.

## Antialiasing

Series smooth their edges with the `antialiasing` property every `Item` has.
`HeatmapSeries` ignores it.

| Series | Default | Smoothed edges |
| --- | --- | --- |
| `LineCurve` | `true` | lines and markers |
| `PointCloud` | `true` | markers |
| `BandSeries` | `true` | edge lines |
| `ErrorBarSeries` | `true` | lines and caps |
| `RectangleSeries`, `BarSeries` | `false` | rectangles and their border |

Rectangles and bars that touch, such as histogram bins, show faint seams with
`antialiasing: true`. `LineCurve` and `PointCloud` also take
`antialiasingFeather`, the width of the edge ramp in pixels (0 to 10,
default 1).

```qml
QAccelPlot.LineCurve {
    antialiasing: false // hard, pixel-aligned edges
}
```

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
    marker.shape: QAccelPlot.MarkerShape.Circle
    marker.size: 4
    marker.filled: false
    marker.strokeWidth: 1
}
```

`Pixel` draws each data point as one logical pixel and ignores `marker.size` and
antialiasing. Use it with a semi-transparent color for scatter plots with
hundreds of thousands of points, where overlapping points build up density.
Hover still detects a `Pixel` marker within 3 px.

## Gradient stroke and fill

Effects are attached through the series' [`effects`][effects] property:

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
  the colormap's `min`, `max`, and `logScale` are ignored.
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

## Glow and fading trails

`Glow` draws a soft halo around a series, in the same pass and without a blur.
`FadeTrail` fades the oldest items, by index, like the trace of an
oscilloscope:

```qml
QAccelPlot.LineCurve {
    effects: [
        QAccelPlot.Glow { radius: 8; intensity: 0.6 },
        QAccelPlot.FadeTrail { length: 0.7 }
    ]
}
```

| Effect | Series | Properties |
|---|---|---|
| `Glow` | `LineCurve`, `PointCloud`, `BarSeries`, `ErrorBarSeries` | `color` (default: the series color), `radius` in pixels, `intensity` |
| `FadeTrail` | `LineCurve`, `PointCloud` | `length`: share of the items that fades, from the oldest; `minOpacity` of the oldest; `easing`: how the opacity rises, `Easing.InQuad` by default |

- An effect a series does not support logs a warning and is ignored.
- Of several effects of one type, the first enabled one applies.
- A fading item and its halo fade together.
- Halos of parts of a line that come close to each other add up, as halos of
  separate lines do.
- `FadeTrail` with `Easing.InQuad` keeps most of the trail dim, like a scope
  trace; `Easing.Linear` raises the opacity evenly.
- Effect properties animate like any QML property, for example with a
  `Behavior on intensity`.

## Animated data changes

A data change matches the old and new items by index, and three transitions
animate it:

| Property | Animates | Takes |
|---|---|---|
| [`enterTransition`][enter-transition] | Added items, and all items of the first data | `DrawTransition`, `GrowTransition` |
| [`updateTransition`][update-transition] | Kept items | `MorphTransition`, `DrawTransition`, `GrowTransition` |
| [`exitTransition`][exit-transition] | Removed items, and all items on `clearData()` | `DrawTransition`, `GrowTransition` |

Items without a transition in their slot change at once. The update transition
runs only when a kept item changes, so adding or removing items leaves the
others alone.

```qml
QAccelPlot.BarSeries {
    enterTransition: QAccelPlot.GrowTransition { stagger: 0.5 }
    updateTransition: QAccelPlot.MorphTransition { duration: 300 }
    exitTransition: QAccelPlot.GrowTransition { duration: 200 }
}
```

- `MorphTransition` moves each kept item from its old values to its new ones.
- `DrawTransition` reveals items one after another. A line draws its last
  segment partway.
- `GrowTransition` grows values from `baseline`: the bar baseline for bars, 0
  for other series.
- A `DrawTransition` or `GrowTransition` in `updateTransition` redraws all new
  items as one sequence when a kept item changes; the enter and exit
  transitions are skipped.
- An exit plays the movement backwards, the last item first.

Every transition takes `duration`, `easing`, and `stagger`:

- `stagger` is the share of `duration` over which the item start times are
  spread. 0 animates all items together; 1 animates them one after another.
  `DrawTransition` defaults to 1, the others to 0.
- `easing` applies to the progress of each item. With a `stagger` of 1, it
  shapes how the reveal moves along the items.

### Fade items

Declare Qt Quick animations of `"opacity"` inside a transition, as in a
`ListView` transition. Each item plays them from the moment it starts to move:

```qml
enterTransition: QAccelPlot.DrawTransition {
    NumberAnimation { property: "opacity"; from: 0; duration: 250 }
}
exitTransition: QAccelPlot.GrowTransition {
    NumberAnimation { property: "opacity"; to: 0; easing.type: Easing.InQuad }
}
updateTransition: QAccelPlot.MorphTransition {
    duration: 600
    SequentialAnimation {
        NumberAnimation { property: "opacity"; to: 0.4; duration: 200 }
        NumberAnimation { property: "opacity"; to: 1; duration: 400 }
    }
}
```

- The transition reads the animations; it never runs them.
- Supported: `NumberAnimation` and other numeric `PropertyAnimation` types,
  `PropertyAction`, `PauseAnimation`, `SequentialAnimation`, and
  `ParallelAnimation`, with finite `loops`. Several animations in a transition
  play together.
- A missing `from` continues from the opacity reached so far; a missing `to`
  is 1. Before its first animation starts, an item shows that animation's
  start opacity.
- Exits play opacity animations as written, so a fade out sets `to: 0`.
- When the animations last longer than an item's own movement, the item start
  times move closer together, so the transition still ends after `duration`.
  With a `stagger` of 1, the reveal crosses the items while the last ones fade.
- Other animations, and animations of other properties, log a warning and only
  add their duration. `NumberAnimation on opacity` does not load: each item
  has its own opacity.
- Every item is opaque again when the transition ends.

Several series can share one transition, and one series can use a transition in
several slots. While a transition runs, the series' `animating` is `true`.
Replacing, clearing, or cancelling a transition, or calling `appendData()`,
shows the new data at once. Data that arrives during an animation continues
from the items drawn now.

Each animated frame interpolates the data on the CPU: about 4 ms per million
points, 6 ms with a stagger. Disable transitions for continuous high-rate
updates.

`BarSeries`, `PointCloud`, and `ErrorBarSeries` take the same transitions; see
[bar charts](bar-charts.md#animate-data-changes),
[point clouds](point-clouds.md#animate-data-changes), and
[error bars](error-bars.md#animate-data-changes).

Complete sources:

- [`examples/styling/theming`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/styling/theming)
- [`examples/styling/line_styles`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/styling/line_styles)
- [`examples/styling/markers`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/styling/markers)
- [`examples/styling/gradients`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/styling/gradients)
- [`examples/styling/effects`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/styling/effects)
- [`examples/styling/transitions`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/styling/transitions)
- [`examples/plot_types/step_lines`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/plot_types/step_lines)

[effects]: ../api/classQAccelPlot_1_1PlotSeries.md#property-effects-12
[step-mode]: ../api/classQAccelPlot_1_1LineCurve.md#property-stepmode-12
[enter-transition]: ../api/classQAccelPlot_1_1LineCurve.md#property-entertransition-12
[update-transition]: ../api/classQAccelPlot_1_1LineCurve.md#property-updatetransition-12
[exit-transition]: ../api/classQAccelPlot_1_1LineCurve.md#property-exittransition-12
