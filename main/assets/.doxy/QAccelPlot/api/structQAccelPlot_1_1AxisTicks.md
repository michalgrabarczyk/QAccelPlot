








# Struct QAccelPlot::AxisTicks



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**AxisTicks**](structQAccelPlot_1_1AxisTicks.md)



_The visible tick and subtick values, with formatted labels, for one axis viewport._ [More...](#detailed-description)

* `#include <AxisTicks.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  QList&lt; [**AxisTick**](structQAccelPlot_1_1AxisTick.md) &gt; | [**majorTicks**](#variable-majorticks)  <br>_Visible major ticks with their labels._  |
|  QList&lt; qreal &gt; | [**subtickValues**](#variable-subtickvalues)  <br>_Data-space values of the visible subticks._  |












































## Detailed Description


Holds data-space values and label text only; no pixel geometry. Placement (padding, font metrics, rotation, value-to-pixel mapping) is resolved in `Axis::paint()`.


Built on the GUI thread by `AxisTickPainter::computeTicks()`, because label formatting may call into the QML/JS engine, and consumed by `Axis::paint()`, which runs on the render thread with the threaded render loop. 


    
## Public Attributes Documentation





### variable majorTicks {#variable-majorticks}

_Visible major ticks with their labels._ 
```C++
QList<AxisTick> QAccelPlot::AxisTicks::majorTicks;
```




<hr>




### variable subtickValues {#variable-subtickvalues}

_Data-space values of the visible subticks._ 
```C++
QList<qreal> QAccelPlot::AxisTicks::subtickValues;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/axis/AxisTicks.hpp`

