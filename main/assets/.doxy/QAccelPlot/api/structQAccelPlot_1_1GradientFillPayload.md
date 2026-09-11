








# Struct QAccelPlot::GradientFillPayload



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**GradientFillPayload**](structQAccelPlot_1_1GradientFillPayload.md)



_Render-thread snapshot of gradient fill (area-under-curve) parameters._ [More...](#detailed-description)

* `#include <GradientColorTypes.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**GradientFillBaseline**](namespaceQAccelPlot_1_1GradientFillBaselineNS.md#enum-mode) | [**baseline**](#variable-baseline)   = `{GradientFillBaseline::AxisMinimum}`<br>_Baseline mode._  |
|  float | [**baselineValue**](#variable-baselinevalue)   = `{0.0f}`<br>_Fixed baseline data value when mode is_ `Value` _._ |
|  [**GradientDirection**](namespaceQAccelPlot_1_1GradientDirectionNS.md#enum-direction) | [**direction**](#variable-direction)   = `{GradientDirection::Horizontal}`<br>_Gradient direction._  |
|  bool | [**enabled**](#variable-enabled)   = `{false}`<br>`true` _when gradient fill is active._ |
|  std::optional&lt; float &gt; | [**gradientValueMax**](#variable-gradientvaluemax)  <br>_Fixed max for gradient normalisation;_ `std::nullopt` _= derived from data range._ |
|  std::optional&lt; float &gt; | [**gradientValueMin**](#variable-gradientvaluemin)  <br>_Fixed min for gradient normalisation;_ `std::nullopt` _= derived from data range._ |
|  float | [**opacity**](#variable-opacity)   = `{1.0f}`<br>_Overall opacity of the fill in [0, 1]._  |
|  std::vector&lt; [**GradientStopData**](structQAccelPlot_1_1GradientStopData.md) &gt; | [**stops**](#variable-stops)  <br>_Color stops in position order._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**isValid**](#function-isvalid) () const<br>_Returns_ `true` _if the payload has at least two stops, is enabled, and has non-zero opacity._ |




























## Detailed Description


Assembled on the main thread and consumed by the line renderer without touching the QObject hierarchy. 


    
## Public Attributes Documentation





### variable baseline {#variable-baseline}

_Baseline mode._ 
```C++
GradientFillBaseline QAccelPlot::GradientFillPayload::baseline;
```




<hr>




### variable baselineValue {#variable-baselinevalue}

_Fixed baseline data value when mode is_ `Value` _._
```C++
float QAccelPlot::GradientFillPayload::baselineValue;
```




<hr>




### variable direction {#variable-direction}

_Gradient direction._ 
```C++
GradientDirection QAccelPlot::GradientFillPayload::direction;
```




<hr>




### variable enabled {#variable-enabled}

`true` _when gradient fill is active._
```C++
bool QAccelPlot::GradientFillPayload::enabled;
```




<hr>




### variable gradientValueMax {#variable-gradientvaluemax}

_Fixed max for gradient normalisation;_ `std::nullopt` _= derived from data range._
```C++
std::optional<float> QAccelPlot::GradientFillPayload::gradientValueMax;
```




<hr>




### variable gradientValueMin {#variable-gradientvaluemin}

_Fixed min for gradient normalisation;_ `std::nullopt` _= derived from data range._
```C++
std::optional<float> QAccelPlot::GradientFillPayload::gradientValueMin;
```




<hr>




### variable opacity {#variable-opacity}

_Overall opacity of the fill in [0, 1]._ 
```C++
float QAccelPlot::GradientFillPayload::opacity;
```




<hr>




### variable stops {#variable-stops}

_Color stops in position order._ 
```C++
std::vector<GradientStopData> QAccelPlot::GradientFillPayload::stops;
```




<hr>
## Public Functions Documentation





### function isValid {#function-isvalid}

_Returns_ `true` _if the payload has at least two stops, is enabled, and has non-zero opacity._
```C++
inline bool QAccelPlot::GradientFillPayload::isValid () const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/effects/GradientColorTypes.hpp`

