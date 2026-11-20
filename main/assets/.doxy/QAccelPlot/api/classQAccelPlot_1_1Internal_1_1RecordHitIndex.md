








# Class QAccelPlot::Internal::RecordHitIndex



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**RecordHitIndex**](classQAccelPlot_1_1Internal_1_1RecordHitIndex.md)



_Finds the drawn record nearest to a pixel position, for series that draw one record per data point._ [More...](#detailed-description)

* `#include <RecordHitIndex.hpp>`

















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**Query**](structQAccelPlot_1_1Internal_1_1RecordHitIndex_1_1Query.md) <br>_A record lookup around a pixel position._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef std::function&lt; std::optional&lt; qreal &gt;(int index)&gt; | [**Distance**](#typedef-distance)  <br>_Returns the pixel distance from the query position to record_ _index_ _, or nullopt when it is not drawn._ |
| typedef std::function&lt; std::optional&lt; std::pair&lt; double, double &gt; &gt;(int index)&gt; | [**SpanAt**](#typedef-spanat)  <br>_Returns record_ _index's_ _(min, max) span along the position axis, or nullopt when it has none._ |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**build**](#function-build) (int count, Qt::Orientation positionAxis, const [**SpanAt**](classQAccelPlot_1_1Internal_1_1RecordHitIndex.md#typedef-spanat) & spanAt) <br>_Rebuilds the spans of_ _count_ _records along__positionAxis_ _from__spanAt_ _._ |
|  void | [**invalidate**](#function-invalidate) () <br>_Marks the spans out of date._  |
|  int | [**nearest**](#function-nearest) (const [**Query**](structQAccelPlot_1_1Internal_1_1RecordHitIndex_1_1Query.md) & query, const [**Axis**](classQAccelPlot_1_1Axis.md) & xAxis, const [**Axis**](classQAccelPlot_1_1Axis.md) & yAxis, const QSizeF & size, const [**Distance**](classQAccelPlot_1_1Internal_1_1RecordHitIndex.md#typedef-distance) & distance) const<br>_Returns the record of_ _query_ _nearest to its position, or -1; on a tie, the one with the highest index._ |
|  bool | [**valid**](#function-valid) () const<br>_Returns_ `true` _while the spans are up to date._ |




























## Detailed Description


A spatial grid of the records' data spans along their position axis narrows the candidates, and a pixel distance test picks among them. The spans are rebuilt lazily after `invalidate()`. 


    
## Public Types Documentation





### typedef Distance {#typedef-distance}

_Returns the pixel distance from the query position to record_ _index_ _, or nullopt when it is not drawn._
```C++
using QAccelPlot::Internal::RecordHitIndex::Distance =  std::function<std::optional<qreal>(int index)>;
```




<hr>




### typedef SpanAt {#typedef-spanat}

_Returns record_ _index's_ _(min, max) span along the position axis, or nullopt when it has none._
```C++
using QAccelPlot::Internal::RecordHitIndex::SpanAt =  std::function<std::optional<std::pair<double, double>>(int index)>;
```




<hr>
## Public Functions Documentation





### function build {#function-build}

_Rebuilds the spans of_ _count_ _records along__positionAxis_ _from__spanAt_ _._
```C++
void QAccelPlot::Internal::RecordHitIndex::build (
    int count,
    Qt::Orientation positionAxis,
    const SpanAt & spanAt
) 
```



Only the position axis is indexed. Records such as candles often reach across much of the other axis, which would push them past the grid's per-rectangle cell limit into its linear scan; the pixel distance test covers that axis instead. 


        

<hr>




### function invalidate {#function-invalidate}

_Marks the spans out of date._ 
```C++
void QAccelPlot::Internal::RecordHitIndex::invalidate () 
```




<hr>




### function nearest {#function-nearest}

_Returns the record of_ _query_ _nearest to its position, or -1; on a tie, the one with the highest index._
```C++
int QAccelPlot::Internal::RecordHitIndex::nearest (
    const Query & query,
    const Axis & xAxis,
    const Axis & yAxis,
    const QSizeF & size,
    const Distance & distance
) const
```



_xAxis_ and _yAxis_ map the position of an item of _size_ to data coordinates. 


        

<hr>




### function valid {#function-valid}

_Returns_ `true` _while the spans are up to date._
```C++
bool QAccelPlot::Internal::RecordHitIndex::valid () const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/internal/RecordHitIndex.hpp`

