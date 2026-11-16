








# Struct QAccelPlot::Internal::ItemFadeGroup



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**ItemFadeGroup**](structQAccelPlot_1_1Internal_1_1ItemFadeGroup.md)



_The items of one transition group whose opacity the shaders animate._ [More...](#detailed-description)

* `#include <ItemFade.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**count**](#variable-count)   = `{0}`<br>_Number of items in the group; 0 when the group does not fade._  |
|  std::array&lt; float, kFadeCurveSamples &gt; | [**curve**](#variable-curve)   = `{}`<br>_Opacity at evenly spaced times over the timeline of an item, from its start to_ `length` _._ |
|  bool | [**exit**](#variable-exit)   = `{false}`<br>`true` _for removed items, which start from the last one._ |
|  int | [**first**](#variable-first)   = `{0}`<br>_Index of the first item of the group in the drawn items._  |
|  bool | [**front**](#variable-front)   = `{false}`<br>_The start times follow_ `starts` _rather than spreading evenly._ |
|  float | [**length**](#variable-length)   = `{0.0f}`<br>_Length of the timeline of each item._  |
|  float | [**spread**](#variable-spread)   = `{0.0f}`<br>_Time over which the start times of the items spread._  |
|  std::array&lt; float, kFadeStartSamples &gt; | [**starts**](#variable-starts)   = `{}`<br>_When_ `front` _, the start time over_`spread` _of the item at evenly spaced shares of the items._ |
|  float | [**time**](#variable-time)   = `{0.0f}`<br>_Progress of the transition._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**active**](#function-active) () noexcept const<br>_Returns_ `true` _when the group fades._ |




























## Detailed Description


The shaders compute each item's opacity from these values, see shaders/item\_opacity.glsl. Times are shares of the transition duration. 


    
## Public Attributes Documentation





### variable count {#variable-count}

_Number of items in the group; 0 when the group does not fade._ 
```C++
int QAccelPlot::Internal::ItemFadeGroup::count;
```




<hr>




### variable curve {#variable-curve}

_Opacity at evenly spaced times over the timeline of an item, from its start to_ `length` _._
```C++
std::array<float, kFadeCurveSamples> QAccelPlot::Internal::ItemFadeGroup::curve;
```




<hr>




### variable exit {#variable-exit}

`true` _for removed items, which start from the last one._
```C++
bool QAccelPlot::Internal::ItemFadeGroup::exit;
```




<hr>




### variable first {#variable-first}

_Index of the first item of the group in the drawn items._ 
```C++
int QAccelPlot::Internal::ItemFadeGroup::first;
```




<hr>




### variable front {#variable-front}

_The start times follow_ `starts` _rather than spreading evenly._
```C++
bool QAccelPlot::Internal::ItemFadeGroup::front;
```




<hr>




### variable length {#variable-length}

_Length of the timeline of each item._ 
```C++
float QAccelPlot::Internal::ItemFadeGroup::length;
```




<hr>




### variable spread {#variable-spread}

_Time over which the start times of the items spread._ 
```C++
float QAccelPlot::Internal::ItemFadeGroup::spread;
```




<hr>




### variable starts {#variable-starts}

_When_ `front` _, the start time over_`spread` _of the item at evenly spaced shares of the items._
```C++
std::array<float, kFadeStartSamples> QAccelPlot::Internal::ItemFadeGroup::starts;
```




<hr>




### variable time {#variable-time}

_Progress of the transition._ 
```C++
float QAccelPlot::Internal::ItemFadeGroup::time;
```




<hr>
## Public Functions Documentation





### function active {#function-active}

_Returns_ `true` _when the group fades._
```C++
inline bool QAccelPlot::Internal::ItemFadeGroup::active () noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/internal/ItemFade.hpp`

