---
description: "Frequently asked questions about QAccelPlot licensing, performance, threading, and Qt compatibility."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# FAQ

## How many data points can QAccelPlot handle?

About 4.5M points replaced and drawn every frame at 60 FPS on the
[reference system](performance.md#what-to-expect). Throughput drops with more
series, markers, gradients, transitions, larger windows, and slower GPUs.
Measure your workload with the included
[benchmarks](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/benchmarks).

## What is the fastest way to supply data?

Move an interleaved float buffer with
[`setDataF(std::vector<float>&&, int)`](api/classQAccelPlot_1_1LineCurve.md#function-setdataf-26).
See [Select the data path](performance.md#select-the-data-path).

## Is QAccelPlot thread-safe?

No. Access QML objects and properties, including `setDataF()`, on the UI
thread. Produce data on any thread and hand it off through
[`postData()`](api/classQAccelPlot_1_1LineCurve.md#function-postdata-22) or a
queued call. See [Background data production](cookbook/background-data.md).

## Which Qt versions are supported?

Qt 6.2 or newer with Core, Gui, Quick, and ShaderTools. `QuickPrivate` is
optional; see
[texture upload modes](performance.md#select-the-texture-upload-mode).

## What graphics backends does QAccelPlot support?

OpenGL, OpenGL ES, Direct3D 11/12, Vulkan, and Metal. The software backend
cannot run the curve shaders; see
[The axes appear but the curve is blank](troubleshooting.md#the-axes-appear-but-the-curve-is-blank).

## Does QAccelPlot work with Qt Widgets?

Only through `QQuickWidget`. There is no `QWidget`-based API.

## What licenses are available?

- **GPLv3 with the Universal FOSS Exception** — free for applications
  distributed under an OSI-approved or FSF-free license.
- **Commercial** — for proprietary or closed-source distribution. Contact
  [qaccelplot@gmail.com](mailto:qaccelplot@gmail.com).

See [LICENSING.md](https://github.com/michalgrabarczyk/QAccelPlot/blob/main/LICENSING.md).
