








# Struct QAccelPlot::Internal::ItemOpacityUniforms



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**ItemOpacityUniforms**](structQAccelPlot_1_1Internal_1_1ItemOpacityUniforms.md)



_The std140 uniform members that shaders/item\_opacity.glsl reads, at a 16-byte aligned offset._ 

* `#include <ItemOpacityUniforms.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**ItemFadeGroupUniforms**](structQAccelPlot_1_1Internal_1_1ItemFadeGroupUniforms.md) | [**groups**](#variable-groups)  <br>_The fading groups: kept items, then added or removed ones._  |
|  float | [**trail**](#variable-trail)  <br>_The_ `FadeTrail` _length, minimum opacity, and item count; then padding._ |
|  float | [**trailEasing**](#variable-traileasing)  <br>_The_ `FadeTrail` _easing curve._ |












































## Public Attributes Documentation





### variable groups {#variable-groups}

_The fading groups: kept items, then added or removed ones._ 
```C++
ItemFadeGroupUniforms QAccelPlot::Internal::ItemOpacityUniforms::groups[kFadeGroupCount];
```




<hr>




### variable trail {#variable-trail}

_The_ `FadeTrail` _length, minimum opacity, and item count; then padding._
```C++
float QAccelPlot::Internal::ItemOpacityUniforms::trail[4];
```




<hr>




### variable trailEasing {#variable-traileasing}

_The_ `FadeTrail` _easing curve._
```C++
float QAccelPlot::Internal::ItemOpacityUniforms::trailEasing[kTrailEasingSamples];
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/materials/internal/ItemOpacityUniforms.hpp`

