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

## Use logarithmic axes and spacing

`xSpacing` and `ySpacing` set how cells are spaced between the grid edges,
independently of the axis scale:

- `HeatmapSeries.Linear` (default): cells of equal size in data coordinates.
- `HeatmapSeries.Log`: cells of equal size in log10, each spanning the same
  ratio, such as constant-Q or octave bins.

| Spacing | Linear axis | Log axis |
|---|---|---|
| `Linear` | Equal widths | Narrower toward larger values |
| `Log` | Wider toward larger values | Equal widths |

```qml
QAccelPlot.Plot {
    id: plot
    yAxis: QAccelPlot.Axis { viewportMin: 20; viewportMax: 20000; logScale: true }

    QAccelPlot.HeatmapSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        xMin: 0; xMax: 10
        // Rows of equal frequency ratio from 20 Hz to 20 kHz.
        yMin: 20; yMax: 20000
        ySpacing: QAccelPlot.HeatmapSeries.Log
    }
}
```

- On a log axis, the parts of `Linear` cells at or below zero are not drawn,
  and the data range starts at the first cell edge above zero. A grid from
  0 Hz, such as FFT bins, keeps its first bin.
- `Log` spacing needs both edges of that dimension positive. Set them: an
  unset edge resolves to index space, which starts at 0. Otherwise the series
  draws nothing, reports no data range in that dimension, and logs a warning.

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

A heatmap has no legend entry: the color bar is its key.

## Read the cell under the pointer

A [`PlotInspector`](data-inspection.md#inspector) reports the cell under the
cursor in every mode. Its row has the cell center (geometric under `Log`
spacing) as `dataPointX` and `dataPointY`, the value as `dataPointValue` and
`valueText`, and `row * columns + column` as `dataPointIndex`.
`InspectionTooltip` shows the center and the value:

```qml
QAccelPlot.PlotInspector {
    plot: plot

    QAccelPlot.InspectionTooltip {}
}
```

A cell with a non-finite value is still a match. In code,
`inspection.dataPointAtPosition()` returns the same data point for a
series-local position: pixels from the top left of the plot area.
`inspection.recordAtPosition()` and `inspection.recordAt(index)` return the
cell's record, whose `fields` are `x` and `y` (the cell center), `value`,
`row`, and `column`. While a grid edge is infinite, or not positive under
`Log` spacing, `recordAt()` returns `Inspection.Unavailable` and no position
matches a cell.

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
- A grid with an infinite edge, such as `xMin: -Infinity`, draws nothing.
- A grid can have as many cells as the GPU data texture holds: about 67
  million on a GPU with an 8192-pixel texture limit. A larger grid is not
  drawn, and a warning is logged.
- A grid of more than 16,777,216 (2^24) columns or rows is not drawn, and a
  warning is logged: the GPU locates cells in single precision.

Complete source: [`examples/plot_types/heatmap`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/plot_types/heatmap)

[heatmap-series]: ../api/classQAccelPlot_1_1HeatmapSeries.md
[colormap]: ../api/classQAccelPlot_1_1Colormap.md
[color-bar]: ../api/classQAccelPlot_1_1ColorBar.md
