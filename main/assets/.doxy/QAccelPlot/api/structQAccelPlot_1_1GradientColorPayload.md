








# Struct QAccelPlot::GradientColorPayload



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**GradientColorPayload**](structQAccelPlot_1_1GradientColorPayload.md)



_Render-thread snapshot of gradient stroke (line-color) parameters._ [More...](#detailed-description)

* `#include <GradientColorTypes.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**GradientDirection**](namespaceQAccelPlot_1_1GradientDirectionNS.md#enum-direction) | [**direction**](#variable-direction)   = `{GradientDirection::Horizontal}`<br>_Gradient direction._  |
|  bool | [**enabled**](#variable-enabled)   = `{false}`<br>`true` _when gradient stroke is active._ |
|  std::optional&lt; qreal &gt; | [**gradientValueMax**](#variable-gradientvaluemax)  <br>_Fixed max for gradient normalisation;_ `std::nullopt` _= derived from data range._ |
|  std::optional&lt; qreal &gt; | [**gradientValueMin**](#variable-gradientvaluemin)  <br>_Fixed min for gradient normalisation;_ `std::nullopt` _= derived from data range._ |
|  std::vector&lt; [**GradientStopData**](structQAccelPlot_1_1GradientStopData.md) &gt; | [**stops**](#variable-stops)  <br>_Color stops in position order._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**isValid**](#function-isvalid) () const<br>_Returns_ `true` _if the payload has at least two stops and is enabled._ |




























## Detailed Description


Assembled on the main thread and consumed by the line renderer without touching the QObject hierarchy. 


    
## Public Attributes Documentation





### variable direction {#variable-direction}

_Gradient direction._ 
```C++
GradientDirection QAccelPlot::GradientColorPayload::direction;
```




<hr>




### variable enabled {#variable-enabled}

`true` _when gradient stroke is active._
```C++
bool QAccelPlot::GradientColorPayload::enabled;
```




<hr>




### variable gradientValueMax {#variable-gradientvaluemax}

_Fixed max for gradient normalisation;_ `std::nullopt` _= derived from data range._
```C++
std::optional<qreal> QAccelPlot::GradientColorPayload::gradientValueMax;
```




<hr>




### variable gradientValueMin {#variable-gradientvaluemin}

_Fixed min for gradient normalisation;_ `std::nullopt` _= derived from data range._
```C++
std::optional<qreal> QAccelPlot::GradientColorPayload::gradientValueMin;
```




<hr>




### variable stops {#variable-stops}

_Color stops in position order._ 
```C++
std::vector<GradientStopData> QAccelPlot::GradientColorPayload::stops;
```




<hr>
## Public Functions Documentation





### function isValid {#function-isvalid}

_Returns_ `true` _if the payload has at least two stops and is enabled._
```C++
inline bool QAccelPlot::GradientColorPayload::isValid () const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/effects/GradientColorTypes.hpp`

