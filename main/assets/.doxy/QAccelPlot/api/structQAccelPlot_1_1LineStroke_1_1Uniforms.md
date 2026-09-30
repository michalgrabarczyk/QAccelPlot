








# Struct QAccelPlot::LineStroke::Uniforms



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**LineStroke**](namespaceQAccelPlot_1_1LineStroke.md) **>** [**Uniforms**](structQAccelPlot_1_1LineStroke_1_1Uniforms.md)



[_**Uniforms**_](structQAccelPlot_1_1LineStroke_1_1Uniforms.md) _of a line material that do not depend on its shader variant._

* `#include <LineStroke.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**antialiasingEnabled**](#variable-antialiasingenabled)  <br>_Whether GPU-side anti-aliasing is active._  |
|  qreal | [**antialiasingFeather**](#variable-antialiasingfeather)  <br>_Anti-aliasing feather width in pixels._  |
|  QColor | [**color**](#variable-color)  <br>_Line color._  |
|  [**DashParameters**](structQAccelPlot_1_1DashParameters.md) | [**dash**](#variable-dash)  <br>_Dash pattern; disabled for solid lines._  |
|  QVector2D | [**domainMax**](#variable-domainmax)  <br>_Maximum data-space coordinate, relative to the render origin._  |
|  QVector2D | [**domainMin**](#variable-domainmin)  <br>_Minimum data-space coordinate, relative to the render origin._  |
|  qreal | [**lineWidth**](#variable-linewidth)  <br>_Line width in pixels._  |
|  bool | [**logScaleX**](#variable-logscalex)  <br>_Whether the X axis uses log scale._  |
|  bool | [**logScaleY**](#variable-logscaley)  <br>_Whether the Y axis uses log scale._  |
|  int | [**pointCount**](#variable-pointcount)  <br>_Number of samples in the data texture._  |
|  QVector2D | [**viewportSize**](#variable-viewportsize)  <br>_Viewport size in pixels._  |












































## Public Attributes Documentation





### variable antialiasingEnabled {#variable-antialiasingenabled}

_Whether GPU-side anti-aliasing is active._ 
```C++
bool QAccelPlot::LineStroke::Uniforms::antialiasingEnabled;
```




<hr>




### variable antialiasingFeather {#variable-antialiasingfeather}

_Anti-aliasing feather width in pixels._ 
```C++
qreal QAccelPlot::LineStroke::Uniforms::antialiasingFeather;
```




<hr>




### variable color {#variable-color}

_Line color._ 
```C++
QColor QAccelPlot::LineStroke::Uniforms::color;
```




<hr>




### variable dash {#variable-dash}

_Dash pattern; disabled for solid lines._ 
```C++
DashParameters QAccelPlot::LineStroke::Uniforms::dash;
```




<hr>




### variable domainMax {#variable-domainmax}

_Maximum data-space coordinate, relative to the render origin._ 
```C++
QVector2D QAccelPlot::LineStroke::Uniforms::domainMax;
```




<hr>




### variable domainMin {#variable-domainmin}

_Minimum data-space coordinate, relative to the render origin._ 
```C++
QVector2D QAccelPlot::LineStroke::Uniforms::domainMin;
```




<hr>




### variable lineWidth {#variable-linewidth}

_Line width in pixels._ 
```C++
qreal QAccelPlot::LineStroke::Uniforms::lineWidth;
```




<hr>




### variable logScaleX {#variable-logscalex}

_Whether the X axis uses log scale._ 
```C++
bool QAccelPlot::LineStroke::Uniforms::logScaleX;
```




<hr>




### variable logScaleY {#variable-logscaley}

_Whether the Y axis uses log scale._ 
```C++
bool QAccelPlot::LineStroke::Uniforms::logScaleY;
```




<hr>




### variable pointCount {#variable-pointcount}

_Number of samples in the data texture._ 
```C++
int QAccelPlot::LineStroke::Uniforms::pointCount;
```




<hr>




### variable viewportSize {#variable-viewportsize}

_Viewport size in pixels._ 
```C++
QVector2D QAccelPlot::LineStroke::Uniforms::viewportSize;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/LineStroke.hpp`

