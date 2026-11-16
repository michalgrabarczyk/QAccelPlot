








# Struct QAccelPlot::Internal::OpacitySegment



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**OpacitySegment**](structQAccelPlot_1_1Internal_1_1OpacitySegment.md)



_One animation of item opacity, on the timeline of an item._ 

* `#include <OpacityTimeline.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  double | [**duration**](#variable-duration)   = `{0.0}`<br>_Duration in milliseconds; 0 sets_ `to` _at once._ |
|  QEasingCurve | [**easing**](#variable-easing)  <br>_Easing from_ `from` _to_`to` _._ |
|  std::optional&lt; double &gt; | [**from**](#variable-from)  <br>_Opacity at the start; without it, the opacity reached so far._  |
|  double | [**start**](#variable-start)   = `{0.0}`<br>_Start time in milliseconds after the item starts._  |
|  double | [**to**](#variable-to)   = `{1.0}`<br>_Opacity at the end._  |












































## Public Attributes Documentation





### variable duration {#variable-duration}

_Duration in milliseconds; 0 sets_ `to` _at once._
```C++
double QAccelPlot::Internal::OpacitySegment::duration;
```




<hr>




### variable easing {#variable-easing}

_Easing from_ `from` _to_`to` _._
```C++
QEasingCurve QAccelPlot::Internal::OpacitySegment::easing;
```




<hr>




### variable from {#variable-from}

_Opacity at the start; without it, the opacity reached so far._ 
```C++
std::optional<double> QAccelPlot::Internal::OpacitySegment::from;
```




<hr>




### variable start {#variable-start}

_Start time in milliseconds after the item starts._ 
```C++
double QAccelPlot::Internal::OpacitySegment::start;
```




<hr>




### variable to {#variable-to}

_Opacity at the end._ 
```C++
double QAccelPlot::Internal::OpacitySegment::to;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/transitions/internal/OpacityTimeline.hpp`

