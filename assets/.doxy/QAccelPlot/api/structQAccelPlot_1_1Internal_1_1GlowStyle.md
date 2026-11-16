








# Struct QAccelPlot::Internal::GlowStyle



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**GlowStyle**](structQAccelPlot_1_1Internal_1_1GlowStyle.md)



_How the shaders draw a_ `Glow` _halo; see shaders/glow.glsl._

* `#include <GlowStyle.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  QColor | [**color**](#variable-color)  <br>_Halo color._  |
|  float | [**intensity**](#variable-intensity)   = `{0.0f}`<br>_Halo opacity next to the shape._  |
|  float | [**radius**](#variable-radius)   = `{0.0f}`<br>_Halo width in pixels; 0 draws no halo._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**active**](#function-active) () noexcept const<br>_Returns_ `true` _when the style draws a halo._ |




























## Public Attributes Documentation





### variable color {#variable-color}

_Halo color._ 
```C++
QColor QAccelPlot::Internal::GlowStyle::color;
```




<hr>




### variable intensity {#variable-intensity}

_Halo opacity next to the shape._ 
```C++
float QAccelPlot::Internal::GlowStyle::intensity;
```




<hr>




### variable radius {#variable-radius}

_Halo width in pixels; 0 draws no halo._ 
```C++
float QAccelPlot::Internal::GlowStyle::radius;
```




<hr>
## Public Functions Documentation





### function active {#function-active}

_Returns_ `true` _when the style draws a halo._
```C++
inline bool QAccelPlot::Internal::GlowStyle::active () noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/internal/GlowStyle.hpp`

