








# Struct QAccelPlot::Internal::ItemTrail



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**ItemTrail**](structQAccelPlot_1_1Internal_1_1ItemTrail.md)



_How the shaders fade the oldest items under a_ `FadeTrail` _effect._

* `#include <ItemFade.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::array&lt; float, kTrailEasingSamples &gt; | [**easing**](#variable-easing)   = `{}`<br>_How far the opacity has risen from_ `minOpacity` _to 1 at evenly spaced shares of the fading items._ |
|  int | [**itemCount**](#variable-itemcount)   = `{0}`<br>_Number of drawn items._  |
|  float | [**length**](#variable-length)   = `{0.0f}`<br>_Share of the items that fades, from the first; 0 fades none._  |
|  float | [**minOpacity**](#variable-minopacity)   = `{1.0f}`<br>_Opacity of the first item._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**active**](#function-active) () noexcept const<br>_Returns_ `true` _when some item fades._ |




























## Public Attributes Documentation





### variable easing {#variable-easing}

_How far the opacity has risen from_ `minOpacity` _to 1 at evenly spaced shares of the fading items._
```C++
std::array<float, kTrailEasingSamples> QAccelPlot::Internal::ItemTrail::easing;
```




<hr>




### variable itemCount {#variable-itemcount}

_Number of drawn items._ 
```C++
int QAccelPlot::Internal::ItemTrail::itemCount;
```




<hr>




### variable length {#variable-length}

_Share of the items that fades, from the first; 0 fades none._ 
```C++
float QAccelPlot::Internal::ItemTrail::length;
```




<hr>




### variable minOpacity {#variable-minopacity}

_Opacity of the first item._ 
```C++
float QAccelPlot::Internal::ItemTrail::minOpacity;
```




<hr>
## Public Functions Documentation





### function active {#function-active}

_Returns_ `true` _when some item fades._
```C++
inline bool QAccelPlot::Internal::ItemTrail::active () noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/internal/ItemFade.hpp`

