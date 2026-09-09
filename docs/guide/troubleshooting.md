---
description: "Troubleshooting common QAccelPlot issues — blank curves, missing QML modules, QuickPrivate errors, rescale problems, and real-time stuttering."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Troubleshooting

## `module "QAccelPlot" is not installed`

QAccelPlot is a static QML module. Verify that the application links
`QAccelPlot::QAccelPlot`, calls `qt_import_qml_plugins()` before finalization,
and imports `QAccelPlot` in QML. The canonical target carries both the backing
library and static QML plugin.
See the complete CMake fragment in [Getting started](getting-started.md).

Delete the application build directory after changing static QML plugin
linkage; stale generated import files can otherwise obscure the fix.

## CMake cannot find `QuickPrivate`

QAccelPlot's default optimized mode uses one narrowly scoped Qt Quick private
API for in-place live-data texture updates. Qt 6.11 and newer package
`QuickPrivate` as a separately discoverable component. Install the
private/development package matching the exact Qt build, and ensure CMake is
not mixing Qt installations.

Inspect `Qt6_DIR`, `CMAKE_PREFIX_PATH`, the compiler architecture, and the Qt
version reported during configuration. A package from a different Qt patch,
compiler, or architecture is not interchangeable.

If private Qt dependencies are unsuitable for the target environment, use:

```sh
cmake -S . -B build -DQACCELPLOT_USE_QT_PRIVATE_API=OFF
```

This mode does not find or link QuickPrivate. It uses only public Qt API and
recreates the live data texture rather than updating it in place, which can be
slower for huge data sets updated every frame.

## The axes appear but the curve is blank

Check these conditions:

1. The `LineCurve` has both [`xAxis`][series-x-axis] and [`yAxis`][series-y-axis] assigned.
2. At least two points exist when a line is enabled, or one point when only a
   marker is enabled.
3. The viewport includes the point coordinates.
4. The Qt Quick renderer is not using the software backend.
5. The data buffer contains exactly two floats per declared point.

Run with Qt logging enabled and look for the QAccelPlot warning that custom
rendering is unavailable on the software backend.

## Rescale does not show all new data

[`setDataFNoRange()`][set-data-f-no-range] intentionally skips data-range
calculation. Set accurate [`dataMin`][data-min] and [`dataMax`][data-max]
values on the axes, or occasionally update through [`setDataF()`][set-data-f]
before calling [`rescaleToData()`][rescale-to-data] or
[`rescaleAllAxes()`][rescale-all-axes].

If several series share an axis, remember that each series contributes to the
axis data range.

## A real-time plot stutters

- Confirm that the data producer is not running on the UI thread.
- Use a Release build.
- Move interleaved float buffers rather than rebuilding `QList<QPointF>` values.
- Avoid publishing more frames than the display can present.
- Temporarily disable markers, gradients, transitions, and sharp-corner
  preservation to isolate rendering cost.
- Measure [`frameSwapped`][frame-swapped] intervals and producer throughput separately.

Continue with the [Performance guide](performance.md) and the
[Background data production recipe](cookbook/background-data.md).

## Reporting a problem

Include:

- A minimal reproducer or the closest included example.
- QAccelPlot version.
- Qt version and installation source.
- OS, compiler, architecture, GPU, and driver.
- Active Qt Quick graphics API.
- Release or Debug configuration.
- Logs, screenshots, or benchmark JSON where relevant.

Report bugs and compatibility findings through
[GitHub Issues](https://github.com/michalgrabarczyk/QAccelPlot/issues).

[series-x-axis]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_plot_series.html#a250150d8eaea44c2608375839747b2b2
[series-y-axis]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_plot_series.html#ae021f84424e1ea4ad44576961ac7d402
[set-data-f-no-range]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_line_curve.html#a80e4d6c6ba351c76333ef4177f3c6e30
[data-min]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_axis.html#a7ff8bbf8f594cce69d3a56c9b7f7dc25
[data-max]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_axis.html#ab9a1549ca7fa37509039903052111c85
[set-data-f]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_line_curve.html#ac99cd1571b8153cae96791c8206d9de6
[rescale-to-data]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_axis.html#a1a4057e12caae2776590a0ce4bc38a84
[rescale-all-axes]: https://michalgrabarczyk.github.io/QAccelPlot/api/class_q_accel_plot_1_1_q_accel_plot.html#a6ab962feb97d0bfccb268ac54bce8c77
[frame-swapped]: https://doc.qt.io/qt-6/qquickwindow.html#frameSwapped
