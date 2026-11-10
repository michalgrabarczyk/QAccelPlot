








# Struct QAccelPlot::Internal::GlowUniforms



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**GlowUniforms**](structQAccelPlot_1_1Internal_1_1GlowUniforms.md)



_The std140 uniform members of a_ `Glow` _halo,_`glowColor` _and_`glowParams` _, at a 16-byte aligned offset._

* `#include <GlowUniforms.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**color**](#variable-color)  <br>_Halo color, not premultiplied._  |
|  float | [**params**](#variable-params)  <br>_Halo radius in pixels and intensity; a zero radius draws no halo._  |












































## Public Attributes Documentation





### variable color {#variable-color}

_Halo color, not premultiplied._ 
```C++
float QAccelPlot::Internal::GlowUniforms::color[4];
```




<hr>




### variable params {#variable-params}

_Halo radius in pixels and intensity; a zero radius draws no halo._ 
```C++
float QAccelPlot::Internal::GlowUniforms::params[4];
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/materials/internal/GlowUniforms.hpp`

