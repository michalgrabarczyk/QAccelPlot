---
description: "Produce QAccelPlot data on a background thread and safely hand off buffers to the UI thread using postData and one-batch patterns."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Background data production

Large or computationally expensive buffers should be produced away from the UI
thread. The producer owns its working memory; the UI receives only completed
frames.

## Simplest handoff

[`LineCurve::postData()`][post-data] is thread-safe. It moves the buffer into a
queued call that runs [`setDataF()`][set-data-f] on the curve's thread:

```cpp
void Producer::publish(QAccelPlot::LineCurve* curve)
{
    auto points = buildInterleavedPoints();
    const auto count = static_cast<int>(points.size() / 2);
    curve->postData(std::move(points), count);
}
```

## One-batch handoff

For high-rate updates, retain one completed batch. The worker prepares the next
batch off the UI thread, then waits to publish it until the previous batch has
been consumed. Once per display frame, the UI thread takes the completed batch
and moves it into the curve.

This applies backpressure and avoids an unbounded queue of frames that can no
longer be displayed on time.

```cpp
DataBatch batch;
if (worker.tryConsume(batch)) {
    curve->setDataFNoRange(std::move(batch.points), batch.pointCount);
}
```

Use the no-range form only when axes already contain valid data bounds. If the
range changes, publish the new bounds on the UI thread or use
[`postData()`][post-data] / [`setDataF()`][set-data-f] so QAccelPlot calculates
them.

The advanced performance showcase also prebuilds deterministic vertex data on
the worker. Treat that as an optimization to adopt only after profiling the
simpler handoff.

Complete sources:

- [`examples/performance_showcase`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/performance_showcase)
- [`benchmarks/common/BenchDataWorker.cpp`](https://github.com/michalgrabarczyk/QAccelPlot/blob/main/benchmarks/common/BenchDataWorker.cpp)

[post-data]: ../../api/classQAccelPlot_1_1LineCurve.html#a8b3d0effe4bd115f091ad505a7787b2c
[set-data-f]: ../../api/classQAccelPlot_1_1LineCurve.html#ac99cd1571b8153cae96791c8206d9de6
