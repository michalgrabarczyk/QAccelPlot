








# Struct QAccelPlot::DataPointStore::Layout



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**DataPointStore**](classQAccelPlot_1_1DataPointStore.md) **>** [**Layout**](structQAccelPlot_1_1DataPointStore_1_1Layout.md)



_Shape of a data point._ 

* `#include <DataPointStore.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**attributeCount**](#variable-attributecount)   = `{0}`<br>_Float attributes per data point; interleaved after the coordinates in render data._  |
|  std::vector&lt; [**Dimension**](classQAccelPlot_1_1DataPointStore.md#enum-dimension) &gt; | [**columns**](#variable-columns)   = `{Dimension::X, Dimension::Y}`<br>_Coordinate columns in data point order._  |
|  [**RangeRule**](classQAccelPlot_1_1DataPointStore.md#enum-rangerule) | [**rangeRule**](#variable-rangerule)   = `{RangeRule::EachCoordinate}`<br>_Rule for the extents._  |












































## Public Attributes Documentation





### variable attributeCount {#variable-attributecount}

_Float attributes per data point; interleaved after the coordinates in render data._ 
```C++
int QAccelPlot::DataPointStore::Layout::attributeCount;
```




<hr>




### variable columns {#variable-columns}

_Coordinate columns in data point order._ 
```C++
std::vector<Dimension> QAccelPlot::DataPointStore::Layout::columns;
```




<hr>




### variable rangeRule {#variable-rangerule}

_Rule for the extents._ 
```C++
RangeRule QAccelPlot::DataPointStore::Layout::rangeRule;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/DataPointStore.hpp`

