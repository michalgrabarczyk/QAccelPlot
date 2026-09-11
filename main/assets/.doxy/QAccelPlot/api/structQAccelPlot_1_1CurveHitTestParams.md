








# Struct QAccelPlot::CurveHitTestParams



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**CurveHitTestParams**](structQAccelPlot_1_1CurveHitTestParams.md)



_All inputs required for a_ `contains()` _hit-test, bundled to reduce parameter count._

* `#include <CurveRendererParams.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  const std::vector&lt; [**CurveChunk**](structQAccelPlot_1_1CurveChunk.md) &gt; & | [**chunks**](#variable-chunks)  <br>_Precomputed chunk bounding boxes._  |
|  const std::vector&lt; float &gt; & | [**data**](#variable-data)  <br>_Interleaved XY float data buffer._  |
|  qreal | [**height**](#variable-height)  <br>_Height of the curve item in pixels._  |
|  qreal | [**hitThreshold**](#variable-hitthreshold)  <br>_Hit distance threshold in pixels._  |
|  int | [**pointCount**](#variable-pointcount)  <br>_Number of points in_ `data` _._ |
|  qreal | [**width**](#variable-width)  <br>_Width of the curve item in pixels._  |
|  [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**xAxis**](#variable-xaxis)  <br>_Horizontal axis for coordinate mapping._  |
|  [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**yAxis**](#variable-yaxis)  <br>_Vertical axis for coordinate mapping._  |












































## Public Attributes Documentation





### variable chunks {#variable-chunks}

_Precomputed chunk bounding boxes._ 
```C++
const std::vector<CurveChunk>& QAccelPlot::CurveHitTestParams::chunks;
```




<hr>




### variable data {#variable-data}

_Interleaved XY float data buffer._ 
```C++
const std::vector<float>& QAccelPlot::CurveHitTestParams::data;
```




<hr>




### variable height {#variable-height}

_Height of the curve item in pixels._ 
```C++
qreal QAccelPlot::CurveHitTestParams::height;
```




<hr>




### variable hitThreshold {#variable-hitthreshold}

_Hit distance threshold in pixels._ 
```C++
qreal QAccelPlot::CurveHitTestParams::hitThreshold;
```




<hr>




### variable pointCount {#variable-pointcount}

_Number of points in_ `data` _._
```C++
int QAccelPlot::CurveHitTestParams::pointCount;
```




<hr>




### variable width {#variable-width}

_Width of the curve item in pixels._ 
```C++
qreal QAccelPlot::CurveHitTestParams::width;
```




<hr>




### variable xAxis {#variable-xaxis}

_Horizontal axis for coordinate mapping._ 
```C++
Axis* QAccelPlot::CurveHitTestParams::xAxis;
```




<hr>




### variable yAxis {#variable-yaxis}

_Vertical axis for coordinate mapping._ 
```C++
Axis* QAccelPlot::CurveHitTestParams::yAxis;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/renderers/CurveRendererParams.hpp`

