








# Struct QAccelPlot::AxisTickValues



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**AxisTickValues**](structQAccelPlot_1_1AxisTickValues.md)



_The visible tick and subtick values for one axis viewport, without labels._ [More...](#detailed-description)

* `#include <AxisTicks.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**logScale**](#variable-logscale)   = `{false}`<br>_Whether the values were placed at powers of ten._  |
|  QList&lt; qreal &gt; | [**majorValues**](#variable-majorvalues)  <br>_Data-space values of the visible major ticks._  |
|  qreal | [**step**](#variable-step)   = `{0.0}`<br>_Major tick step on a linear scale; 0 on a logarithmic scale._  |
|  QList&lt; qreal &gt; | [**subtickValues**](#variable-subtickvalues)  <br>_Data-space values of the visible subticks._  |












































## Detailed Description


Built by `AxisTickPainter::computeTickValues()`. [**Grid**](classQAccelPlot_1_1Grid.md) lines and tick marks are both placed from it. 


    
## Public Attributes Documentation





### variable logScale {#variable-logscale}

_Whether the values were placed at powers of ten._ 
```C++
bool QAccelPlot::AxisTickValues::logScale;
```




<hr>




### variable majorValues {#variable-majorvalues}

_Data-space values of the visible major ticks._ 
```C++
QList<qreal> QAccelPlot::AxisTickValues::majorValues;
```




<hr>




### variable step {#variable-step}

_Major tick step on a linear scale; 0 on a logarithmic scale._ 
```C++
qreal QAccelPlot::AxisTickValues::step;
```




<hr>




### variable subtickValues {#variable-subtickvalues}

_Data-space values of the visible subticks._ 
```C++
QList<qreal> QAccelPlot::AxisTickValues::subtickValues;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/axis/AxisTicks.hpp`

