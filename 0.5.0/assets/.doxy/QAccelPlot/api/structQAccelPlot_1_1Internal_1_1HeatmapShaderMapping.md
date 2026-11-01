








# Struct QAccelPlot::Internal::HeatmapShaderMapping



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**HeatmapShaderMapping**](structQAccelPlot_1_1Internal_1_1HeatmapShaderMapping.md)



_How heatmap.frag maps an item-local pixel position_ _p_ _to a fractional cell coordinate along one dimension._

* `#include <HeatmapGrid.hpp>`



















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**Mode**](#enum-mode)  <br>_Formula the shader evaluates; the values match heatmap.frag._  |




## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**Mode**](structQAccelPlot_1_1Internal_1_1HeatmapShaderMapping.md#enum-mode) | [**mode**](#variable-mode)   = `{Mode::Affine}`<br>_Formula to evaluate._  |
|  double | [**offset**](#variable-offset)   = `{0.0}`<br>_Offset of the formula._  |
|  double | [**rate**](#variable-rate)   = `{0.0}`<br>_Per-pixel rate of the formula; unused by_ `Affine` _._ |
|  double | [**scale**](#variable-scale)   = `{0.0}`<br>_Factor of the formula._  |
|  double | [**shift**](#variable-shift)   = `{0.0}`<br>_Argument of_ `log1p` _at pixel 0; used by_`Logarithmic` _only._ |












































## Public Types Documentation





### enum Mode {#enum-mode}

_Formula the shader evaluates; the values match heatmap.frag._ 
```C++
enum QAccelPlot::Internal::HeatmapShaderMapping::Mode {
    Affine = 0,
    Exponential = 1,
    Logarithmic = 2
};
```




<hr>
## Public Attributes Documentation





### variable mode {#variable-mode}

_Formula to evaluate._ 
```C++
Mode QAccelPlot::Internal::HeatmapShaderMapping::mode;
```




<hr>




### variable offset {#variable-offset}

_Offset of the formula._ 
```C++
double QAccelPlot::Internal::HeatmapShaderMapping::offset;
```




<hr>




### variable rate {#variable-rate}

_Per-pixel rate of the formula; unused by_ `Affine` _._
```C++
double QAccelPlot::Internal::HeatmapShaderMapping::rate;
```




<hr>




### variable scale {#variable-scale}

_Factor of the formula._ 
```C++
double QAccelPlot::Internal::HeatmapShaderMapping::scale;
```




<hr>




### variable shift {#variable-shift}

_Argument of_ `log1p` _at pixel 0; used by_`Logarithmic` _only._
```C++
double QAccelPlot::Internal::HeatmapShaderMapping::shift;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/internal/HeatmapGrid.hpp`

