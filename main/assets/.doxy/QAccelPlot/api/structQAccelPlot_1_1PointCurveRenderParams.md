








# Struct QAccelPlot::PointCurveRenderParams



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**PointCurveRenderParams**](structQAccelPlot_1_1PointCurveRenderParams.md)



_Input parameters for_ [_**LineCurvePointRenderer::paint()**_](classQAccelPlot_1_1LineCurvePointRenderer.md#function-paint) _, assembled on the main thread._

* `#include <LineCurvePointRenderer.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**antialiasingEnabled**](#variable-antialiasingenabled)  <br>_Whether GPU-side anti-aliasing is active._  |
|  qreal | [**antialiasingFeather**](#variable-antialiasingfeather)  <br>_Anti-aliasing feather width in pixels._  |
|  QColor | [**color**](#variable-color)  <br>_Base marker color._  |
|  const std::vector&lt; float &gt; & | [**data**](#variable-data)  <br>_GPU-ready interleaved XY float data._  |
|  bool | [**dataChanged**](#variable-datachanged)  <br>_Whether data has changed since the last frame._  |
|  QVector2D | [**domainMax**](#variable-domainmax)  <br>_Maximum data-space coordinate._  |
|  QVector2D | [**domainMin**](#variable-domainmin)  <br>_Minimum data-space coordinate._  |
|  const [**GradientColorPayload**](structQAccelPlot_1_1GradientColorPayload.md) & | [**gradientPayload**](#variable-gradientpayload)  <br>_Gradient color parameters._  |
|  bool | [**hovered**](#variable-hovered)  <br>_Whether the curve is currently hovered._  |
|  bool | [**logScaleX**](#variable-logscalex)  <br>_Whether the X axis uses log scale._  |
|  bool | [**logScaleY**](#variable-logscaley)  <br>_Whether the Y axis uses log scale._  |
|  qreal | [**markerSize**](#variable-markersize)  <br>_Marker radius in pixels._  |
|  int | [**pointCount**](#variable-pointcount)  <br>_Number of points in_ `data` _._ |
|  int | [**shapeType**](#variable-shapetype)  <br>_Marker shape index (matches_ `LineCurve::PointShape` _)._ |
|  [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) | [**sourceData**](#variable-sourcedata)  <br>_Original float or double data used by CPU-side operations._  |
|  const std::vector&lt; char &gt; \* | [**vertexCache**](#variable-vertexcache)  <br>_Pre-built vertex buffer, or_ `nullptr` _._ |
|  QVector2D | [**viewportSize**](#variable-viewportsize)  <br>_Viewport size in pixels._  |












































## Public Attributes Documentation





### variable antialiasingEnabled {#variable-antialiasingenabled}

_Whether GPU-side anti-aliasing is active._ 
```C++
bool QAccelPlot::PointCurveRenderParams::antialiasingEnabled;
```




<hr>




### variable antialiasingFeather {#variable-antialiasingfeather}

_Anti-aliasing feather width in pixels._ 
```C++
qreal QAccelPlot::PointCurveRenderParams::antialiasingFeather;
```




<hr>




### variable color {#variable-color}

_Base marker color._ 
```C++
QColor QAccelPlot::PointCurveRenderParams::color;
```




<hr>




### variable data {#variable-data}

_GPU-ready interleaved XY float data._ 
```C++
const std::vector<float>& QAccelPlot::PointCurveRenderParams::data;
```




<hr>




### variable dataChanged {#variable-datachanged}

_Whether data has changed since the last frame._ 
```C++
bool QAccelPlot::PointCurveRenderParams::dataChanged;
```




<hr>




### variable domainMax {#variable-domainmax}

_Maximum data-space coordinate._ 
```C++
QVector2D QAccelPlot::PointCurveRenderParams::domainMax;
```




<hr>




### variable domainMin {#variable-domainmin}

_Minimum data-space coordinate._ 
```C++
QVector2D QAccelPlot::PointCurveRenderParams::domainMin;
```




<hr>




### variable gradientPayload {#variable-gradientpayload}

_Gradient color parameters._ 
```C++
const GradientColorPayload& QAccelPlot::PointCurveRenderParams::gradientPayload;
```




<hr>




### variable hovered {#variable-hovered}

_Whether the curve is currently hovered._ 
```C++
bool QAccelPlot::PointCurveRenderParams::hovered;
```




<hr>




### variable logScaleX {#variable-logscalex}

_Whether the X axis uses log scale._ 
```C++
bool QAccelPlot::PointCurveRenderParams::logScaleX;
```




<hr>




### variable logScaleY {#variable-logscaley}

_Whether the Y axis uses log scale._ 
```C++
bool QAccelPlot::PointCurveRenderParams::logScaleY;
```




<hr>




### variable markerSize {#variable-markersize}

_Marker radius in pixels._ 
```C++
qreal QAccelPlot::PointCurveRenderParams::markerSize;
```




<hr>




### variable pointCount {#variable-pointcount}

_Number of points in_ `data` _._
```C++
int QAccelPlot::PointCurveRenderParams::pointCount;
```




<hr>




### variable shapeType {#variable-shapetype}

_Marker shape index (matches_ `LineCurve::PointShape` _)._
```C++
int QAccelPlot::PointCurveRenderParams::shapeType;
```




<hr>




### variable sourceData {#variable-sourcedata}

_Original float or double data used by CPU-side operations._ 
```C++
CurveDataView QAccelPlot::PointCurveRenderParams::sourceData;
```




<hr>




### variable vertexCache {#variable-vertexcache}

_Pre-built vertex buffer, or_ `nullptr` _._
```C++
const std::vector<char>* QAccelPlot::PointCurveRenderParams::vertexCache;
```




<hr>




### variable viewportSize {#variable-viewportsize}

_Viewport size in pixels._ 
```C++
QVector2D QAccelPlot::PointCurveRenderParams::viewportSize;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/renderers/LineCurvePointRenderer.hpp`

