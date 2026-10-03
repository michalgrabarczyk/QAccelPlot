








# Class QAccelPlot::InspectionIndex



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**InspectionIndex**](classQAccelPlot_1_1InspectionIndex.md)



_Immutable k-d tree over the valid samples of a series, with their order along each axis._ 

* `#include <InspectionIndex.hpp>`

















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**Point**](structQAccelPlot_1_1InspectionIndex_1_1Point.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**build**](#function-build) (std::vector&lt; [**Point**](structQAccelPlot_1_1InspectionIndex_1_1Point.md) &gt; && points, bool logX, bool logY, const std::atomic\_bool & cancelled) <br>_Builds the tree from_ _points_ _; returns false when__cancelled_ _was set meanwhile._ |
|  void | [**collect**](#function-collect) (const [**InspectionBounds**](structQAccelPlot_1_1InspectionBounds.md) & bounds, int offset, int limit, QList&lt; int &gt; & indices) const<br> |
|  [**InspectionHit**](structQAccelPlot_1_1InspectionHit.md) | [**nearest**](#function-nearest) (const [**InspectionMetric**](structQAccelPlot_1_1InspectionMetric.md) & metric, const QPointF & position, double radius) const<br> |
|  [**InspectionHit**](structQAccelPlot_1_1InspectionHit.md) | [**nearestAlong**](#function-nearestalong) (const [**InspectionMetric**](structQAccelPlot_1_1InspectionMetric.md) & metric, InspectionAxis axis, double pixel, double radius) const<br>_Returns the sample nearest to_ _pixel_ _along__axis_ _._ |
|  [**InspectionNeighbors**](structQAccelPlot_1_1InspectionNeighbors.md) | [**neighbors**](#function-neighbors) (InspectionAxis axis, double value) const<br>_Returns the samples on either side of_ _value_ _on__axis_ _._ |
|  std::size\_t | [**storageBytes**](#function-storagebytes) () const<br> |
|  [**SummaryAccumulator**](structQAccelPlot_1_1SummaryAccumulator.md) | [**summarize**](#function-summarize) (const [**InspectionBounds**](structQAccelPlot_1_1InspectionBounds.md) & bounds) const<br> |




























## Public Functions Documentation





### function build {#function-build}

_Builds the tree from_ _points_ _; returns false when__cancelled_ _was set meanwhile._
```C++
bool QAccelPlot::InspectionIndex::build (
    std::vector< Point > && points,
    bool logX,
    bool logY,
    const std::atomic_bool & cancelled
) 
```




<hr>




### function collect {#function-collect}

```C++
void QAccelPlot::InspectionIndex::collect (
    const InspectionBounds & bounds,
    int offset,
    int limit,
    QList< int > & indices
) const
```




<hr>




### function nearest {#function-nearest}

```C++
InspectionHit QAccelPlot::InspectionIndex::nearest (
    const InspectionMetric & metric,
    const QPointF & position,
    double radius
) const
```




<hr>




### function nearestAlong {#function-nearestalong}

_Returns the sample nearest to_ _pixel_ _along__axis_ _._
```C++
InspectionHit QAccelPlot::InspectionIndex::nearestAlong (
    const InspectionMetric & metric,
    InspectionAxis axis,
    double pixel,
    double radius
) const
```




<hr>




### function neighbors {#function-neighbors}

_Returns the samples on either side of_ _value_ _on__axis_ _._
```C++
InspectionNeighbors QAccelPlot::InspectionIndex::neighbors (
    InspectionAxis axis,
    double value
) const
```




<hr>




### function storageBytes {#function-storagebytes}

```C++
std::size_t QAccelPlot::InspectionIndex::storageBytes () const
```




<hr>




### function summarize {#function-summarize}

```C++
SummaryAccumulator QAccelPlot::InspectionIndex::summarize (
    const InspectionBounds & bounds
) const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/internal/InspectionIndex.hpp`

