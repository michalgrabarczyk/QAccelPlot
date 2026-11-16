








# Namespace QAccelPlot::Internal::InspectionScan



[**Namespace List**](namespaces.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**InspectionScan**](namespaceQAccelPlot_1_1Internal_1_1InspectionScan.md)



_Inspection queries that scan every record; used for small series that are not ordered along the queried axis._ 








































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**collect**](#function-collect) (const [**InspectionSource**](structQAccelPlot_1_1Internal_1_1InspectionSource.md) & source, const [**InspectionBounds**](structQAccelPlot_1_1Internal_1_1InspectionBounds.md) & bounds, int offset, int limit, QList&lt; int &gt; & indices) <br> |
|  [**InspectionHit**](structQAccelPlot_1_1Internal_1_1InspectionHit.md) | [**nearest**](#function-nearest) (const [**InspectionSource**](structQAccelPlot_1_1Internal_1_1InspectionSource.md) & source, const [**InspectionMetric**](structQAccelPlot_1_1Internal_1_1InspectionMetric.md) & metric, const QPointF & position, double radius) <br> |
|  [**InspectionHit**](structQAccelPlot_1_1Internal_1_1InspectionHit.md) | [**nearestAlong**](#function-nearestalong) (const [**InspectionSource**](structQAccelPlot_1_1Internal_1_1InspectionSource.md) & source, const [**InspectionMetric**](structQAccelPlot_1_1Internal_1_1InspectionMetric.md) & metric, InspectionAxis axis, double pixel, double radius) <br>_Returns the valid data point nearest to_ _pixel_ _along__axis_ _._ |
|  [**InspectionNeighbors**](structQAccelPlot_1_1Internal_1_1InspectionNeighbors.md) | [**neighbors**](#function-neighbors) (const [**InspectionSource**](structQAccelPlot_1_1Internal_1_1InspectionSource.md) & source, InspectionAxis axis, double value) <br>_Returns the valid data points on either side of_ _value_ _on__axis_ _._ |
|  [**SummaryAccumulator**](structQAccelPlot_1_1Internal_1_1SummaryAccumulator.md) | [**summarize**](#function-summarize) (const [**InspectionSource**](structQAccelPlot_1_1Internal_1_1InspectionSource.md) & source, const [**InspectionBounds**](structQAccelPlot_1_1Internal_1_1InspectionBounds.md) & bounds) <br> |




























## Public Functions Documentation





### function collect {#function-collect}

```C++
void QAccelPlot::Internal::InspectionScan::collect (
    const InspectionSource & source,
    const InspectionBounds & bounds,
    int offset,
    int limit,
    QList< int > & indices
) 
```




<hr>




### function nearest {#function-nearest}

```C++
InspectionHit QAccelPlot::Internal::InspectionScan::nearest (
    const InspectionSource & source,
    const InspectionMetric & metric,
    const QPointF & position,
    double radius
) 
```




<hr>




### function nearestAlong {#function-nearestalong}

_Returns the valid data point nearest to_ _pixel_ _along__axis_ _._
```C++
InspectionHit QAccelPlot::Internal::InspectionScan::nearestAlong (
    const InspectionSource & source,
    const InspectionMetric & metric,
    InspectionAxis axis,
    double pixel,
    double radius
) 
```




<hr>




### function neighbors {#function-neighbors}

_Returns the valid data points on either side of_ _value_ _on__axis_ _._
```C++
InspectionNeighbors QAccelPlot::Internal::InspectionScan::neighbors (
    const InspectionSource & source,
    InspectionAxis axis,
    double value
) 
```




<hr>




### function summarize {#function-summarize}

```C++
SummaryAccumulator QAccelPlot::Internal::InspectionScan::summarize (
    const InspectionSource & source,
    const InspectionBounds & bounds
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/internal/SourceInspection.hpp`

