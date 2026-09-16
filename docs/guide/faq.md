---
description: "Frequently asked questions about QAccelPlot licensing, performance, threading, Qt compatibility, and differences from other Qt plotting libraries."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# FAQ

## How does QAccelPlot differ from QCustomPlot or Qt Graphs?

| | QAccelPlot | Qt Graphs | QCustomPlot |
| --- | --- | --- | --- |
| UI framework | Qt Quick | Qt Quick | Qt Widgets |
| Rendering | Custom Scene Graph shaders | Qt Scene Graph | `QPainter`, optional OpenGL |
| Points replaced per frame at 60 FPS\* | ~8M | ~13K | ~2.75M (OpenGL) |

\* Single curve on the [reference system](performance-comparison.md).

## How many data points can QAccelPlot handle?

About 8M points replaced and drawn every frame at 60 FPS on the
[reference system](performance-comparison.md). Throughput drops with more
series, markers, gradients, transitions, larger windows, and slower GPUs.
Measure your workload with the included
[benchmarks](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/benchmarks).

## What is the fastest way to supply data?

Move an interleaved float buffer with
[`setDataF(std::vector<float>&&, int)`](api/classQAccelPlot_1_1LineCurve.md#function-setdataf-22).
If axis data ranges are fixed, `setDataFNoRange()` also skips the range scan.
See [Select the data path](performance.md#select-the-data-path).

## When should I use `postData()` instead of `setDataF()`?

Use [`postData()`](api/classQAccelPlot_1_1LineCurve.md#function-postdata) from
a worker thread; it queues the buffer transfer to the curve's thread.
`setDataF()` must be called on the curve's thread, normally the UI thread.

## Is QAccelPlot thread-safe?

No. Access QML objects and properties on the UI thread. Produce data on any
thread and hand it off through `postData()` or a queued call. See
[Background data production](cookbook/background-data.md).

## Which Qt versions are supported?

Qt 6.2 or newer with Core, Gui, Quick, and ShaderTools. When the matching
`QuickPrivate` component is available, QAccelPlot uses it for in-place data
texture updates, which is faster for large data sets updated every frame.
`-DQACCELPLOT_USE_QT_PRIVATE_API=OFF` builds against public Qt API only. No
private Qt types appear in public headers or ABI.

## What graphics backends does QAccelPlot support?

OpenGL, OpenGL ES, Direct3D 11/12, Vulkan, and Metal. The software backend
cannot run the curve shaders; if a curve is blank, check that
`QSG_RHI_BACKEND` and `QT_QUICK_BACKEND` are not set to `software`.

## Does QAccelPlot work with Qt Widgets?

Only through `QQuickWidget`. There is no `QWidget`-based API.

## What licenses are available?

- **GPLv3 with the Universal FOSS Exception** — free for applications
  distributed under an OSI-approved or FSF-free license.
- **Commercial** — for proprietary or closed-source distribution. Contact
  [qaccelplot@gmail.com](mailto:qaccelplot@gmail.com).

See [LICENSING.md](https://github.com/michalgrabarczyk/QAccelPlot/blob/main/LICENSING.md).

## Why is my curve blank?

See [The axes appear but the curve is blank](troubleshooting.md#the-axes-appear-but-the-curve-is-blank).
