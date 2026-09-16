








# Struct QAccelPlot::AxisTickPainter::Params



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**AxisTickPainter**](classQAccelPlot_1_1AxisTickPainter.md) **>** [**Params**](structQAccelPlot_1_1AxisTickPainter_1_1Params.md)



_All style inputs required for a single paint call, bundled to reduce parameter count._ 

* `#include <AxisTickPainter.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**clampEdgeLabels**](#variable-clampedgelabels)   = `{false}`<br>_Whether labels at the extremes are clamped inward._  |
|  QColor | [**defaultSubtickColor**](#variable-defaultsubtickcolor)  <br>_Sub-tick color when not hovered._  |
|  QColor | [**hoverColor**](#variable-hovercolor)  <br>_Label/tick color when hovered._  |
|  bool | [**hovered**](#variable-hovered)   = `{false}`<br>_Whether the axis is currently hovered._  |
|  qreal | [**labelOverflow**](#variable-labeloverflow)   = `{0.0}`<br>_Extra space reserved for edge label overflow._  |
|  [**Axis::Orientation**](classQAccelPlot_1_1Axis.md#enum-orientation) | [**orientation**](#variable-orientation)   = `{Axis::Horizontal}`<br>[_**Axis**_](classQAccelPlot_1_1Axis.md) _orientation._ |
|  [**Axis::Side**](classQAccelPlot_1_1Axis.md#enum-side) | [**side**](#variable-side)   = `{Axis::Left}`<br>[_**Axis**_](classQAccelPlot_1_1Axis.md) _side._ |
|  const [**AxisTicker**](classQAccelPlot_1_1AxisTicker.md) \* | [**ticker**](#variable-ticker)   = `{nullptr}`<br>_Tick style configuration object._  |












































## Public Attributes Documentation





### variable clampEdgeLabels {#variable-clampedgelabels}

_Whether labels at the extremes are clamped inward._ 
```C++
bool QAccelPlot::AxisTickPainter::Params::clampEdgeLabels;
```




<hr>




### variable defaultSubtickColor {#variable-defaultsubtickcolor}

_Sub-tick color when not hovered._ 
```C++
QColor QAccelPlot::AxisTickPainter::Params::defaultSubtickColor;
```




<hr>




### variable hoverColor {#variable-hovercolor}

_Label/tick color when hovered._ 
```C++
QColor QAccelPlot::AxisTickPainter::Params::hoverColor;
```




<hr>




### variable hovered {#variable-hovered}

_Whether the axis is currently hovered._ 
```C++
bool QAccelPlot::AxisTickPainter::Params::hovered;
```




<hr>




### variable labelOverflow {#variable-labeloverflow}

_Extra space reserved for edge label overflow._ 
```C++
qreal QAccelPlot::AxisTickPainter::Params::labelOverflow;
```




<hr>




### variable orientation {#variable-orientation}

[_**Axis**_](classQAccelPlot_1_1Axis.md) _orientation._
```C++
Axis::Orientation QAccelPlot::AxisTickPainter::Params::orientation;
```




<hr>




### variable side {#variable-side}

[_**Axis**_](classQAccelPlot_1_1Axis.md) _side._
```C++
Axis::Side QAccelPlot::AxisTickPainter::Params::side;
```




<hr>




### variable ticker {#variable-ticker}

_Tick style configuration object._ 
```C++
const AxisTicker* QAccelPlot::AxisTickPainter::Params::ticker;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/axis/AxisTickPainter.hpp`

