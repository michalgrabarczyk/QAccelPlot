---
description: "QAccelPlot performance guide — data-ingestion API comparison, efficient buffer strategies, worker-thread handoff, hover cost analysis, and benchmarking methodology."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Performance

Throughput depends on point count, update rate, data conversion, range scans,
series count, line style, effects, window size, graphics backend, and refresh
rate. Measure a workload that matches your application. For cross-library
numbers, see the [performance comparison](performance-comparison.md).

## Select the data path

| API | Use it when | Cost |
| --- | --- | --- |
| [`setData(QList<QPointF>)`][set-data-points] | Data originates in QML | Allocation and interleaving |
| [`setData(xs, ys)`][set-data-vectors] | C++ data is in separate double vectors | Allocation and interleaving |
| [`setData(std::vector<double>&&, count)`][set-data-moved] | C++ data is interleaved doubles | Moved, no copy |
| [`setDataF(const float*, count)`][set-data-f-copy] | Interleaved floats stay caller-owned | One copy into a reusable buffer |
| [`setDataF(std::vector<float>&&, count)`][set-data-f-moved] | Interleaved floats, on the curve's thread | Moved, no copy |
| `postData()` with a [float][post-data] or [double][post-data-double] vector | A worker thread owns the completed buffer | Queues the moved-buffer handoff |

Start with [`setDataF(std::vector<float>&&, count)`][set-data-f-moved] for large
C++ data sets.

Use a [`setData()`][set-data-points] overload for Unix-epoch timestamps, large
coordinates, or small differences at a large offset. These paths keep doubles
and rebase the GPU buffer so nearby values stay distinct. The `F` APIs store
floats.

The table shows `LineCurve`. Every series has `setData()`,
`setDataF()`, and `postData()` overloads for its own record layout.

[`setDataFWithCache()`][set-data-f-with-cache] also accepts a
prebuilt vertex cache, so vertex assembly can move off the UI and render
threads. It ties application code to renderer details; use it only when
measurements justify it. The cache must match the current point count and
renderer layout, otherwise it is rejected and normal vertex assembly runs. For
a step line, build it with `buildVertexCache(data, count, cache, stepMode)`.

## Data ranges are computed on demand

No setter scans the data for its range. After an update, a series scans its
records the first time something reads the range:

| Reader | Scans |
| --- | --- |
| [`rescaleToData()`][rescale-to-data], `rescaleAllAxes()`, a double-click | Once per call |
| An axis with [`autoRescale`][auto-rescale] | On every data update |
| [`dataMin`][data-min] / [`dataMax`][data-max], or a binding to them | On every data update that is followed by a read |
| A `LineCurve` gradient with a `DataRange` bound | On every data update of that curve |

[`appendData()`][append-data] widens a computed range without a rescan.

To skip the scan when the extents are already known, pass them with the data:

```cpp
const auto bounds = QAccelPlot::PlotSeries::DataBounds{xMin, xMax, yMin, yMax};
curve->setDataF(std::move(points), pointCount, bounds);
```

The bounds describe that update only. A change that affects which values are
valid, such as switching an axis to a logarithmic scale, discards them.

## Build buffers efficiently

- For the `F` paths, store points as interleaved floats: `[x0, y0, x1, y1, ...]`.
- Reserve or resize the vector once before filling it.
- Move the completed vector instead of copying it.
- Publish no faster than the display presents frames.
- For a fixed streaming window, pass the window's bounds with each frame, as in
  the [`realtime` example](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/data/realtime).

## Keep work off the UI thread

Generate data on a worker thread and hand off only completed buffers, using
[`postData()`][post-data] or a queued call. Never write to a `QQuickItem` from
the worker. If producers are faster than the display, keep at most one pending
batch; see [Background data production](cookbook/background-data.md).

## Optional rendering costs

### Hover interactions

Hover cost per mouse move:

| Series | Cost |
| --- | --- |
| `LineCurve` | Checks one bounding box per 512 points, then the segments and markers in the matching boxes. `hoverRadius: 0` skips the segments. |
| `PointCloud`, `RectangleSeries` | Tests every record, roughly 1–2 ms per million, until those scans have cost as much as a spatial index. The index is then built, and later moves test only nearby records. Data replaced every frame never builds one. |
| `BarSeries` | Builds a grid on the first move after a data change, then tests nearby bars. |

A `RectangleSeries` also tests every rectangle spanning more than 64 grid
cells, so many long rectangles make its lookup `O(N)`.

Set `hoverEnabled: false` to turn hover off for one series. Set the environment
variable `QACCELPLOT_HOVER_ENABLED=0` to turn it off for every series and axis,
for example in streaming-only views.

### Other rendering costs

- Markers add geometry and fragment work per visible point. Fragment work grows
  with marker size; `LineCurve.Pixel` markers shade one pixel per point and
  suit very dense scatter plots.
- A `PointCloud` or `HeatmapSeries` whose `Colormap` leaves `min` or `max` unset scans every value
  on each data update to resolve the range. Set both when the bounds of
  streaming values are known.
- Gradient effects add material and domain work.
- Transitions update intermediate data while animating.
- Many series add per-node and per-material overhead at the same total point
  count.
- Larger windows and high-DPI displays add fragment work.

Changing line width, marker size, or color is cheaper than changing data, but
still schedules a redraw.

## Use a hardware backend

Curves require a hardware Qt Quick Scene Graph backend: OpenGL, OpenGL ES,
Direct3D 11/12, Vulkan, or Metal. The software backend skips curve rendering and
emits a warning. When a curve is blank or slow, check `QSG_RHI_BACKEND`,
`QT_QUICK_BACKEND`, remote-desktop sessions, and GPU drivers.

## Select the texture upload mode

Curve coordinates live in a GPU data texture. How live updates reach it depends
on `QACCELPLOT_USE_QT_PRIVATE_API`:

| Mode | Upload | Use when |
| --- | --- | --- |
| `ON` (default when `Qt6::QuickPrivate` is found) | `QSGPlainTexture::setImage()` replaces the texture contents in place | Large data sets updated every frame |
| `OFF` | `QQuickWindow::createTextureFromImage()` recreates the texture | Private Qt dependencies are unacceptable for packaging |

No private Qt types appear in public headers or ABI. Gradient textures always
use the public path.

Benchmark both modes with your workload, or configure the
[included benchmarks](#included-benchmarks) once with each value.

## Benchmarking

- Build QAccelPlot and the application in Release mode, and warm up shaders,
  allocations, and caches before recording.
- Record presented frames ([`QQuickWindow::frameSwapped`][frame-swapped]), not
  timer callbacks, and report the frame-time distribution as well as the
  average.
- For streamed data, also report how many new datasets are applied per second:
  several presented frames can show the same dataset.
- Record the Qt version, OS, GPU, driver, graphics API, resolution, and device
  pixel ratio with the result.

### Performance Lab

[`examples/showcases/performance`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/showcases/performance)
streams a `LineCurve`, a `PointCloud`, a `RectangleSeries`, a `BarSeries`, and
a `HeatmapSeries`, one per tab, with options for record and series count, ingestion API, hover,
and rendering. Change one option at a time, at a record count that keeps
display FPS below the refresh rate. The lab reports:

- **Display FPS** — frames presented to the screen.
- **Data Update Rate** (Hz) — new datasets applied to the plot per second,
  including worker-side generation.
- **Peak update gap** — the longest interval between two dataset applications
  in the last reporting period.
- **Throughput** — points, rectangles, bars, or heatmap cells updated per
  second.

### Included benchmarks

- Frame time runs from just before the data or viewport update until
  `QQuickRenderControl::endFrame()` returns: data copy, scene-graph sync,
  texture staging, rendering, command submission, and GPU completion. Data
  generation is excluded.
- Live scenarios cover 1M, 2M, 5M, and 10M points; static scenarios
  pan the viewport over one fixed buffer.
- Multi-curve throughput counts points across all curves.
- Separate executables measure CPU ingestion and vertex-cache building without
  rendering.

Build them with:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DQACCELPLOT_BUILD_BENCHMARKS=ON \
  -DQACCELPLOT_BUILD_EXAMPLES=OFF \
  -DQACCELPLOT_BUILD_TESTS=OFF
cmake --build build --config Release
```

Scenarios are defined in
[`benchmarks/common/BenchmarkScenario.cpp`](https://github.com/michalgrabarczyk/QAccelPlot/blob/main/benchmarks/common/BenchmarkScenario.cpp).

[set-data-points]: api/classQAccelPlot_1_1LineCurve.md#function-setdata-18
[set-data-vectors]: api/classQAccelPlot_1_1LineCurve.md#function-setdata-28
[set-data-moved]: api/classQAccelPlot_1_1LineCurve.md#function-setdata-48
[set-data-f-copy]: api/classQAccelPlot_1_1LineCurve.md#function-setdataf-16
[set-data-f-moved]: api/classQAccelPlot_1_1LineCurve.md#function-setdataf-26
[set-data-f-with-cache]: api/classQAccelPlot_1_1LineCurve.md#function-setdatafwithcache-12
[append-data]: api/classQAccelPlot_1_1LineCurve.md#function-appenddata
[rescale-to-data]: api/classQAccelPlot_1_1Axis.md#function-rescaletodata
[auto-rescale]: api/classQAccelPlot_1_1Axis.md#property-autorescale-12
[post-data]: api/classQAccelPlot_1_1LineCurve.md#function-postdata-22
[post-data-double]: api/classQAccelPlot_1_1LineCurve.md#function-postdata-12
[data-min]: api/classQAccelPlot_1_1Axis.md#property-datamin-12
[data-max]: api/classQAccelPlot_1_1Axis.md#property-datamax-12
[frame-swapped]: https://doc.qt.io/qt-6/qquickwindow.html#frameSwapped
