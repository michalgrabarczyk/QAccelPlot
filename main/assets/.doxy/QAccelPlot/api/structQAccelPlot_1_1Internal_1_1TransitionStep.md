








# Struct QAccelPlot::Internal::TransitionStep



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**TransitionStep**](structQAccelPlot_1_1Internal_1_1TransitionStep.md)



_The state of one slot's transition at the frame._ 

* `#include <TransitionFrame.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  double | [**baseline**](#variable-baseline)   = `{std::numeric\_limits&lt;double&gt;::quiet\_NaN()}`<br>`GrowTransition` _baseline; NaN for the layout's._ |
|  double | [**duration**](#variable-duration)   = `{0.0}`<br>_Duration of the run in milliseconds, which times_ `opacity` _._ |
|  QEasingCurve | [**easing**](#variable-easing)  <br>_Easing of each item's progress._  |
|  TransitionKind | [**kind**](#variable-kind)   = `{TransitionKind::Morph}`<br>_How the transition moves items._  |
|  [**OpacityTimeline**](classQAccelPlot_1_1Internal_1_1OpacityTimeline.md) | [**opacity**](#variable-opacity)  <br>_Opacity of each item over its own time; empty for none._  |
|  double | [**progress**](#variable-progress)   = `{1.0}`<br>_Progress of the run, before easing._  |
|  double | [**stagger**](#variable-stagger)   = `{0.0}`<br>_Stagger of the transition._  |












































## Public Attributes Documentation





### variable baseline {#variable-baseline}

`GrowTransition` _baseline; NaN for the layout's._
```C++
double QAccelPlot::Internal::TransitionStep::baseline;
```




<hr>




### variable duration {#variable-duration}

_Duration of the run in milliseconds, which times_ `opacity` _._
```C++
double QAccelPlot::Internal::TransitionStep::duration;
```




<hr>




### variable easing {#variable-easing}

_Easing of each item's progress._ 
```C++
QEasingCurve QAccelPlot::Internal::TransitionStep::easing;
```




<hr>




### variable kind {#variable-kind}

_How the transition moves items._ 
```C++
TransitionKind QAccelPlot::Internal::TransitionStep::kind;
```




<hr>




### variable opacity {#variable-opacity}

_Opacity of each item over its own time; empty for none._ 
```C++
OpacityTimeline QAccelPlot::Internal::TransitionStep::opacity;
```




<hr>




### variable progress {#variable-progress}

_Progress of the run, before easing._ 
```C++
double QAccelPlot::Internal::TransitionStep::progress;
```




<hr>




### variable stagger {#variable-stagger}

_Stagger of the transition._ 
```C++
double QAccelPlot::Internal::TransitionStep::stagger;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/transitions/internal/TransitionFrame.hpp`

