








# Class QAccelPlot::Internal::FrontStarts



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**FrontStarts**](classQAccelPlot_1_1Internal_1_1FrontStarts.md)



_When an eased reveal front reaches each share of the items._ 

* `#include <TransitionFrame.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**FrontStarts**](#function-frontstarts) (const QEasingCurve & easing) <br>_Samples the front that_ _easing_ _moves._ |
|  double | [**operator()**](#function-operator) (double share) const<br>_Returns the progress in [0, 1] at which the eased progress first reaches_ _share_ _._ |




























## Public Functions Documentation





### function FrontStarts {#function-frontstarts}

_Samples the front that_ _easing_ _moves._
```C++
explicit QAccelPlot::Internal::FrontStarts::FrontStarts (
    const QEasingCurve & easing
) 
```




<hr>




### function operator() {#function-operator}

_Returns the progress in [0, 1] at which the eased progress first reaches_ _share_ _._
```C++
double QAccelPlot::Internal::FrontStarts::operator() (
    double share
) const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/transitions/internal/TransitionFrame.hpp`

