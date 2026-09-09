---
description: "Frequently asked questions about QAccelPlot licensing, performance, threading, Qt compatibility, and differences from other Qt plotting libraries."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# FAQ

## What is QAccelPlot?

QAccelPlot is a high-performance plotting library for Qt Quick. It uses the Qt
Scene Graph for hardware-accelerated GPU rendering and provides QML-friendly
plot components backed by C++. It is designed for real-time visualization of
large data sets — hundreds of thousands of points at display refresh rate.

## How does QAccelPlot differ from QCustomPlot or Qt Charts?

QCustomPlot renders on the CPU using `QPainter` inside a `QWidget`.
Qt Charts also uses `QPainter` (or a Qt Quick `ChartView`).
QAccelPlot renders through the Qt Quick Scene Graph using precompiled GPU
shaders (OpenGL, Direct3D, Vulkan, Metal via RHI), which makes it
significantly faster for large and frequently updated data sets in QML
applications. QAccelPlot is designed exclusively for Qt Quick, not Qt Widgets.

## What licenses are available?

QAccelPlot is dual-licensed:

1. **Open source** — GPLv3 with the Universal FOSS Exception. This permits
   linking with applications distributed under OSI-approved or FSF-free
   licenses.
2. **Commercial** — A separate commercial license for proprietary or
   closed-source products. Contact
   [qaccelplot@gmail.com](mailto:qaccelplot@gmail.com) for details.

See [LICENSING.md](https://github.com/michalgrabarczyk/QAccelPlot/blob/main/LICENSING.md)
for the full explanation.

## Which Qt versions are supported?

Qt 6.2 or newer with Core, Gui, Quick, and ShaderTools modules. The default
optimized build also uses the matching `QuickPrivate` component for in-place
live-data texture updates. Configure with
`-DQACCELPLOT_USE_QT_PRIVATE_API=OFF` for a public-Qt-only build that does not
find or link QuickPrivate.

No private Qt types are exposed through QAccelPlot's public headers or ABI. The
private API is isolated because it provides substantially better performance
for huge, continuously updated data sets; the public fallback favors packaging
portability over maximum live-update throughput.

## What graphics backends does QAccelPlot support?

QAccelPlot works with any hardware Qt Quick Scene Graph backend: OpenGL,
Direct3D 11/12, Vulkan, and Metal. The **software backend cannot render
QAccelPlot curves** because it cannot execute custom shaders. If your curve is
blank, check that `QSG_RHI_BACKEND` and `QT_QUICK_BACKEND` are not set to
`software`.

## How many data points can QAccelPlot handle?

QAccelPlot is designed for hundreds of thousands of points per curve. Actual
throughput depends on update frequency, series count, line style, effects,
window size, graphics backend, and display refresh rate. Use the included
[benchmarks](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/benchmarks)
to measure your specific workload.

## What is the fastest way to supply data?

For maximum throughput, use
[`setDataF(std::vector<float>&&, int)`](https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_line_curve.html#ac99cd1571b8153cae96791c8206d9de6)
which moves an already-interleaved float buffer with zero allocation. For
fixed axis ranges, `setDataFNoRange()` additionally skips the range scan.
See the [Performance guide](performance.md#select-the-data-path) for a
comparison of all data-ingestion APIs.

## When should I use `postData()` instead of `setDataF()`?

Use [`postData()`](https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_line_curve.html#a8b3d0effe4bd115f091ad505a7787b2c)
when data is produced on a **worker thread**. It queues the buffer transfer to
the UI thread safely. `setDataF()` must be called on the curve's own thread
(normally the UI thread). Never write directly to a `QQuickItem` from a worker.

## Is QAccelPlot thread-safe?

QML objects and their properties must be accessed on the UI thread. Data
*production* can happen on any thread, but the handoff to the curve must go
through `postData()` or be queued to the UI thread. See
[Background data production](cookbook/background-data.md).

## Does QAccelPlot work with Qt Widgets?

No. QAccelPlot is designed exclusively for Qt Quick. It relies on the Qt Quick
Scene Graph for rendering and does not provide a `QWidget`-based API. If your
application uses Qt Widgets, consider embedding QAccelPlot inside a
`QQuickWidget` or using a different plotting library.

## Can I use QAccelPlot in a commercial product?

Yes. If your product is distributed under a qualifying open-source license
(OSI-approved or FSF-free), you can use the open-source GPLv3 + Universal FOSS
Exception license at no cost. For proprietary or closed-source distribution,
obtain a [commercial license](https://github.com/michalgrabarczyk/QAccelPlot/blob/main/COMMERCIAL-LICENSING.md).

## Why is my curve blank?

Check these conditions:

1. The `LineCurve` has both `xAxis` and `yAxis` assigned.
2. At least two points exist when a line is enabled.
3. The viewport includes the point coordinates.
4. The Qt Quick renderer is not using the software backend.
5. The data buffer contains exactly two floats per declared point.

See [Troubleshooting](troubleshooting.md) for more details.
