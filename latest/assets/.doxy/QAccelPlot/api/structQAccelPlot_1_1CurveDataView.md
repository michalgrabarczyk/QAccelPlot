








# Struct QAccelPlot::CurveDataView



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md)



_Read-only view over either interleaved float or double curve coordinates._ 

* `#include <CurveRendererParams.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  const double \* | [**doubleData**](#variable-doubledata)   = `{nullptr}`<br> |
|  const float \* | [**floatData**](#variable-floatdata)   = `{nullptr}`<br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  qreal | [**x**](#function-x) (int index) const<br>_Returns the X coordinate at_ _index_ _without narrowing double data._ |
|  qreal | [**y**](#function-y) (int index) const<br>_Returns the Y coordinate at_ _index_ _without narrowing double data._ |




























## Public Attributes Documentation





### variable doubleData {#variable-doubledata}

```C++
const double* QAccelPlot::CurveDataView::doubleData;
```




<hr>




### variable floatData {#variable-floatdata}

```C++
const float* QAccelPlot::CurveDataView::floatData;
```




<hr>
## Public Functions Documentation





### function x {#function-x}

_Returns the X coordinate at_ _index_ _without narrowing double data._
```C++
inline qreal QAccelPlot::CurveDataView::x (
    int index
) const
```




<hr>




### function y {#function-y}

_Returns the Y coordinate at_ _index_ _without narrowing double data._
```C++
inline qreal QAccelPlot::CurveDataView::y (
    int index
) const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/renderers/CurveRendererParams.hpp`

