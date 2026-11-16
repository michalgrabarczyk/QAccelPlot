








# Class QAccelPlot::Internal::OpacityTimeline



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**OpacityTimeline**](classQAccelPlot_1_1Internal_1_1OpacityTimeline.md)



_The opacity of an item over the time since it started, built from opacity animations._ [More...](#detailed-description)

* `#include <OpacityTimeline.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**OpacityTimeline**](#function-opacitytimeline-12) () = default<br>_Constructs a timeline without segments._  |
|   | [**OpacityTimeline**](#function-opacitytimeline-22) (std::vector&lt; [**OpacitySegment**](structQAccelPlot_1_1Internal_1_1OpacitySegment.md) &gt; segments, double length) <br>_Constructs the timeline of_ _segments_ _, in declaration order, lasting__length_ _milliseconds._ |
|  bool | [**empty**](#function-empty) () noexcept const<br>_Returns_ `true` _when no segment animates opacity._ |
|  double | [**length**](#function-length) () noexcept const<br>_Returns the duration of the timeline in milliseconds._  |
|  double | [**valueAt**](#function-valueat-12) (double time) const<br>_Returns the opacity_ _time_ _milliseconds after the item started, not clamped._ |




























## Detailed Description


At a given time, the segment that started last applies; segments that start together apply in their order. Before the first segment, the item shows that segment's start opacity, and an item without segments is opaque. 


    
## Public Functions Documentation





### function OpacityTimeline {#function-opacitytimeline-12}

_Constructs a timeline without segments._ 
```C++
QAccelPlot::Internal::OpacityTimeline::OpacityTimeline () = default
```




<hr>




### function OpacityTimeline {#function-opacitytimeline-22}

_Constructs the timeline of_ _segments_ _, in declaration order, lasting__length_ _milliseconds._
```C++
QAccelPlot::Internal::OpacityTimeline::OpacityTimeline (
    std::vector< OpacitySegment > segments,
    double length
) 
```



_length_ can exceed the end of the last segment, for example after a pause. 


        

<hr>




### function empty {#function-empty}

_Returns_ `true` _when no segment animates opacity._
```C++
bool QAccelPlot::Internal::OpacityTimeline::empty () noexcept const
```




<hr>




### function length {#function-length}

_Returns the duration of the timeline in milliseconds._ 
```C++
double QAccelPlot::Internal::OpacityTimeline::length () noexcept const
```




<hr>




### function valueAt {#function-valueat-12}

_Returns the opacity_ _time_ _milliseconds after the item started, not clamped._
```C++
double QAccelPlot::Internal::OpacityTimeline::valueAt (
    double time
) const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/transitions/internal/OpacityTimeline.hpp`

