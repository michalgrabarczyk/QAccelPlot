---
description: "QAccelPlot performance guide — data-ingestion API comparison, efficient buffer strategies, worker-thread handoff, hover cost analysis, and benchmarking methodology."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Performance

Performance is a core design goal of QAccelPlot. Its GPU-rendered series are
designed for large data sets, including plots that use visual effects.

Actual throughput depends on more than point count. Update frequency, data
conversion, range scans, series count, line style, effects, window size,
graphics backend, and display refresh rate all contribute. Measure the workload
that resembles your application.

## Select the data path

| API | Use it when | Main cost |
| --- | --- | --- |
| [`setData(QList<QPointF>)`][set-data-points] | Data originates in QML or convenience matters most | Allocation plus conversion to interleaved floats |
| [`setData(xs, ys)`][set-data-vectors] | Existing C++ data is stored as separate double vectors | Allocation plus double-to-float interleaving |
| [`setDataF(const float*, count)`][set-data-f-copy] | Existing interleaved memory cannot be transferred | One copy plus a range scan |
| [`setDataF(std::vector<float>&&, count)`][set-data-f-moved] | The caller is on the curve's thread, normally the UI thread | Range scan; buffer ownership is moved immediately |
| [`setDataFNoRange(std::vector<float>&&, count)`][set-data-f-no-range] | Axis data ranges are fixed or maintained separately | Skips the range scan and signals |
| [`setDataFNoRange(const float*, count)`][set-data-f-no-range-copy] | Existing interleaved memory remains caller-owned and axis ranges are maintained separately | One reusable-buffer copy; skips the range scan and signals |
| [`postData(std::vector<float>&&, count)`][post-data] | A worker thread owns the completed buffer | Queues the same moved-buffer handoff, followed by the normal range scan |

[`setDataF(std::vector<float>&&, count)`][set-data-f-moved] transfers the
buffer immediately and must be called on the curve's thread, normally the UI
thread. [`postData(std::vector<float>&&, count)`][post-data] is its
worker-thread-safe counterpart: it queues that same transfer to the curve's
thread.

Start with [`setDataF(std::vector<float>&&, count)`][set-data-f-moved] for large
C++ data sets. Move to [`setDataFNoRange()`][set-data-f-no-range] only after
confirming that range calculation matters and after arranging accurate
[`dataMin`][data-min]/[`dataMax`][data-max] values yourself.

!!! caution "No-range updates change the contract"

    [`setDataFNoRange()`][set-data-f-no-range] does not report new extents to
    attached axes. If the data can leave the declared range, update the axes
    yourself or periodically use a range-calculating path. Otherwise rescaling
    and clipping behavior will be incorrect.

[`setDataFNoRangeWithCache()`][set-data-f-no-range-with-cache] is an advanced
path used by the performance showcase and benchmark harness. It is valuable
when an application can prepare the renderer's vertex cache off the UI/render
path, but it couples application code more closely to renderer details. Prefer
the simpler moved-buffer APIs until measurements justify that complexity. A
supplied cache must exactly match the current point count and renderer layout;
invalid caches are rejected and rendering falls back to normal vertex assembly.

## Build buffers efficiently

- Store points as interleaved floats: `[x0, y0, x1, y1, ...]`.
- Reserve or resize the vector once before filling it.
- Move the completed vector instead of copying it.
- Reuse producer-side buffers after ownership returns through your handoff
  scheme.
- Publish at the rate the renderer can consume. Generating many intermediate
  frames that will never be displayed wastes CPU time.

For worker-thread production and one-batch handoff, see
[Background data production](cookbook/background-data.md).

For a stable streaming window, set the axis data range explicitly and use
[`setDataFNoRange()`][set-data-f-no-range] for subsequent frames. The
[`realtime` example](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/realtime)
demonstrates this pattern.

## Keep work off critical paths

Generate or transform expensive data on a worker thread. Transfer only a
completed buffer to the UI thread; never write directly to a `QQuickItem` from
the worker. [`postData()`][post-data] is the simplest safe handoff when
automatic range calculation is acceptable.

If producers are faster than display, keep at most one completed batch, or use
a queue with a fixed limit. This applies backpressure, preventing memory growth
and avoiding work on frames that will never be displayed. The
[`performance_showcase` worker](https://github.com/michalgrabarczyk/QAccelPlot/blob/main/examples/performance_showcase/src/DataGenerationWorker.cpp)
uses the one-batch approach and waits until the UI thread consumes the previous
batch.

## Optional rendering costs

### Hover interactions

`LineCurve` hover tests use cached bounding boxes for blocks of 512 points.
Each pointer move checks those boxes, then tests only the surviving segments or
markers: `O(N / 512 + K)`, where `K` is the number of candidates. The worst
case remains `O(N)` when many blocks overlap the pointer. For normal data
updates, the bounds are built during data handoff rather than on the first
hover. Built-in hover feedback and changes to solid-curve color or line width
update material uniforms without rebuilding vertex geometry; gradient effects
may require additional per-vertex work.

[`QAccelPlot.RectangleList.hoveredIndex`][hovered-index] uses a spatial grid, giving an `O(1)`
lookup per pointer move after the grid is built when its data changes.

### Other rendering costs

- Point markers add geometry and fragment work for every visible point.
- Gradient effects require additional material and domain work.
- Transitions update intermediate data while animation is active.
- Many separate series add per-node and per-material overhead even when their
  total point count is unchanged.
- Larger windows and high-DPI displays increase fragment workload.

Changing uniform-like properties such as line width, marker size, or color is
usually cheaper than changing the data or line topology, but it still schedules
a redraw.

## Use a hardware backend

QAccelPlot's custom curve rendering requires a hardware Qt Quick Scene Graph
backend. OpenGL, Direct3D, Vulkan, and Metal are supported through Qt's RHI
shaders; the software backend skips custom curve rendering and emits a warning.

Check `QSG_RHI_BACKEND`, `QT_QUICK_BACKEND`, remote-desktop behavior, and driver
availability when a curve is blank or unexpectedly slow.

## Select the texture upload mode

QAccelPlot keeps curve coordinates in a compact GPU data texture. By default,
when `Qt6::QuickPrivate` is available, live updates use
`QSGPlainTexture::setImage()` to replace the image contents in place. This
narrowly scoped private call avoids recreating the scene-graph texture every
frame and provides a material performance advantage for very large,
continuously updated data sets. Private Qt types stay confined to one internal
implementation file and are not exposed through public headers or ABI.

Configure with `-DQACCELPLOT_USE_QT_PRIVATE_API=OFF` when private dependencies
are unacceptable for packaging or deployment. The public-only path retains the
same compact data-texture shaders and uses
`QQuickWindow::createTextureFromImage()` to recreate the live data texture.
Gradient textures always use this public recreation path because gradient stop
changes are infrequent.

Benchmark both modes against the application's real workload. The included
live scenarios cover 1M, 2M, 5M, and 10M points and can be built separately:

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

The Performance Showcase displays both **Display FPS** and **Data
Update Rate**. Display FPS counts frames presented to the screen. Data Update
Rate is expressed in hertz and counts newly generated curve datasets applied to
the plot each second; this is the cadence that determines how fluid the changing
curve appears. The showcase generates its sine data on a background worker using
a lookup table, so this rate includes data preparation and application. The
worker keeps at most one completed batch pending and waits for its consumption
before generating the next batch.

The Data Update Rate card also reports the **peak update gap**: the longest
wall-clock interval between consecutive dataset applications during the last
reporting period.

### What the included benchmarks measure

Data generation is excluded from the benchmarks.

Graphical and headless frame timing starts immediately before the curve data or
viewport is updated and ends after `QQuickRenderControl::endFrame()` returns.
Because the runner uses offscreen frames, this includes data copying,
scene-graph synchronization, texture staging, rendering, command submission,
and GPU completion. Frame intervals are recorded separately for sustained-FPS
calculations.

Warmup is wall-clock based rather than assuming a 60 Hz presentation rate. This
matters because the offscreen runners are uncapped and can process many more than
60 frames per second.

Static scenarios keep one source buffer and perform the same bounded viewport pan
in the graphical and headless runners. Multi-curve throughput counts all points
across all curves. The separate CPU ingestion and vertex-cache executables isolate
those operations without rendering.

Build the included benchmarks with:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DQACCELPLOT_BUILD_BENCHMARKS=ON \
  -DQACCELPLOT_BUILD_EXAMPLES=OFF \
  -DQACCELPLOT_BUILD_TESTS=OFF
cmake --build build --config Release
```

The benchmark scenarios live in
[`benchmarks/common/BenchmarkScenario.cpp`](https://github.com/michalgrabarczyk/QAccelPlot/blob/main/benchmarks/common/BenchmarkScenario.cpp).
Treat their results as measurements of the recorded machine and scenario, not
as a guarantee for every application.

[set-data-points]: ../api/classQAccelPlot_1_1LineCurve.html#acf9463cc9f4c0e0eb7cc9d053b7e7114
[set-data-vectors]: ../api/classQAccelPlot_1_1LineCurve.html#aad067a9b84289a5c73637905cbc61a69
[set-data-f-copy]: ../api/classQAccelPlot_1_1LineCurve.html#ad79086eb28ddc5112e534c8cdbfa03ef
[set-data-f-moved]: ../api/classQAccelPlot_1_1LineCurve.html#ac99cd1571b8153cae96791c8206d9de6
[set-data-f-no-range]: ../api/classQAccelPlot_1_1LineCurve.html#a80e4d6c6ba351c76333ef4177f3c6e30
[set-data-f-no-range-copy]: ../api/classQAccelPlot_1_1LineCurve.html#a0cb6f8ea1a797ee69886a4508266d7db
[set-data-f-no-range-with-cache]: ../api/classQAccelPlot_1_1LineCurve.html#a549c74098a67f68d77ea45e9a0e32649
[post-data]: ../api/classQAccelPlot_1_1LineCurve.html#a8b3d0effe4bd115f091ad505a7787b2c
[hovered-index]: ../api/classQAccelPlot_1_1RectangleList.html#afb85815f91d34b3a35bbf957e9aa5e09
[data-min]: ../api/classQAccelPlot_1_1Axis.html#a7ff8bbf8f594cce69d3a56c9b7f7dc25
[data-max]: ../api/classQAccelPlot_1_1Axis.html#ab9a1549ca7fa37509039903052111c85
[frame-swapped]: https://doc.qt.io/qt-6/qquickwindow.html#frameSwapped
