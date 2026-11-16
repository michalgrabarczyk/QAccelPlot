








# Struct QAccelPlot::Internal::ItemTiming



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**ItemTiming**](structQAccelPlot_1_1Internal_1_1ItemTiming.md)



_The timing of the items of one group, as shares of the transition duration._ 

* `#include <TransitionFrame.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**front**](#variable-front)   = `{false}`<br>_Items start as an eased reveal front reaches them, at a stagger of 1._  |
|  double | [**length**](#variable-length)   = `{1.0}`<br>_Time each item animates: its movement or its opacity animations, whichever is longer._  |
|  double | [**motion**](#variable-motion)   = `{1.0}`<br>_Time each item takes to move._  |
|  double | [**spread**](#variable-spread)   = `{0.0}`<br>_Time over which the start times of the items spread._  |












































## Public Attributes Documentation





### variable front {#variable-front}

_Items start as an eased reveal front reaches them, at a stagger of 1._ 
```C++
bool QAccelPlot::Internal::ItemTiming::front;
```




<hr>




### variable length {#variable-length}

_Time each item animates: its movement or its opacity animations, whichever is longer._ 
```C++
double QAccelPlot::Internal::ItemTiming::length;
```




<hr>




### variable motion {#variable-motion}

_Time each item takes to move._ 
```C++
double QAccelPlot::Internal::ItemTiming::motion;
```




<hr>




### variable spread {#variable-spread}

_Time over which the start times of the items spread._ 
```C++
double QAccelPlot::Internal::ItemTiming::spread;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/transitions/internal/TransitionFrame.hpp`

