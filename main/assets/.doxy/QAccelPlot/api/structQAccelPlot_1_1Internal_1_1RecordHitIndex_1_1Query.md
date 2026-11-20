








# Struct QAccelPlot::Internal::RecordHitIndex::Query



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**RecordHitIndex**](classQAccelPlot_1_1Internal_1_1RecordHitIndex.md) **>** [**Query**](structQAccelPlot_1_1Internal_1_1RecordHitIndex_1_1Query.md)



_A record lookup around a pixel position._ 

* `#include <RecordHitIndex.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**drawnCount**](#variable-drawncount)   = `{0}`<br>_Records drawn; those at or above it are not found._  |
|  qreal | [**overhang**](#variable-overhang)   = `{0.0}`<br>_Pixels a drawn record reaches past its span, such as half its minimum width._  |
|  QPointF | [**position**](#variable-position)  <br>_Item position to look around._  |
|  qreal | [**radius**](#variable-radius)   = `{0.0}`<br>_Largest pixel distance of the found record._  |












































## Public Attributes Documentation





### variable drawnCount {#variable-drawncount}

_Records drawn; those at or above it are not found._ 
```C++
int QAccelPlot::Internal::RecordHitIndex::Query::drawnCount;
```




<hr>




### variable overhang {#variable-overhang}

_Pixels a drawn record reaches past its span, such as half its minimum width._ 
```C++
qreal QAccelPlot::Internal::RecordHitIndex::Query::overhang;
```




<hr>




### variable position {#variable-position}

_Item position to look around._ 
```C++
QPointF QAccelPlot::Internal::RecordHitIndex::Query::position;
```




<hr>




### variable radius {#variable-radius}

_Largest pixel distance of the found record._ 
```C++
qreal QAccelPlot::Internal::RecordHitIndex::Query::radius;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/internal/RecordHitIndex.hpp`

