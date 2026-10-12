---
description: "Zoom into a rectangular plot region with a mouse gesture or a custom tool."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Rectangle zoom

Rectangle zoom is disabled by default. Enable it on `Plot` or `PlotView`:

```qml
import QAccelPlot as QAccelPlot

QAccelPlot.Plot {
    rectangleZoom.enabled: true
}
```

- Shift + left drag selects a region; release to zoom all attached axes.
- Escape cancels. Double-click rescales to the data. Ordinary left dragging pans.
- The selection is clipped to the plot area and must be at least
  `rectangleZoom.minimumSize` logical pixels wide and high, 6 by default.
- `rectangleZoom.modifiers` must match exactly at press time; use
  `Qt.NoModifier` for unmodified dragging. A
  [`SelectionTool`](data-inspection.md#selection) uses the same gesture by
  default and takes it when both are enabled; change `modifiers` on one of them.
- `rectangleZoom.fillColor` and `borderColor` default to the dark palette's
  `rectangleZoomFill` and `rectangleZoomBorder`. Bind them to `Colors.light`
  for a light theme.

For a custom tool, call [`zoomToRect()`][zoom-to-rect] with item-local pixels:

```qml
const applied = plot.zoomToRect(Qt.rect(left, top, width, height))
```

It works with gestures disabled and supports logarithmic and reversed axes.
It returns `false` without changing ranges if the selection or target axes are invalid.

Complete source: [`examples/plot_types/point_cloud`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/plot_types/point_cloud)

[zoom-to-rect]: ../api/classQAccelPlot_1_1QAccelPlot.md#function-zoomtorect
