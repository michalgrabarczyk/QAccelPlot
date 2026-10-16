








# Struct QAccelPlot::BandEdgeRenderParams



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**BandEdgeRenderParams**](structQAccelPlot_1_1BandEdgeRenderParams.md)



_Inputs for_ [_**BandEdgeRenderer::paint()**_](classQAccelPlot_1_1BandEdgeRenderer.md#function-paint) _, assembled while the GUI thread is blocked._

* `#include <BandEdgeRenderer.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**dataChanged**](#variable-datachanged)  <br>_Whether the data points changed since the last paint._  |
|  [**DataPointStore::View**](structQAccelPlot_1_1DataPointStore_1_1View.md) | [**dataPoints**](#variable-datapoints)  <br>_The band's_ `(x, low, high)` _data points, used for dash lengths._ |
|  std::shared\_ptr&lt; [**DataTexture**](classQAccelPlot_1_1DataTexture.md) &gt; | [**dataTexture**](#variable-datatexture)  <br>_The band's data texture; the edge line never uploads data itself._  |
|  int | [**reservedCount**](#variable-reservedcount)  <br>_Data points the vertex buffer holds room for, at least_ `dataPoints.count` _, so appends rarely resize it._ |
|  [**LineStroke::Uniforms**](structQAccelPlot_1_1LineStroke_1_1Uniforms.md) | [**uniforms**](#variable-uniforms)  <br>_Line uniforms;_ `uniforms.pointCount` _equals_`dataPoints.count` _._ |
|  [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**xAxis**](#variable-xaxis)  <br>_Horizontal axis._  |
|  [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**yAxis**](#variable-yaxis)  <br>_Vertical axis._  |












































## Public Attributes Documentation





### variable dataChanged {#variable-datachanged}

_Whether the data points changed since the last paint._ 
```C++
bool QAccelPlot::BandEdgeRenderParams::dataChanged;
```




<hr>




### variable dataPoints {#variable-datapoints}

_The band's_ `(x, low, high)` _data points, used for dash lengths._
```C++
DataPointStore::View QAccelPlot::BandEdgeRenderParams::dataPoints;
```




<hr>




### variable dataTexture {#variable-datatexture}

_The band's data texture; the edge line never uploads data itself._ 
```C++
std::shared_ptr<DataTexture> QAccelPlot::BandEdgeRenderParams::dataTexture;
```




<hr>




### variable reservedCount {#variable-reservedcount}

_Data points the vertex buffer holds room for, at least_ `dataPoints.count` _, so appends rarely resize it._
```C++
int QAccelPlot::BandEdgeRenderParams::reservedCount;
```




<hr>




### variable uniforms {#variable-uniforms}

_Line uniforms;_ `uniforms.pointCount` _equals_`dataPoints.count` _._
```C++
LineStroke::Uniforms QAccelPlot::BandEdgeRenderParams::uniforms;
```




<hr>




### variable xAxis {#variable-xaxis}

_Horizontal axis._ 
```C++
Axis* QAccelPlot::BandEdgeRenderParams::xAxis;
```




<hr>




### variable yAxis {#variable-yaxis}

_Vertical axis._ 
```C++
Axis* QAccelPlot::BandEdgeRenderParams::yAxis;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/BandEdgeRenderer.hpp`

