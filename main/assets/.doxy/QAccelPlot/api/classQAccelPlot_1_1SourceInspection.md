








# Class QAccelPlot::SourceInspection



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**SourceInspection**](classQAccelPlot_1_1SourceInspection.md)



_Inspection queries that read a series' own buffer whose records are ordered along one axis._ [More...](#detailed-description)

* `#include <SourceInspection.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**appended**](#function-appended) (int count) <br>_Updates cached statistics after one record was appended, giving_ _count_ _records._ |
|  void | [**collect**](#function-collect) (const [**InspectionSource**](structQAccelPlot_1_1InspectionSource.md) & source, InspectionAxis order, const [**InspectionBounds**](structQAccelPlot_1_1InspectionBounds.md) & bounds, int offset, int limit, QList&lt; int &gt; & indices) <br> |
|  [**InspectionHit**](structQAccelPlot_1_1InspectionHit.md) | [**nearest**](#function-nearest) (const [**InspectionSource**](structQAccelPlot_1_1InspectionSource.md) & source, const [**InspectionMetric**](structQAccelPlot_1_1InspectionMetric.md) & metric, InspectionAxis order, const QPointF & position, double radius) <br> |
|  [**InspectionHit**](structQAccelPlot_1_1InspectionHit.md) | [**nearestAlong**](#function-nearestalong) (const [**InspectionSource**](structQAccelPlot_1_1InspectionSource.md) & source, const [**InspectionMetric**](structQAccelPlot_1_1InspectionMetric.md) & metric, InspectionAxis order, double pixel, double radius) const<br>_Returns the valid sample nearest to_ _pixel_ _along the ordered axis._ |
|  [**InspectionNeighbors**](structQAccelPlot_1_1InspectionNeighbors.md) | [**neighbors**](#function-neighbors) (const [**InspectionSource**](structQAccelPlot_1_1InspectionSource.md) & source, InspectionAxis order, double value) const<br>_Returns the valid samples on either side of_ _value_ _on the ordered axis._ |
|  void | [**reset**](#function-reset) () <br>_Discards cached statistics after the records were replaced or their validity changed._  |
|  std::size\_t | [**storageBytes**](#function-storagebytes) () const<br>_Returns the bytes held by cached statistics._  |
|  [**SummaryAccumulator**](structQAccelPlot_1_1SummaryAccumulator.md) | [**summarize**](#function-summarize) (const [**InspectionSource**](structQAccelPlot_1_1InspectionSource.md) & source, InspectionAxis order, const [**InspectionBounds**](structQAccelPlot_1_1InspectionBounds.md) & bounds) <br> |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  bool | [**isSorted**](#function-issorted) (const [**InspectionSource**](structQAccelPlot_1_1InspectionSource.md) & source, InspectionAxis axis) <br>_Returns true when every coordinate of_ _source_ _along__axis_ _is finite and not smaller than its predecessor._ |


























## Detailed Description


The `order` argument names the axis whose coordinates are finite and non-decreasing. Positions along it are found by binary search, so no index is built and appended records are visible immediately. Per-block extents and Y statistics are cached lazily to answer large region summaries and to prune nearest-point searches. 


    
## Public Functions Documentation





### function appended {#function-appended}

_Updates cached statistics after one record was appended, giving_ _count_ _records._
```C++
void QAccelPlot::SourceInspection::appended (
    int count
) 
```




<hr>




### function collect {#function-collect}

```C++
void QAccelPlot::SourceInspection::collect (
    const InspectionSource & source,
    InspectionAxis order,
    const InspectionBounds & bounds,
    int offset,
    int limit,
    QList< int > & indices
) 
```




<hr>




### function nearest {#function-nearest}

```C++
InspectionHit QAccelPlot::SourceInspection::nearest (
    const InspectionSource & source,
    const InspectionMetric & metric,
    InspectionAxis order,
    const QPointF & position,
    double radius
) 
```




<hr>




### function nearestAlong {#function-nearestalong}

_Returns the valid sample nearest to_ _pixel_ _along the ordered axis._
```C++
InspectionHit QAccelPlot::SourceInspection::nearestAlong (
    const InspectionSource & source,
    const InspectionMetric & metric,
    InspectionAxis order,
    double pixel,
    double radius
) const
```




<hr>




### function neighbors {#function-neighbors}

_Returns the valid samples on either side of_ _value_ _on the ordered axis._
```C++
InspectionNeighbors QAccelPlot::SourceInspection::neighbors (
    const InspectionSource & source,
    InspectionAxis order,
    double value
) const
```




<hr>




### function reset {#function-reset}

_Discards cached statistics after the records were replaced or their validity changed._ 
```C++
void QAccelPlot::SourceInspection::reset () 
```




<hr>




### function storageBytes {#function-storagebytes}

_Returns the bytes held by cached statistics._ 
```C++
std::size_t QAccelPlot::SourceInspection::storageBytes () const
```




<hr>




### function summarize {#function-summarize}

```C++
SummaryAccumulator QAccelPlot::SourceInspection::summarize (
    const InspectionSource & source,
    InspectionAxis order,
    const InspectionBounds & bounds
) 
```




<hr>
## Public Static Functions Documentation





### function isSorted {#function-issorted}

_Returns true when every coordinate of_ _source_ _along__axis_ _is finite and not smaller than its predecessor._
```C++
static bool QAccelPlot::SourceInspection::isSorted (
    const InspectionSource & source,
    InspectionAxis axis
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/internal/SourceInspection.hpp`

