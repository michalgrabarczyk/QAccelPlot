---
description: "Build a real-time scrolling QAccelPlot chart with stable axis ranges, frame-synced updates, and efficient data handoff."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Real-time scrolling data

Keep the viewport fixed and replace the visible buffer once per frame.

## Declare a stable viewport

```qml
import QtQuick
import QAccelPlot as QAccelPlot

QAccelPlot.Plot {
    id: plot

    xAxis: QAccelPlot.Axis {
        viewportMin: -20
        viewportMax: 0
        label: "Time before present (s)"
    }

    yAxis: QAccelPlot.Axis {
        viewportMin: -2.5
        viewportMax: 2.5
        label: "Acceleration (g)"
    }

    QAccelPlot.LineCurve {
        objectName: "vibrationCurve"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        lineWidth: 2.5
    }
}
```

## Update every frame

Pass each new buffer to [`setDataF()`][set-data-f]. No data update scans the
buffer for its range; that happens only when something reads the range, such
as a double-click rescale:

```cpp
curve->setDataF(std::move(points), pointCount);
```

A rescale then fits the samples of that frame. To fit the nominal window
instead, and skip the scan, pass its bounds with the data:

```cpp
constexpr auto bounds = QAccelPlot::PlotSeries::DataBounds{-20.0, 0.0, -2.5, 2.5};
curve->setDataF(std::move(points), pointCount, bounds);
```

If buffer generation is expensive, move it to a worker thread; see
[Background data production](background-data.md).

## Follow the data range

To fit an axis to the data instead of fixing its viewport, set
[`autoRescale`][auto-rescale]. The viewport is refitted on every data update,
including each [`appendData()`][append-data] call:

```qml
yAxis: QAccelPlot.Axis {
    autoRescale: true
}
```

- The fit is exact, as with [`rescaleToData()`][rescale-to-data].
- A panned or zoomed viewport is replaced on the next update. Set
  `autoRescale` to `false` while the user navigates.
- Appending only extends the data range. Replace the buffer with
  [`setDataF()`][set-data-f] to let the range shrink.
- Each update scans the data for its range. Appending widens the range
  without a scan, and data passed with bounds is not scanned.
- When the last series on the axis is cleared, the viewport is kept.

## Show dropouts as gaps

Write `NaN` for lost or invalid samples instead of repeating or dropping them.
The curve breaks there, and the gap is excluded from auto-ranging and hover:

```cpp
const auto value = packet.valid ? packet.value : std::numeric_limits<float>::quiet_NaN();
points[index * 2] = timestamp;
points[index * 2 + 1] = value;
```

To draw across dropouts instead, set
`gaps.nanMode: QAccelPlot.NanGapMode.Connect`. See
[Invalid samples and gaps](../concepts.md#invalid-samples-and-gaps).

Complete sources:

- [`examples/data/realtime`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/data/realtime)
- [`examples/data/missing_data`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/data/missing_data)

[set-data-f]: ../api/classQAccelPlot_1_1LineCurve.md#function-setdataf-22
[auto-rescale]: ../api/classQAccelPlot_1_1Axis.md#property-autorescale-12
[append-data]: ../api/classQAccelPlot_1_1LineCurve.md#function-appenddata
[rescale-to-data]: ../api/classQAccelPlot_1_1Axis.md#function-rescaletodata
