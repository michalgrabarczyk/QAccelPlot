---
description: "Zoom into a rectangular plot region with a mouse gesture or a custom tool."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Rectangle zoom

Enable rectangle zoom on `Plot` or `PlotView`:

```qml
import QAccelPlot as QAccelPlot

QAccelPlot.Plot {
    readonly property color zoomBorderColor: QAccelPlot.Colors.dark.rectangleZoomBorder

    rectangleZoom.enabled: true
    rectangleZoom.modifiers: Qt.ShiftModifier
    rectangleZoom.minimumSize: 6
    rectangleZoom.fillColor: QAccelPlot.Colors.dark.rectangleZoomFill
    rectangleZoom.borderColor: zoomBorderColor
}
```

- Shift + left drag selects a region; release to zoom all attached axes.
- Escape cancels. Double-click rescales to the data. Ordinary left dragging pans.
- The selection is clipped to the plot area and must be at least 6 logical pixels
  wide and high; adjust `rectangleZoom.minimumSize` to change this.

Rectangle zoom is disabled by default. `rectangleZoom.modifiers` must match
exactly at press time; use `Qt.NoModifier` for unmodified dragging.

Bind `fillColor` to the palette's `rectangleZoomFill` and `borderColor` to
`rectangleZoomBorder`. Both are gray in the dark theme and light gray in the light theme;
the fill color includes 15% opacity.

For a custom tool, call [`zoomToRect()`][zoom-to-rect] with item-local pixels:

```qml
const applied = plot.zoomToRect(Qt.rect(left, top, width, height))
```

It works with gestures disabled and supports logarithmic and reversed axes.
It returns `false` without changing ranges if the selection or target axes are invalid.

Complete source:
[`examples/plot_types/point_cloud`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/plot_types/point_cloud).

[zoom-to-rect]: ../api/classQAccelPlot_1_1QAccelPlot.md#function-zoomtorect
