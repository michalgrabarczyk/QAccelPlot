








# Struct QAccelPlot::LineCurveRenderParams



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**LineCurveRenderParams**](structQAccelPlot_1_1LineCurveRenderParams.md)



_Input parameters for_ [_**LineCurveLineRenderer::paint()**_](classQAccelPlot_1_1LineCurveLineRenderer.md#function-paint) _, assembled on the main thread._

* `#include <LineCurveLineRenderer.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**antialiasingEnabled**](#variable-antialiasingenabled)  <br>_Whether GPU-side anti-aliasing is active._  |
|  qreal | [**antialiasingFeather**](#variable-antialiasingfeather)  <br>_Anti-aliasing feather width in pixels._  |
|  QColor | [**color**](#variable-color)  <br>_Base line color._  |
|  const std::vector&lt; float &gt; & | [**data**](#variable-data)  <br>_GPU-ready interleaved XY float data._  |
|  bool | [**dataChanged**](#variable-datachanged)  <br>_Whether the data buffer has changed since the last frame._  |
|  QVector2D | [**domainMax**](#variable-domainmax)  <br>_Maximum data-space coordinate._  |
|  QVector2D | [**domainMin**](#variable-domainmin)  <br>_Minimum data-space coordinate._  |
|  const [**GradientFillPayload**](structQAccelPlot_1_1GradientFillPayload.md) & | [**gradientFillPayload**](#variable-gradientfillpayload)  <br>_Gradient fill parameters._  |
|  const [**GradientColorPayload**](structQAccelPlot_1_1GradientColorPayload.md) & | [**gradientPayload**](#variable-gradientpayload)  <br>_Gradient stroke parameters._  |
|  bool | [**hovered**](#variable-hovered)  <br>_Whether the curve is currently hovered._  |
|  const [**LineStyle**](classQAccelPlot_1_1LineStyle.md) \* | [**lineStyle**](#variable-linestyle)  <br>_Active line style (dash pattern etc.)._  |
|  qreal | [**lineWidth**](#variable-linewidth)  <br>_Line width in pixels._  |
|  bool | [**logScaleX**](#variable-logscalex)  <br>_Whether the X axis uses log scale._  |
|  bool | [**logScaleY**](#variable-logscaley)  <br>_Whether the Y axis uses log scale._  |
|  int | [**pointCount**](#variable-pointcount)  <br>_Number of points in_ `data` _._ |
|  [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) | [**sourceData**](#variable-sourcedata)  <br>_Original float or double data used by CPU-side operations._  |
|  const std::vector&lt; char &gt; \* | [**vertexCache**](#variable-vertexcache)  <br>_Pre-built vertex buffer, or_ `nullptr` _to build on the render thread._ |
|  QVector2D | [**viewportSize**](#variable-viewportsize)  <br>_Viewport size in pixels._  |
|  QQuickWindow \* | [**window**](#variable-window)  <br>_Window providing the texture upload context._  |
|  [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**xAxis**](#variable-xaxis)  <br>_Horizontal axis._  |
|  [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**yAxis**](#variable-yaxis)  <br>_Vertical axis._  |












































## Public Attributes Documentation





### variable antialiasingEnabled {#variable-antialiasingenabled}

_Whether GPU-side anti-aliasing is active._ 
```C++
bool QAccelPlot::LineCurveRenderParams::antialiasingEnabled;
```




<hr>




### variable antialiasingFeather {#variable-antialiasingfeather}

_Anti-aliasing feather width in pixels._ 
```C++
qreal QAccelPlot::LineCurveRenderParams::antialiasingFeather;
```




<hr>




### variable color {#variable-color}

_Base line color._ 
```C++
QColor QAccelPlot::LineCurveRenderParams::color;
```




<hr>




### variable data {#variable-data}

_GPU-ready interleaved XY float data._ 
```C++
const std::vector<float>& QAccelPlot::LineCurveRenderParams::data;
```




<hr>




### variable dataChanged {#variable-datachanged}

_Whether the data buffer has changed since the last frame._ 
```C++
bool QAccelPlot::LineCurveRenderParams::dataChanged;
```




<hr>




### variable domainMax {#variable-domainmax}

_Maximum data-space coordinate._ 
```C++
QVector2D QAccelPlot::LineCurveRenderParams::domainMax;
```




<hr>




### variable domainMin {#variable-domainmin}

_Minimum data-space coordinate._ 
```C++
QVector2D QAccelPlot::LineCurveRenderParams::domainMin;
```




<hr>




### variable gradientFillPayload {#variable-gradientfillpayload}

_Gradient fill parameters._ 
```C++
const GradientFillPayload& QAccelPlot::LineCurveRenderParams::gradientFillPayload;
```




<hr>




### variable gradientPayload {#variable-gradientpayload}

_Gradient stroke parameters._ 
```C++
const GradientColorPayload& QAccelPlot::LineCurveRenderParams::gradientPayload;
```




<hr>




### variable hovered {#variable-hovered}

_Whether the curve is currently hovered._ 
```C++
bool QAccelPlot::LineCurveRenderParams::hovered;
```




<hr>




### variable lineStyle {#variable-linestyle}

_Active line style (dash pattern etc.)._ 
```C++
const LineStyle* QAccelPlot::LineCurveRenderParams::lineStyle;
```




<hr>




### variable lineWidth {#variable-linewidth}

_Line width in pixels._ 
```C++
qreal QAccelPlot::LineCurveRenderParams::lineWidth;
```




<hr>




### variable logScaleX {#variable-logscalex}

_Whether the X axis uses log scale._ 
```C++
bool QAccelPlot::LineCurveRenderParams::logScaleX;
```




<hr>




### variable logScaleY {#variable-logscaley}

_Whether the Y axis uses log scale._ 
```C++
bool QAccelPlot::LineCurveRenderParams::logScaleY;
```




<hr>




### variable pointCount {#variable-pointcount}

_Number of points in_ `data` _._
```C++
int QAccelPlot::LineCurveRenderParams::pointCount;
```




<hr>




### variable sourceData {#variable-sourcedata}

_Original float or double data used by CPU-side operations._ 
```C++
CurveDataView QAccelPlot::LineCurveRenderParams::sourceData;
```




<hr>




### variable vertexCache {#variable-vertexcache}

_Pre-built vertex buffer, or_ `nullptr` _to build on the render thread._
```C++
const std::vector<char>* QAccelPlot::LineCurveRenderParams::vertexCache;
```




<hr>




### variable viewportSize {#variable-viewportsize}

_Viewport size in pixels._ 
```C++
QVector2D QAccelPlot::LineCurveRenderParams::viewportSize;
```




<hr>




### variable window {#variable-window}

_Window providing the texture upload context._ 
```C++
QQuickWindow* QAccelPlot::LineCurveRenderParams::window;
```




<hr>




### variable xAxis {#variable-xaxis}

_Horizontal axis._ 
```C++
Axis* QAccelPlot::LineCurveRenderParams::xAxis;
```




<hr>




### variable yAxis {#variable-yaxis}

_Vertical axis._ 
```C++
Axis* QAccelPlot::LineCurveRenderParams::yAxis;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/renderers/LineCurveLineRenderer.hpp`

