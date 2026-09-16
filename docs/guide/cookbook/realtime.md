---
description: "Build a real-time scrolling QAccelPlot chart with stable axis ranges, frame-synced updates, and efficient data handoff."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Real-time scrolling data

Keep the axis range fixed and replace the visible buffer once per frame.

## Declare stable ranges

```qml
import QtQuick
import QAccelPlot as QAccelPlot

QAccelPlot.Plot {
    id: plot

    xAxis: QAccelPlot.Axis {
        viewportMin: -20
        viewportMax: 0
        dataMin: -20
        dataMax: 0
        label: "Time before present (s)"
    }

    yAxis: QAccelPlot.Axis {
        viewportMin: -2.5
        viewportMax: 2.5
        dataMin: -2.5
        dataMax: 2.5
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

Pass each new buffer to [`setDataFNoRange()`][set-data-f-no-range], which skips
the range scan:

```cpp
curve->setDataFNoRange(std::move(points), pointCount);
```

Before using the no-range path, declare the axis data ranges or call
[`setDataF()`][set-data-f] once. If the signal can leave those bounds, update
the ranges yourself.

If buffer generation is expensive, move it to a worker thread; see
[Background data production](background-data.md).

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

Complete source: [`examples/realtime`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/realtime)

[set-data-f-no-range]: ../api/classQAccelPlot_1_1LineCurve.md#function-setdatafnorange-12
[set-data-f]: ../api/classQAccelPlot_1_1LineCurve.md#function-setdataf-22
