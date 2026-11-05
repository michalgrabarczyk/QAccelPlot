








# Struct QAccelPlot::Internal::AxisLabelLayoutParams



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**AxisLabelLayoutParams**](structQAccelPlot_1_1Internal_1_1AxisLabelLayoutParams.md)



_Inputs that place an axis label relative to the axis baseline._ 

* `#include <AxisLabelLayout.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  qreal | [**alongFraction**](#variable-alongfraction)   = `{0.5}`<br>_Position along the axis: 0 at the minimum-value end, 1 at the maximum-value end._  |
|  qreal | [**baseline**](#variable-baseline)   = `{0.0}`<br>_Baseline position across the axis: y for horizontal axes, x for vertical ones._  |
|  QRectF | [**dataRect**](#variable-datarect)  <br>[_**Axis**_](classQAccelPlot_1_1Axis.md) _span the label is aligned along, in item coordinates._ |
|  QPointF | [**offset**](#variable-offset)  <br>_Pixel offset applied after alignment and padding._  |
|  Qt::Edge | [**outward**](#variable-outward)   = `{Qt::BottomEdge}`<br>_Direction from the baseline toward the label._  |
|  qreal | [**padding**](#variable-padding)   = `{0.0}`<br>_Gap between the baseline and the near edge of the rotated text._  |
|  qreal | [**rotation**](#variable-rotation)   = `{0.0}`<br>_Clockwise text rotation in degrees._  |
|  QSizeF | [**textSize**](#variable-textsize)  <br>_Unrotated size of the label text._  |












































## Public Attributes Documentation





### variable alongFraction {#variable-alongfraction}

_Position along the axis: 0 at the minimum-value end, 1 at the maximum-value end._ 
```C++
qreal QAccelPlot::Internal::AxisLabelLayoutParams::alongFraction;
```




<hr>




### variable baseline {#variable-baseline}

_Baseline position across the axis: y for horizontal axes, x for vertical ones._ 
```C++
qreal QAccelPlot::Internal::AxisLabelLayoutParams::baseline;
```




<hr>




### variable dataRect {#variable-datarect}

[_**Axis**_](classQAccelPlot_1_1Axis.md) _span the label is aligned along, in item coordinates._
```C++
QRectF QAccelPlot::Internal::AxisLabelLayoutParams::dataRect;
```




<hr>




### variable offset {#variable-offset}

_Pixel offset applied after alignment and padding._ 
```C++
QPointF QAccelPlot::Internal::AxisLabelLayoutParams::offset;
```




<hr>




### variable outward {#variable-outward}

_Direction from the baseline toward the label._ 
```C++
Qt::Edge QAccelPlot::Internal::AxisLabelLayoutParams::outward;
```




<hr>




### variable padding {#variable-padding}

_Gap between the baseline and the near edge of the rotated text._ 
```C++
qreal QAccelPlot::Internal::AxisLabelLayoutParams::padding;
```




<hr>




### variable rotation {#variable-rotation}

_Clockwise text rotation in degrees._ 
```C++
qreal QAccelPlot::Internal::AxisLabelLayoutParams::rotation;
```




<hr>




### variable textSize {#variable-textsize}

_Unrotated size of the label text._ 
```C++
QSizeF QAccelPlot::Internal::AxisLabelLayoutParams::textSize;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/axis/internal/AxisLabelLayout.hpp`

