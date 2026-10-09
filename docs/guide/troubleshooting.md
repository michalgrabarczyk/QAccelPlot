---
description: "Troubleshooting common QAccelPlot issues — blank curves, missing QML modules, QuickPrivate errors, rescale problems, and real-time stuttering."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Troubleshooting

## `module "QAccelPlot" is not installed`

QAccelPlot is a static QML module. Check that the application:

- links `QAccelPlot::QAccelPlot`,
- calls `qt_import_qml_plugins()` before `qt_finalize_executable()`,
- imports `QAccelPlot` in QML.

See the CMake file in [Getting started](getting-started.md). Delete the
application build directory after changing plugin linkage; stale generated
import files can hide the fix.

## CMake cannot find `QuickPrivate`

`QACCELPLOT_USE_QT_PRIVATE_API=ON` requires `Qt6::QuickPrivate`. Qt 6.11 and
newer package it as a separate component. Install the private development
package for the exact Qt build, and check `Qt6_DIR`, `CMAKE_PREFIX_PATH`, the
compiler architecture, and the Qt version reported during configuration. A
package from a different Qt patch release, compiler, or architecture does not
work.

To build without it:

```sh
cmake -S . -B build -DQACCELPLOT_USE_QT_PRIVATE_API=OFF
```

This mode recreates the data texture on each update instead of updating it in
place, which is slower for large data sets updated every frame.

## The axes appear but the curve is blank

Check these conditions:

1. The `LineCurve` has both [`xAxis`][series-x-axis] and [`yAxis`][series-y-axis] assigned.
2. At least two points exist when a line is enabled, or one point when only a
   marker is enabled.
3. The viewport includes the point coordinates.
4. The Qt Quick renderer is not using the software backend.
5. The data buffer contains exactly two floats per declared point.

On the software backend, QAccelPlot logs a warning that curve rendering is
unavailable.

## Rescale does not fit the data

Data passed with `DataBounds` is not scanned: a rescale fits the given bounds,
so they must cover the data. Every series attached to an axis contributes to
its data range.

## Pan and zoom snap back

An axis with [`autoRescale`][auto-rescale] refits its viewport on every
data update. Set `autoRescale` to `false` while the user navigates.

## A real-time plot stutters

- Run the data producer off the UI thread.
- Use a Release build.
- Move interleaved float buffers instead of building `QList<QPointF>`.
- Do not publish more frames than the display presents.
- Disable markers, gradients, and transitions to isolate rendering cost.
- Measure [`frameSwapped`][frame-swapped] intervals and producer throughput separately.

See the [Performance guide](performance.md) and
[Background data production](cookbook/background-data.md).

## Reporting a problem

Include:

- A minimal reproducer or the closest included example.
- QAccelPlot version.
- Qt version and installation source.
- OS, compiler, architecture, GPU, and driver.
- Active Qt Quick graphics API.
- Release or Debug configuration.
- Logs, screenshots, or benchmark JSON where relevant.

Report through [GitHub Issues](https://github.com/michalgrabarczyk/QAccelPlot/issues).

[series-x-axis]: api/classQAccelPlot_1_1PlotSeries.md#property-xaxis-12
[series-y-axis]: api/classQAccelPlot_1_1PlotSeries.md#property-yaxis-12
[auto-rescale]: api/classQAccelPlot_1_1Axis.md#property-autorescale-12
[frame-swapped]: https://doc.qt.io/qt-6/qquickwindow.html#frameSwapped
