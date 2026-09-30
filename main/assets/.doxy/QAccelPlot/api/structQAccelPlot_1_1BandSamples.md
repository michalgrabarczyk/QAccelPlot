








# Struct QAccelPlot::BandSamples



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**BandSamples**](structQAccelPlot_1_1BandSamples.md)



_Read-only view over interleaved_ `(x, low, high)` _band samples in float or double precision._

* `#include <BandEdgeRenderer.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**count**](#variable-count)   = `{0}`<br>_Number of samples._  |
|  const double \* | [**doubleData**](#variable-doubledata)   = `{nullptr}`<br>_Double samples, or_ `nullptr` _when_`floatData` _is set._ |
|  const float \* | [**floatData**](#variable-floatdata)   = `{nullptr}`<br>_Float samples, or_ `nullptr` _when_`doubleData` _is set._ |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  qreal | [**value**](#function-value) (const int index, const int component) const<br>_Returns value_ _component_ _(0 = x, 1 = low, 2 = high) of sample__index_ _without narrowing double data._ |




























## Public Attributes Documentation





### variable count {#variable-count}

_Number of samples._ 
```C++
int QAccelPlot::BandSamples::count;
```




<hr>




### variable doubleData {#variable-doubledata}

_Double samples, or_ `nullptr` _when_`floatData` _is set._
```C++
const double* QAccelPlot::BandSamples::doubleData;
```




<hr>




### variable floatData {#variable-floatdata}

_Float samples, or_ `nullptr` _when_`doubleData` _is set._
```C++
const float* QAccelPlot::BandSamples::floatData;
```




<hr>
## Public Functions Documentation





### function value {#function-value}

_Returns value_ _component_ _(0 = x, 1 = low, 2 = high) of sample__index_ _without narrowing double data._
```C++
inline qreal QAccelPlot::BandSamples::value (
    const int index,
    const int component
) const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/BandEdgeRenderer.hpp`

