---
description: "Produce QAccelPlot data on a background thread and safely hand off buffers to the UI thread using postData and one-batch patterns."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Background data production

Produce large or expensive buffers off the UI thread and hand over only
completed buffers.

## Simplest handoff

[`LineCurve::postData()`][post-data] can be called from any thread. It moves
the buffer into a queued call that runs [`setDataF()`][set-data-f] on the
curve's thread:

```cpp
void Producer::publish(QAccelPlot::LineCurve* curve)
{
    auto points = buildInterleavedPoints();
    const auto count = static_cast<int>(points.size() / 2);
    curve->postData(std::move(points), count);
}
```

## One-batch handoff

For high-rate updates, keep one pending batch. The worker prepares the next
batch and waits until the UI thread has consumed the previous one. Once per
frame, the UI thread moves the pending batch into the curve. This bounds memory
and skips frames that could never be displayed.

```cpp
DataBatch batch;
if (worker.tryConsume(batch)) {
    curve->setDataFNoRange(std::move(batch.points), batch.pointCount);
}
```

Use the no-range form only when the axes already hold valid data bounds. If the
range changes, set the new bounds on the UI thread or use
[`postData()`][post-data] / [`setDataF()`][set-data-f] to calculate them.

The performance showcase also builds the vertex cache on the worker. Adopt that
only after profiling the simpler handoff.

Complete sources:

- [`examples/performance_showcase`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/performance_showcase)
- [`benchmarks/common/BenchDataWorker.cpp`](https://github.com/michalgrabarczyk/QAccelPlot/blob/main/benchmarks/common/BenchDataWorker.cpp)

[post-data]: ../api/classQAccelPlot_1_1LineCurve.md#function-postdata
[set-data-f]: ../api/classQAccelPlot_1_1LineCurve.md#function-setdataf-22
