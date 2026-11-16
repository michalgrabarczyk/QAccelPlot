








# Struct QAccelPlot::Internal::ItemFadeGroupUniforms



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**ItemFadeGroupUniforms**](structQAccelPlot_1_1Internal_1_1ItemFadeGroupUniforms.md)



_The std140 uniform members of one fading transition group, see_ `ItemFadeGroup` _._

* `#include <ItemOpacityUniforms.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**curve**](#variable-curve)  <br>_Opacity over the timeline of an item._  |
|  float | [**range**](#variable-range)  <br>_First item, item count (0: no fade), 1 when the starts follow a front, 1 when the items exit._  |
|  float | [**starts**](#variable-starts)  <br>_Start times along the front._  |
|  float | [**timing**](#variable-timing)  <br>_Progress, spread, and timeline length of each item; then padding._  |












































## Public Attributes Documentation





### variable curve {#variable-curve}

_Opacity over the timeline of an item._ 
```C++
float QAccelPlot::Internal::ItemFadeGroupUniforms::curve[kFadeCurveSamples];
```




<hr>




### variable range {#variable-range}

_First item, item count (0: no fade), 1 when the starts follow a front, 1 when the items exit._ 
```C++
float QAccelPlot::Internal::ItemFadeGroupUniforms::range[4];
```




<hr>




### variable starts {#variable-starts}

_Start times along the front._ 
```C++
float QAccelPlot::Internal::ItemFadeGroupUniforms::starts[kFadeStartSamples];
```




<hr>




### variable timing {#variable-timing}

_Progress, spread, and timeline length of each item; then padding._ 
```C++
float QAccelPlot::Internal::ItemFadeGroupUniforms::timing[4];
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/materials/internal/ItemOpacityUniforms.hpp`

