---
description: "Crosshairs, tooltips, nearest-data-point queries, region statistics, and selection for large and live datasets."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Data inspection

`PlotInspector` shows a crosshair, a marker on every series, and a tooltip at
the cursor. `SelectionTool` selects a region and reports its statistics.

```qml
import QAccelPlot as QAccelPlot

QAccelPlot.Plot {
    id: plotView
    // Bind each series to its axes.
    QAccelPlot.PlotInspector {
        plot: plotView
        QAccelPlot.Crosshair { axisLabels: true }
        QAccelPlot.InspectionMarkers {}
        QAccelPlot.InspectionTooltip {}
    }
    QAccelPlot.SelectionTool { plot: plotView }
}
```

- Components declared inside a `PlotInspector` use it and are drawn in
  declaration order, the last one on top. A component declared elsewhere needs
  `inspector:` set and is not restacked.
- They are children of `plotView.overlay`, an item above the series and the
  legend. Parent your own overlays to it instead of choosing a `z` relative to
  the series.
- Shift + left drag selects a region; Escape clears it. Ordinary left dragging
  still pans.

[Rectangle zoom](rectangle-zoom.md) defaults to Ctrl + left drag. If both are
set to the same gesture, `SelectionTool` takes it.

## Inspector

[`PlotInspector`][plot-inspector] queries every visible `LineCurve` and
`PointCloud` of the plot at the cursor. Other series types have no row.

| Property | Effect |
| --- | --- |
| `mode` | `NearestX` reports each series at the cursor's X. `NearestY` does the same at the cursor's Y, for series whose independent variable is Y. `NearestXY` picks the data point closest on screen. |
| `radius` | Pick distance in logical pixels. In `NearestX` and `NearestY` it only applies beyond a series' extreme data points; between two data points a series always matches. |
| `interpolate` | In `NearestX` and `NearestY`, report the point on the line between two consecutive data points. |
| `snapToDataPoint` | Move the crosshair to the closest matching data point. |
| `summaries`, `summaryRadius` | Add the count and Y statistics of the data points within `summaryRadius` pixels of the cursor. Use it for dense data, where one pixel column holds many data points. |
| `includedSeries`, `excludedSeries` | Restrict the inspected series. |

The inspector reports `active`, the crosshair `position`, `cursorX` and
`cursorY` with their formatted texts, `validCount`, and `model`.

### Tooltip rows

`inspector.model` has one row per inspected series, updated in place as the
cursor moves. Replace `InspectionTooltip.rowDelegate` to change a tooltip row,
and declare the roles it uses as required properties:

```qml
QAccelPlot.InspectionTooltip {
    rowDelegate: Text {
        required property bool valid
        required property string seriesName
        required property string yText
        visible: valid
        text: seriesName + " = " + yText
    }
}
```

[`InspectionRowModel`][row-model] lists the roles. Text roles such as `yText`
use the axis' tick formatter at a precision that follows the zoom level.

### Cursor from code

Set `followPointer: false` and write `cursorX`, `cursorY`, or both to place the
cursor in data coordinates. With one coordinate, series are matched along that
axis in every mode. Link two plots by binding one inspector to the other:

```qml
QAccelPlot.PlotInspector { id: top; plot: topPlot }
QAccelPlot.PlotInspector { plot: bottomPlot; followPointer: false; cursorX: top.cursorX }
```

`stepCursor(n)` moves the cursor `n` source records along the first matching
series and stops following the mouse pointer; use it for keyboard navigation.

## Queries

Every series exposes its queries as `series.inspection`. Call them on the series'
thread. Pixel arguments are series-local logical pixels: map a plot-local
position with `series.mapFromItem(plot, position)`.

```qml
const dataPoint = curve.inspection.nearestByX(curve.mapFromItem(plotView, plotView.pointerPosition).x)
if (dataPoint.valid)
    console.log(dataPoint.index, dataPoint.x, dataPoint.y)
```

| Query | Methods |
| --- | --- |
| A data point by index | `dataPointAt()` |
| Nearest valid data point | `nearest()`, `nearestByX()`, `nearestByY()` |
| Data points on either side and the point between them | `bracketByX()`, `bracketByY()` |
| Y statistics of a region or an X interval | `summarize()`, `summarizeRange()` |
| Source indices inside a region, in pages | `indices()` |
| Native record of a bar, rectangle, or band series | `recordAt()`, `recordAtPosition()` |

Results carry a `status` and a `valid` flag. Compare `status` with
`QAccelPlot.Inspection.Ready`, `NoMatch`, `Preparing`, or another status.
[`SeriesInspection`][series-inspection] specifies regions, tie-breaking, and
paging.

### Readiness

| Data | Behavior |
| --- | --- |
| X or Y finite and non-decreasing | Searched in place. No preparation and no index; `appendData()` keeps queries available. |
| Unordered, up to 20 000 records | Scanned per query. No preparation. |
| Unordered, more records | Indexed on a worker thread. Queries return `Preparing` until `inspection.status` is `Ready`; `statusChanged` announces it. |

- `nearestByX()`, `bracketByX()`, and their Y counterparts need the order along
  their own axis.
- `prepare()` starts indexing before the first query.
- Any data change discards the index. A stream that replaces unordered data
  faster than it can be indexed never becomes `Ready`.
- The index takes about 40 MB per million valid data points;
  `inspection.indexBytes()` reports the actual size.

## Selection

[`SelectionTool`][selection-tool] selects a `Box`, an `XRange`, or a `YRange`.
`button`, `modifiers`, and `minimumSize` configure the gesture; the modifiers
must match exactly and a shorter drag only clears the selection.

- The region is `xMin`, `xMax`, `yMin`, `yMax` in the coordinates of `xAxis` and
  `yAxis`, which default to the plot's primary axes. A range selection is
  unbounded in its other dimension.
- Visible XY series bound to the axis of every bounded dimension are selected,
  so an X range also covers series on a secondary Y axis.
- `model` has one row of statistics per selected series, recomputed when data
  changes. `indices(series, offset, limit)` pages through the selected data points.
- `select(xMin, xMax, yMin, yMax)` sets the region from code; `clear()` removes
  it. `selectionChanged` reports a new or cleared region, `completed` a
  finished gesture.
- Set `rectangleVisible: false` to draw `pixelRect` yourself.

Complete source: [`examples/interaction/data_inspection`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/interaction/data_inspection)

[plot-inspector]: ../api/classQAccelPlot_1_1PlotInspector.md
[row-model]: ../api/classQAccelPlot_1_1InspectionRowModel.md
[series-inspection]: ../api/classQAccelPlot_1_1SeriesInspection.md
[selection-tool]: ../api/classQAccelPlot_1_1SelectionTool.md
