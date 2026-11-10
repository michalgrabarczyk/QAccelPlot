








# Struct QAccelPlot::Internal::ItemFade



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**ItemFade**](structQAccelPlot_1_1Internal_1_1ItemFade.md)



_The transition groups of a frame whose opacity the shaders animate._ 

* `#include <ItemFade.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::array&lt; [**ItemFadeGroup**](structQAccelPlot_1_1Internal_1_1ItemFadeGroup.md), kFadeGroupCount &gt; | [**groups**](#variable-groups)   = `{}`<br>_The groups; inactive ones have no items._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**active**](#function-active) () noexcept const<br>_Returns_ `true` _when some group fades._ |




























## Public Attributes Documentation





### variable groups {#variable-groups}

_The groups; inactive ones have no items._ 
```C++
std::array<ItemFadeGroup, kFadeGroupCount> QAccelPlot::Internal::ItemFade::groups;
```




<hr>
## Public Functions Documentation





### function active {#function-active}

_Returns_ `true` _when some group fades._
```C++
inline bool QAccelPlot::Internal::ItemFade::active () noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/internal/ItemFade.hpp`

