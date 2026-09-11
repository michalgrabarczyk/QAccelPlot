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
        side: QAccelPlot.Axis.Bottom
        label: "Time before present (s)"
    }

    yAxis: QAccelPlot.Axis {
        viewportMin: -2.5
        viewportMax: 2.5
        dataMin: -2.5
        dataMax: 2.5
        side: QAccelPlot.Axis.Left
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

Complete source: [`examples/realtime`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/realtime)

[after-animating]: https://doc.qt.io/qt-6/qquickwindow.html#afterAnimating
[set-data-f]: ../api/classQAccelPlot_1_1LineCurve.md#function-setdataf-22
