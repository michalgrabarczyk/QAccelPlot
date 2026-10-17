---
description: "Draw a grid of values as colored cells in QAccelPlot with HeatmapSeries: grid shape and placement, colormaps and color bars, reading cells under the pointer, and streaming grids from worker threads."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Heatmaps

Use [`QAccelPlot.HeatmapSeries`][heatmap-series] for values sampled on a
regular grid, such as a field, an image, or a matrix. Each cell is colored by
its value through a [`Colormap`][colormap].

## Draw a heatmap

```qml
QAccelPlot.Plot {
    id: plot
    xAxis: QAccelPlot.Axis { viewportMin: -10; viewportMax: 10 }
    yAxis: QAccelPlot.Axis { viewportMin: -5; viewportMax: 5 }

    QAccelPlot.HeatmapSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        xMin: -10; xMax: 10
        yMin: -5; yMax: 5
        // Three columns by two rows.
        Component.onCompleted: setValues([0, 1, 2,
                                          3, 4, 5], 3)
    }
}
```

- Values are stored row by row, `columns` per row. Row 0 is at `yMin` and
  column 0 at `xMin`.
- `xMin`, `xMax`, `yMin`, and `yMax` are the outer edges of the grid, so a
  cell is `(xMax - xMin) / columns` wide. An unset edge places the grid in
  index space: column `c` spans `[c, c + 1]`. A reversed extent flips the grid.
- The grid edges are the series' data range, so `rescaleToData()` fits the
  whole grid.
- `setValues()` sets `columns`. Do not bind `columns` while using it: when
  the binding is re-evaluated, it reshapes the grid.
- `valueAt(row, column)` reads a cell.

## Choose the colors

The series starts with its own Viridis `Colormap`, with the value range
resolved from the data. Change it in place, or assign a shared one:

```qml
QAccelPlot.HeatmapSeries {
    colormap: QAccelPlot.Colormap {
        preset: QAccelPlot.Colormap.Inferno
        min: -1
        max: 1
    }
}
```

The colormap works as for [point clouds](point-clouds.md#color-points-by-value):
presets, `reversed`, custom `stops`, fixed or data-resolved `min` and `max`,
and `norm: Colormap.Log`. `dataValueMin` and `dataValueMax` expose the resolved
range. Nothing is drawn while `colormap` is null.

These cells are transparent:

- cells with a non-finite value, such as `NaN` for missing data;
- under a `Log` colormap, cells with a value at or below zero.

Non-finite values do not count toward the value range.

## Show a color bar

A [`ColorBar`][color-bar] keys a heatmap as it keys a point cloud; see
[Show a color bar](point-clouds.md#show-a-color-bar):

```qml
QAccelPlot.ColorBar {
    series: heatmap
    label: "Amplitude"
    interactive: true
}
```

## Read the cell under the pointer

`inspection.recordAtPosition()` returns the record of the cell at a
series-local position: pixels from the top left of the plot area. Its
`fields` are `x` and `y` (the cell center), `value`, `row`, and `column`;
`index` is `row * columns + column`.

```qml
// Covers the plot area, so its positions are series-local.
Item {
    parent: plot.overlay
    x: plot.plotRect.x
    y: plot.plotRect.y
    width: plot.plotRect.width
    height: plot.plotRect.height

    HoverHandler { id: hover }

    Text {
        readonly property var record: heatmap.inspection.recordAtPosition(hover.point.position)
        visible: hover.hovered && record.valid
        text: visible ? record.fields.value.toFixed(3) : ""
    }
}
```

A cell with a non-finite value is still a match. `inspection.recordAt(index)`
looks a cell up by index. While a grid edge is infinite, `recordAt()` returns
`Inspection.Unavailable` and `recordAtPosition()` matches no cell.
`PlotInspector`, and so `InspectionTooltip` and `Crosshair`, do not inspect
heatmaps.

## Set data from C++

`setValues()` replaces the values and the shape together:

```cpp
// columns * rows floats, row by row.
heatmap->setValues(std::move(values), columns, rows);
```

From a worker thread, `postValues()` takes the same arguments and queues the
handoff; see [Background data production](background-data.md).

The `PlotSeries` setters, such as `setDataF(std::move(values), count)` and
`postData()`, take one value per cell and keep `columns`, so `count` must be
a multiple of it. Setting `columns` keeps the values and recomputes `rows`.

## Limits

- Each pixel takes the color of the cell under its center. Cells are not
  interpolated.
- Values are single precision.
- Logarithmic axes are not supported. On one, the series draws nothing,
  reports no data range in that dimension, and logs a warning.
- A grid with an infinite edge, such as `xMin: -Infinity`, draws nothing.
- A grid can have as many cells as the GPU data texture holds: about 67
  million on a GPU with an 8192-pixel texture limit. A larger grid is not
  drawn, and a warning is logged.

Complete source: [`examples/plot_types/heatmap`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/plot_types/heatmap)

[heatmap-series]: ../api/classQAccelPlot_1_1HeatmapSeries.md
[colormap]: ../api/classQAccelPlot_1_1Colormap.md
[color-bar]: ../api/classQAccelPlot_1_1ColorBar.md
