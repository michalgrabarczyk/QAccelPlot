---
description: "Draw histograms in QAccelPlot: count samples into equal or uneven bins with Histogram, show counts or density, and draw them with a BarSeries from QML or C++."
---

<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
-->

# Histograms

[`Histogram`][histogram] counts samples into bins and returns one bar per bin
for a [`BarSeries`](bar-charts.md).

## Count samples

```qml
QAccelPlot.BarSeries {
    xAxis: plot.xAxis
    yAxis: plot.yAxis

    Component.onCompleted: setData(QAccelPlot.Histogram.fromSamples(samples, 40).bars())
}
```

`fromSamples(samples, 40)` spreads 40 bins over the samples.
`fromSamples(samples, 40, -3, 3)` uses a fixed range instead.

## Choose the bin edges

Pass bin edges instead of a bin count:

```qml
const edges = QAccelPlot.Histogram.logEdges(1, 10000, 40);
setData(QAccelPlot.Histogram.fromSamples(samples, edges).bars());
```

| Edges | Result |
| --- | --- |
| `[0, 1, 2, 5, 10]` | Any list of finite, strictly increasing numbers |
| `Histogram.linearEdges(0, 10, 20)` | 20 equal bins from 0 to 10 |
| `Histogram.logEdges(1, 10000, 40)` | 40 bins of equal width on a logarithmic axis |
| `Histogram.decadeEdges(1, 10000)` | One bin between each pair of logarithmic axis ticks: 1, 2, …, 9, 10, 20, … |

The bars are [ranged](bar-charts.md#give-bars-their-own-extent): each one spans
its bin, whatever its width. `orientation: Qt.Horizontal` on the series puts
the bins on the Y axis.

## Density

`density()` divides each count by the sample count and the bin width, so the
bin areas sum to 1:

```qml
setData(QAccelPlot.Histogram.fromSamples(samples, edges).density().bars());
```

Use it when bins differ in width, or to compare data sets of different sizes.
A wide bin collects more samples than a narrow one, so raw counts of uneven
bins misrepresent the distribution.

On a logarithmic axis, `logEdges()` bins have equal drawn widths, so their raw
counts show the shape of the distribution. `decadeEdges()` bins widen tenfold
at each power of ten, so draw them as density.

## Counting rules

| Sample | Counted in |
| --- | --- |
| On an edge | The bin above it; the last bin includes its upper edge |
| NaN, or outside the edges | No bin |

Invalid edges or a bin count below 1 give an empty histogram (`binCount` 0)
and a logged warning. A histogram exposes `binCount`, `edges`, `values`,
`total`, `uniform`, and `binWidth`, which is NaN for uneven bins.

## C++

```cpp
#include <QAccelPlot/data/Histogram.hpp>

using QAccelPlot::Histogram;

const auto histogram = Histogram::fromSamples(samples.data(), samples.size(), 40);
bars->setRangedData(histogram.barData(), histogram.binCount());

const auto density = Histogram::fromSamples(samples.data(), samples.size(), Histogram::logEdges(1.0, 10000.0, 40)).density();
bars->setRangedData(density.barData(), density.binCount());
```

`samples` is an array of `double` or `float`. The functions keep no state, so
a worker thread can build the histogram and pass its data to
`postRangedData()`.
`Histogram::fromCounts(edges, counts)` wraps data that is already binned.

Complete source:

- [`examples/plot_types/histogram`](https://github.com/michalgrabarczyk/QAccelPlot/tree/main/examples/plot_types/histogram):
  equal and logarithmic bins, with a bin count slider and a switch to density
  bins on the axis ticks.

[histogram]: ../api/classQAccelPlot_1_1Histogram.md
