---
description: "Crosshairs, tooltips, nearest-sample queries, region statistics, and selection for large and live datasets."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Data inspection

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
    QAccelPlot.SelectionTool { id: selection; plot: plotView }
    QAccelPlot.SelectionOverlay { tool: selection }
}
```

- Components declared inside a `PlotInspector` use it and are drawn in
  declaration order, the last one on top.
- They are children of `plotView.overlay`, an item above the series and the
  legend. Parent your own overlays to it instead of choosing a `z` relative to
  the series.
- A component declared elsewhere needs `inspector:` set and is not restacked.

Hovering shows a crosshair, a marker on every series, and a tooltip. Shift + left
drag selects a region; Escape clears it. Ordinary left dragging still pans.

## Inspector

`PlotInspector` queries every visible `LineCurve` and `PointCloud` of the plot
at the cursor. Other series types have no row.

| Property | Effect |
| --- | --- |
| `mode` | `NearestX` reports each series at the cursor's X. `NearestY` does the same at the cursor's Y, for series whose independent variable is Y. `NearestXY` picks the sample closest on screen. |
| `radius` | Pick distance in logical pixels. In `NearestX` and `NearestY` it only applies beyond a series' extreme samples; between two samples a series always matches. |
| `interpolate` | In `NearestX` and `NearestY`, report the point on the line between two consecutive samples. |
| `snapToSample` | Move the crosshair to the closest matching sample. |
| `summaries`, `summaryRadius` | Add the count and Y statistics of the samples within `summaryRadius` pixels of the cursor. Use it for dense data, where one pixel column holds many samples. `NearestY` covers the horizontal band around the cursor. |
| `includedSeries`, `excludedSeries` | Restrict the inspected series. |

Results:

- `active` — the cursor is inside the plot area.
- `position` — crosshair position in plot-local pixels. A coordinate the cursor
  lacks is NaN.
- `cursorX`, `cursorY`, `cursorXText`, `cursorYText` — crosshair position on the
  plot's primary axes.
- `validCount` — number of series with a match.
- `model` — one row per inspected series.

### Rows

`inspector.model` is a list model. Rows are created when the set of inspected
series changes and updated in place as the cursor moves, so delegates are reused.

| Role | Value |
| --- | --- |
| `series`, `seriesName`, `seriesColor` | The series, its name, and its `color`. |
| `valid`, `sampleStatus` | Whether a sample matched, and why not. |
| `sampleIndex`, `sampleX`, `sampleY`, `sampleValue` | The source index and values. `sampleValue` is the `PointCloud` scalar. |
| `xText`, `yText` | Values formatted by the series' axes. |
| `pixelPosition`, `distance`, `interpolated` | Plot-local sample position, pixel distance from the cursor, and whether the point is interpolated. |
| `hasSummary`, `summaryCount`, `minimum`, `maximum`, `mean`, `standardDeviation`, `minimumIndex`, `maximumIndex` | Neighborhood statistics. |
| `minimumText`, `maximumText`, `meanText` | Statistics formatted by the series' Y axis. |

Replace `InspectionTooltip.rowDelegate` to change a tooltip row. Declare the
roles it uses as required properties:

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

Texts use `Axis.formatValue(value, length)`: the axis' tick formatter with a
precision one digit finer than a pixel, so they follow the zoom level.

### Cursor from code

Set `followPointer: false` and write `cursorX`, `cursorY`, or both to place the
cursor in data coordinates. With one coordinate, series are matched along that
axis in every mode. Link two plots by binding one inspector to the other:

```qml
QAccelPlot.PlotInspector { id: top; plot: topPlot }
QAccelPlot.PlotInspector { plot: bottomPlot; followPointer: false; cursorX: top.cursorX }
```

`stepCursor(n)` moves the cursor `n` source records along the first matching
series and stops following the pointer; use it for keyboard navigation.

A touch places the cursor at the touched point and leaves it there after the
release.

## Queries

Every series exposes its queries as `series.inspection`. Call them on the series'
thread. Pixel arguments are series-local logical pixels: map a plot-local
position with `series.mapFromItem(plot, position)`.

| Method | Result |
| --- | --- |
| `sampleAt(index)` | The record at a source index. |
| `nearest(position, radius)` | Nearest valid sample by screen distance. |
| `nearestByX(pixelX, radius)` | Nearest valid sample by horizontal distance. |
| `bracketByX(pixelX)` | The valid samples on either side, whether they are consecutive, and the point between them. |
| `nearestByY(pixelY, radius)`, `bracketByY(pixelY)` | The same along Y. |
| `summarize(xMin, xMax, yMin, yMax)` | Y statistics inside a region. |
| `summarizeRange(xMin, xMax)` | Y statistics over an X interval. |
| `indices(xMin, xMax, yMin, yMax, offset, limit)` | One page of source indices inside a region. |
| `recordAt(index)`, `recordAtPosition(position)` | Native record of a bar, rectangle, or band series. |

- Results are value objects with a `status` and a `valid` flag. Compare `status`
  with `QAccelPlot.Inspection.Ready`, `NoMatch`, `Idle`, `Preparing`,
  `Unsupported`, `Unavailable`, `InvalidArgument`, or `Stale`.
- Regions are inclusive. Reversed limits are swapped. `Infinity` leaves a side
  unbounded.
- Indices refer to the supplied records, including invalid ones. Invalid samples
  (NaN, infinite, or non-positive on a logarithmic axis) never match.
- Equal distances resolve to the highest index.
- Statistics are sample-weighted, not time-weighted, and use original Y values.
- Queries cover all stored samples, also outside the viewport. They do not test
  line segments, except for the interpolated point of `bracketByX()` and
  `bracketByY()`.
- `bracketByX()` and `bracketByY()`: `left` is the sample with the largest
  coordinate at or below the position, `right` the one with the smallest
  coordinate above it. Either is invalid beyond the extremes of the data.

```qml
const sample = curve.inspection.nearestByX(curve.mapFromItem(plotView, plotView.pointerPosition).x)
if (sample.valid)
    console.log(sample.index, sample.x, sample.y)
```

### Readiness

| Data | Behavior |
| --- | --- |
| X or Y finite and non-decreasing | Searched in place. No preparation and no index; `appendData()` keeps queries available. |
| Unordered, up to 20 000 records | Scanned per query. No preparation. |
| Unordered, more records | Indexed on a worker thread. Queries return `Preparing` until `inspection.status` is `Ready`; `statusChanged` announces it. |

- `nearestByX()`, `bracketByX()`, and their Y counterparts need the order along
  their own axis. Data ordered along the other axis only counts as unordered for
  them; `status` and `prepare()` refer to queries that take any order.
- The index takes memory on top of the series' own data: about 40 MB per
  million valid samples. `inspection.indexBytes()` reports the actual size.
- Requesting an index copies the series' buffer once on the UI thread; the rest
  of the build runs on the worker.
- Any data change discards the index; the next query requests a new one. A
  stream that replaces unordered data faster than it can be indexed never
  becomes `Ready`.
- `prepare()` starts indexing before the first query. `status` is `Idle` until
  an index is requested.
- Inspection is `Unavailable` while a `LineCurve` data transition is pending and
  for pixel queries without axes or size.

### Pages and revisions

`dataRevision` advances on every accepted data change. `indices()` returns
`total`, `hasMore`, the effective `limit` (at most `maximumPageSize`), and the
`dataRevision` it used. Pass that revision as the last argument of the next call
to get `Stale` instead of indices of newer data. `sourceOrder` is false for
indexed series, whose pages follow the index instead of ascending source order.
In QML, `indices` is a list; `Array.from(page.indices)` gives a JavaScript array.

## Selection

`SelectionTool` selects a `Box`, an `XRange`, or a `YRange`. `button`,
`modifiers`, and `minimumSize` configure the gesture; the modifiers must match
exactly and a shorter drag only clears the selection.

- The region is `xMin`, `xMax`, `yMin`, `yMax` in the coordinates of `xAxis` and
  `yAxis`, which default to the plot's primary axes. A range selection is
  unbounded in its other dimension.
- Visible XY series bound to the axis of every bounded dimension are selected,
  so an X range also covers series on a secondary Y axis.
- `model` has one row per selected series with the summary roles listed above.
- The region is kept when data changes: rows are recomputed and
  `indices(series, offset, limit)` runs against the current data.
- `select(xMin, xMax, yMin, yMax)` sets the region from code. `clear()` and
  Escape remove it. Changing the plot or the selection axes clears it.
- `selectionChanged` reports a new or cleared region, `completed` a finished
  gesture.

`SelectionOverlay` draws `pixelRect` with the tool's `fillColor` and
`borderColor`, which default to the palette's `selectionFill` and
`selectionBorder`.

Complete source:
[`examples/interaction/data_inspection`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/interaction/data_inspection).
