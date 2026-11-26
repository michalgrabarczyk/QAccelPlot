








# Struct QAccelPlot::TickSet



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**TickSet**](structQAccelPlot_1_1TickSet.md)



_The visible major ticks of one axis viewport, passed to_ `TickLabelFormatter::formatTicks()` _._

* `#include <TickLabelFormatter.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**logScale**](#variable-logscale)   = `{false}`<br>_Whether the values were placed at powers of ten._  |
|  qreal | [**step**](#variable-step)   = `{0.0}`<br>_Major tick step on a linear scale; 0 on a logarithmic scale._  |
|  QList&lt; qreal &gt; | [**values**](#variable-values)  <br>_Data-space values of the visible major ticks, in ascending order._  |
|  qreal | [**viewportMax**](#variable-viewportmax)   = `{0.0}`<br>_Upper bound of the visible range._  |
|  qreal | [**viewportMin**](#variable-viewportmin)   = `{0.0}`<br>_Lower bound of the visible range._  |












































## Public Attributes Documentation





### variable logScale {#variable-logscale}

_Whether the values were placed at powers of ten._ 
```C++
bool QAccelPlot::TickSet::logScale;
```




<hr>




### variable step {#variable-step}

_Major tick step on a linear scale; 0 on a logarithmic scale._ 
```C++
qreal QAccelPlot::TickSet::step;
```




<hr>




### variable values {#variable-values}

_Data-space values of the visible major ticks, in ascending order._ 
```C++
QList<qreal> QAccelPlot::TickSet::values;
```




<hr>




### variable viewportMax {#variable-viewportmax}

_Upper bound of the visible range._ 
```C++
qreal QAccelPlot::TickSet::viewportMax;
```




<hr>




### variable viewportMin {#variable-viewportmin}

_Lower bound of the visible range._ 
```C++
qreal QAccelPlot::TickSet::viewportMin;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/formatters/TickLabelFormatter.hpp`

