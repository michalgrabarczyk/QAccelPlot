---
description: "Build a real-time scrolling QAccelPlot chart with stable axis ranges, frame-synced updates, and efficient data handoff."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Real-time scrolling data

A scrolling plot normally keeps a fixed axis range and replaces the visible
buffer at the display cadence.

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

## Update after animation processing

The included example listens to [`QQuickWindow::afterAnimating`][after-animating],
produces the current interleaved buffer, and skips repeated range scans:

```cpp
QObject::connect(window, &QQuickWindow::afterAnimating, &app, [curve]() {
    auto points = buildCurrentWindow();
    curve->setDataFNoRange(std::move(points), pointCount);
});
```

Initialize once with [`setDataF()`][set-data-f] or declare accurate axis data ranges before
using the no-range path. If the signal can exceed those bounds, update the
ranges explicitly.

For sampled signals, anchor samples to a stable time grid. Re-evaluating every
sample at a slightly different phase each frame can look like waveform jitter
rather than horizontal motion.

When buffer generation could compete with rendering, move it to a worker
thread. See [Background data production](background-data.md) for safe handoff
patterns.

## Show dropouts as gaps

When packets are lost or a sensor reports an invalid value, write `NaN` for
that sample instead of repeating the last value or dropping the sample. The
curve breaks there, the gap is excluded from auto-ranging, and hover never
bridges it:

```cpp
const auto value = packet.valid ? packet.value : std::numeric_limits<float>::quiet_NaN();
points[index * 2] = timestamp;
points[index * 2 + 1] = value;
```

To draw a continuous trace across short dropouts instead, set
`gaps.nanMode: QAccelPlot.NanGapMode.Connect` on the curve. See
[Invalid samples and gaps](../concepts.md#invalid-samples-and-gaps) for the
complete contract, including `±Inf` and non-positive values on logarithmic axes.

Complete source: [`examples/realtime`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/realtime)

[after-animating]: https://doc.qt.io/qt-6/qquickwindow.html#afterAnimating
[set-data-f]: ../api/classQAccelPlot_1_1LineCurve.md#function-setdataf-22
