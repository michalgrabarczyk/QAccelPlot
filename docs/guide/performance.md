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

| API | Use it when | Main cost |
| --- | --- | --- |
| [`setData(QList<QPointF>)`][set-data-points] | Data originates in QML, coordinates require double precision, or convenience matters most | Allocation plus interleaving; coordinates remain doubles |
| [`setData(xs, ys)`][set-data-vectors] | Existing C++ data is stored as separate double vectors or includes large coordinates | Allocation plus interleaving; coordinates remain doubles |
| [`setData(std::vector<double>&&, count)`][set-data-vectors] | Existing C++ data is already interleaved as doubles, e.g. epoch timestamps | Range scan; buffer ownership is moved immediately and coordinates remain doubles |
| [`setDataF(const float*, count)`][set-data-f-copy] | Existing interleaved memory cannot be transferred | One copy plus a range scan |
| [`setDataF(std::vector<float>&&, count)`][set-data-f-moved] | The caller is on the curve's thread, normally the UI thread | Range scan; buffer ownership is moved immediately |
| [`setDataFNoRange(std::vector<float>&&, count)`][set-data-f-no-range] | Axis data ranges are fixed or maintained separately | Skips the range scan and signals |
| [`setDataFNoRange(const float*, count)`][set-data-f-no-range-copy] | Existing interleaved memory remains caller-owned and axis ranges are maintained separately | One reusable-buffer copy; skips the range scan and signals |
| [`postData(std::vector<float>&&, count)`][post-data] | A worker thread owns the completed buffer | Queues the same moved-buffer handoff, followed by the normal range scan |
| [`postData(std::vector<double>&&, count)`][post-data] | A worker thread owns a completed interleaved buffer that needs double precision, such as epoch timestamps | Queues the same moved-buffer handoff as `setData(std::vector<double>&&, count)`, followed by the normal range scan |

Start with [`setDataF(std::vector<float>&&, count)`][set-data-f-moved] for large
C++ data sets. Switch to [`setDataFNoRange()`][set-data-f-no-range] only when
the range scan is measurable and you maintain accurate
[`dataMin`][data-min]/[`dataMax`][data-max] values yourself.

Use a [`setData()`][set-data-points] overload for Unix-epoch timestamps, large
coordinates, or small differences at a large offset. These paths keep doubles
and rebase the GPU buffer so nearby values stay distinct. The `F` APIs store
floats; use them only when float precision is sufficient.

!!! caution "No-range updates change the contract"

    [`setDataFNoRange()`][set-data-f-no-range] does not report new extents to
    attached axes. If the data can leave the declared range, update the axes
    yourself or periodically use a range-calculating path. Otherwise rescaling
    and clipping behavior will be incorrect.

[`setDataFNoRangeWithCache()`][set-data-f-no-range-with-cache] also accepts a
prebuilt vertex cache, so vertex assembly can move off the UI and render
threads. It ties application code to renderer details; use it only when
measurements justify it. The cache must match the current point count and
renderer layout, otherwise it is rejected and normal vertex assembly runs.

## Build buffers efficiently

- For the `F` paths, store points as interleaved floats: `[x0, y0, x1, y1, ...]`.
- Reserve or resize the vector once before filling it.
- Move the completed vector instead of copying it.
- Publish no faster than the display presents frames.

For a fixed streaming window, set the axis data range once and use
[`setDataFNoRange()`][set-data-f-no-range] for each frame, as in the
[`realtime` example](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/realtime).

## Keep work off the UI thread

Generate data on a worker thread and hand off only completed buffers, using
[`postData()`][post-data] or a queued call. Never write to a `QQuickItem` from
the worker.

If producers are faster than the display, keep at most one pending batch or a
bounded queue. The
[`performance_showcase` worker](https://github.com/michalgrabarczyk/QAccelPlot/blob/main/examples/performance_showcase/src/DataGenerationWorker.cpp)
waits until the UI thread consumes the previous batch. See
[Background data production](cookbook/background-data.md).

## Optional rendering costs

### Hover interactions

`LineCurve` caches a bounding box for every 512 points. A pointer move checks
the boxes, then tests only segments or markers inside matching boxes:
`O(N / 512 + K)` for `K` candidates, `O(N)` in the worst case. The boxes are
built during data handoff, not on the first hover. Hover highlighting and
changes to solid-curve color or line width update shader uniforms without
rebuilding geometry; gradient effects may need per-vertex work.

[`RectangleList.hoveredIndex`][hovered-index] uses a spatial grid rebuilt on
data change, giving `O(1)` lookup per pointer move.

### Other rendering costs

- Markers add geometry and fragment work per visible point. Fragment work grows
  with `markerSize`; `LineCurve.Pixel` markers shade one pixel per point and
  suit very dense scatter plots.
- Gradient effects add material and domain work.
- Transitions update intermediate data while animating.
- Many series add per-node and per-material overhead at the same total point
  count.
- Larger windows and high-DPI displays add fragment work.

Changing line width, marker size, or color is cheaper than changing data, but
still schedules a redraw.

## Use a hardware backend

Curves require a hardware Qt Quick Scene Graph backend: OpenGL, OpenGL ES,
Direct3D, Vulkan, or Metal. The software backend skips curve rendering and
emits a warning. When a curve is blank or slow, check `QSG_RHI_BACKEND`,
`QT_QUICK_BACKEND`, remote-desktop sessions, and GPU drivers.

## Select the texture upload mode

Curve coordinates live in a GPU data texture. How live updates reach it depends
on `QACCELPLOT_USE_QT_PRIVATE_API`:

| Mode | Upload | Use when |
| --- | --- | --- |
| `ON` (default when `Qt6::QuickPrivate` is found) | `QSGPlainTexture::setImage()` replaces the texture contents in place | Large data sets updated every frame |
| `OFF` | `QQuickWindow::createTextureFromImage()` recreates the texture | Private Qt dependencies are unacceptable for packaging |

The private call is confined to one internal source file; no private Qt types
appear in public headers or ABI. Gradient textures always use the public path.

Benchmark both modes with your workload. The included live scenarios cover 1M,
2M, 5M, and 10M points:

```sh
cmake -S . -B build-private -DCMAKE_BUILD_TYPE=Release \
  -DQACCELPLOT_BUILD_BENCHMARKS=ON \
  -DQACCELPLOT_USE_QT_PRIVATE_API=ON
cmake -S . -B build-public -DCMAKE_BUILD_TYPE=Release \
  -DQACCELPLOT_BUILD_BENCHMARKS=ON \
  -DQACCELPLOT_USE_QT_PRIVATE_API=OFF
```

## Benchmarking tips

1. Build QAccelPlot and the application in Release mode.
2. Warm up shader compilation, allocations, and caches before recording.
3. Record presented frames ([`QQuickWindow::frameSwapped`][frame-swapped]), not timer callbacks.
4. For generated or streamed data, separately report how many new datasets are
   applied per second. Presented FPS can be higher because several frames may
   display the same dataset.
5. Report frame-time distribution as well as average FPS.
6. Record Qt version, OS, GPU, driver, graphics API, resolution, and device
   pixel ratio.
7. Separate static rendering, live updates, and data-ingestion measurements.
8. Repeat the test and publish the scenario with the result.

The Performance Showcase reports:

- **Display FPS** — frames presented to the screen.
- **Data Update Rate** (Hz) — new datasets applied to the plot per second,
  including worker-side generation. This determines how smooth the changing
  curve looks.
- **Peak update gap** — the longest interval between two dataset applications
  in the last reporting period.

### What the included benchmarks measure

- Data generation is excluded.
- Frame time runs from just before the data or viewport update until
  `QQuickRenderControl::endFrame()` returns. With offscreen frames this covers
  data copy, scene-graph sync, texture staging, rendering, command submission,
  and GPU completion.
- Frame intervals are recorded separately for sustained-FPS results.
- Warmup is time-based because offscreen runners are not capped at 60 Hz.
- Static scenarios pan the viewport over one fixed buffer.
- Multi-curve throughput counts points across all curves.
- Separate executables measure CPU ingestion and vertex-cache building without
  rendering.

Build the included benchmarks with:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DQACCELPLOT_BUILD_BENCHMARKS=ON \
  -DQACCELPLOT_BUILD_EXAMPLES=OFF \
  -DQACCELPLOT_BUILD_TESTS=OFF
cmake --build build --config Release
```

Scenarios are defined in
[`benchmarks/common/BenchmarkScenario.cpp`](https://github.com/michalgrabarczyk/QAccelPlot/blob/main/benchmarks/common/BenchmarkScenario.cpp).

[set-data-points]: api/classQAccelPlot_1_1LineCurve.md#function-setdata-12
[set-data-vectors]: api/classQAccelPlot_1_1LineCurve.md#function-setdata-22
[set-data-f-copy]: api/classQAccelPlot_1_1LineCurve.md#function-setdataf-12
[set-data-f-moved]: api/classQAccelPlot_1_1LineCurve.md#function-setdataf-22
[set-data-f-no-range]: api/classQAccelPlot_1_1LineCurve.md#function-setdatafnorange-12
[set-data-f-no-range-copy]: api/classQAccelPlot_1_1LineCurve.md#function-setdatafnorange-22
[set-data-f-no-range-with-cache]: api/classQAccelPlot_1_1LineCurve.md#function-setdatafnorangewithcache-12
[post-data]: api/classQAccelPlot_1_1LineCurve.md#function-postdata
[hovered-index]: api/classQAccelPlot_1_1RectangleList.md#property-hoveredindex-12
[data-min]: api/classQAccelPlot_1_1Axis.md#property-datamin-12
[data-max]: api/classQAccelPlot_1_1Axis.md#property-datamax-12
[frame-swapped]: https://doc.qt.io/qt-6/qquickwindow.html#frameSwapped
