








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
|  std::shared\_ptr&lt; [**DataTexture**](classQAccelPlot_1_1DataTexture.md) &gt; | [**dataTexture**](#variable-datatexture)  <br>_Texture that already holds_ `data` _, or_`nullptr` _to upload_`data` _._ |
|  QVector2D | [**domainMax**](#variable-domainmax)  <br>_Maximum data-space coordinate._  |
|  QVector2D | [**domainMin**](#variable-domainmin)  <br>_Minimum data-space coordinate._  |
|  const [**GradientColorPayload**](structQAccelPlot_1_1GradientColorPayload.md) & | [**gradientPayload**](#variable-gradientpayload)  <br>_Gradient color parameters, with value bounds in the coordinates of_ `data` _._ |
|  bool | [**hovered**](#variable-hovered)  <br>_Whether the curve is currently hovered._  |
|  bool | [**logScaleX**](#variable-logscalex)  <br>_Whether the X axis uses log scale._  |
|  bool | [**logScaleY**](#variable-logscaley)  <br>_Whether the Y axis uses log scale._  |
|  bool | [**markerFilled**](#variable-markerfilled)  <br>_Whether closed marker shapes are filled rather than outlined._  |
|  qreal | [**markerSize**](#variable-markersize)  <br>_Marker radius in pixels._  |
|  qreal | [**markerStrokeWidth**](#variable-markerstrokewidth)  <br>_Outline width in pixels for hollow markers._  |
|  int | [**pointCount**](#variable-pointcount)  <br>_Number of points in_ `data` _._ |
|  int | [**shapeType**](#variable-shapetype)  <br>_Marker shape index (_ `MarkerShape` _value minus one)._ |
|  const std::vector&lt; char &gt; \* | [**vertexCache**](#variable-vertexcache)  <br>_Pre-built vertex buffer, or_ `nullptr` _._ |
|  QVector2D | [**viewportSize**](#variable-viewportsize)  <br>_Viewport size in pixels._  |
|  QQuickWindow \* | [**window**](#variable-window)  <br>_Window providing the texture upload context._  |












































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




### variable dataTexture {#variable-datatexture}

_Texture that already holds_ `data` _, or_`nullptr` _to upload_`data` _._
```C++
std::shared_ptr<DataTexture> QAccelPlot::PointCurveRenderParams::dataTexture;
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

_Gradient color parameters, with value bounds in the coordinates of_ `data` _._
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




### variable markerFilled {#variable-markerfilled}

_Whether closed marker shapes are filled rather than outlined._ 
```C++
bool QAccelPlot::PointCurveRenderParams::markerFilled;
```




<hr>




### variable markerSize {#variable-markersize}

_Marker radius in pixels._ 
```C++
qreal QAccelPlot::PointCurveRenderParams::markerSize;
```




<hr>




### variable markerStrokeWidth {#variable-markerstrokewidth}

_Outline width in pixels for hollow markers._ 
```C++
qreal QAccelPlot::PointCurveRenderParams::markerStrokeWidth;
```




<hr>




### variable pointCount {#variable-pointcount}

_Number of points in_ `data` _._
```C++
int QAccelPlot::PointCurveRenderParams::pointCount;
```




<hr>




### variable shapeType {#variable-shapetype}

_Marker shape index (_ `MarkerShape` _value minus one)._
```C++
int QAccelPlot::PointCurveRenderParams::shapeType;
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




### variable window {#variable-window}

_Window providing the texture upload context._ 
```C++
QQuickWindow* QAccelPlot::PointCurveRenderParams::window;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/LineCurvePointRenderer.hpp`

