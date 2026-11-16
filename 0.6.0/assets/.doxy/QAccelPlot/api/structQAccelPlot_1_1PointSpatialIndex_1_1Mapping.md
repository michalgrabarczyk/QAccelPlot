








# Struct QAccelPlot::PointSpatialIndex::Mapping



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**PointSpatialIndex**](classQAccelPlot_1_1PointSpatialIndex.md) **>** [**Mapping**](structQAccelPlot_1_1PointSpatialIndex_1_1Mapping.md)



_Coordinate mapping applied before indexing._ 

* `#include <PointSpatialIndex.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**logX**](#variable-logx)   = `{false}`<br>_Index log10(x) instead of x._  |
|  bool | [**logY**](#variable-logy)   = `{false}`<br>_Index log10(y) instead of y._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**operator==**](#function-operator) (const [**Mapping**](structQAccelPlot_1_1PointSpatialIndex_1_1Mapping.md) & other) const<br>_Returns_ `true` _when both mappings are identical._ |




























## Public Attributes Documentation





### variable logX {#variable-logx}

_Index log10(x) instead of x._ 
```C++
bool QAccelPlot::PointSpatialIndex::Mapping::logX;
```




<hr>




### variable logY {#variable-logy}

_Index log10(y) instead of y._ 
```C++
bool QAccelPlot::PointSpatialIndex::Mapping::logY;
```




<hr>
## Public Functions Documentation





### function operator== {#function-operator}

_Returns_ `true` _when both mappings are identical._
```C++
bool QAccelPlot::PointSpatialIndex::Mapping::operator== (
    const Mapping & other
) const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/PointSpatialIndex.hpp`

