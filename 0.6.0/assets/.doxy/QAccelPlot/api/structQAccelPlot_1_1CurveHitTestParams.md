








# Struct QAccelPlot::CurveHitTestParams



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**CurveHitTestParams**](structQAccelPlot_1_1CurveHitTestParams.md)



_All inputs required for a_ `contains()` _hit-test, bundled to reduce parameter count._

* `#include <CurveRendererParams.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  const std::vector&lt; [**CurveChunk**](structQAccelPlot_1_1CurveChunk.md) &gt; & | [**chunks**](#variable-chunks)  <br>_Precomputed chunk bounding boxes._  |
|  [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) | [**data**](#variable-data)  <br>_Interleaved XY source data without precision loss._  |
|  qreal | [**height**](#variable-height)  <br>_Height of the curve item in pixels._  |
|  qreal | [**hitThreshold**](#variable-hitthreshold)  <br>_Hit distance threshold in pixels._  |
|  bool | [**nonPositiveXInvalid**](#variable-nonpositivexinvalid)  <br>_Whether non-positive X values are invalid data points (X axis uses log scale)._  |
|  bool | [**nonPositiveYInvalid**](#variable-nonpositiveyinvalid)  <br>_Whether non-positive Y values are invalid data points (Y axis uses log scale)._  |
|  int | [**pointCount**](#variable-pointcount)  <br>_Number of leading points in_ `data` _to test; later points are never hit._ |
|  [**StepMode**](namespaceQAccelPlot_1_1StepModeNS.md#enum-mode) | [**stepMode**](#variable-stepmode)   = `{StepMode::None}`<br>_How the line joins consecutive data points._  |
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

_Interleaved XY source data without precision loss._ 
```C++
CurveDataView QAccelPlot::CurveHitTestParams::data;
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




### variable nonPositiveXInvalid {#variable-nonpositivexinvalid}

_Whether non-positive X values are invalid data points (X axis uses log scale)._ 
```C++
bool QAccelPlot::CurveHitTestParams::nonPositiveXInvalid;
```




<hr>




### variable nonPositiveYInvalid {#variable-nonpositiveyinvalid}

_Whether non-positive Y values are invalid data points (Y axis uses log scale)._ 
```C++
bool QAccelPlot::CurveHitTestParams::nonPositiveYInvalid;
```




<hr>




### variable pointCount {#variable-pointcount}

_Number of leading points in_ `data` _to test; later points are never hit._
```C++
int QAccelPlot::CurveHitTestParams::pointCount;
```




<hr>




### variable stepMode {#variable-stepmode}

_How the line joins consecutive data points._ 
```C++
StepMode QAccelPlot::CurveHitTestParams::stepMode;
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
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/CurveRendererParams.hpp`

