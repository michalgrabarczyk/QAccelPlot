








# Struct QAccelPlot::Internal::RectUbo



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**RectUbo**](structQAccelPlot_1_1Internal_1_1RectUbo.md)



_Mirrors the std140 uniform block of rect.vert._ [More...](#detailed-description)

* `#include <RectUniforms.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**borderColor**](#variable-bordercolor)  <br> |
|  float | [**borderWidth**](#variable-borderwidth)  <br> |
|  float | [**color**](#variable-color)  <br> |
|  float | [**domainMax**](#variable-domainmax)  <br> |
|  float | [**domainMin**](#variable-domainmin)  <br> |
|  float | [**hoverColor**](#variable-hovercolor)  <br> |
|  float | [**hoveredIndex**](#variable-hoveredindex)  <br> |
|  float | [**logScaleX**](#variable-logscalex)  <br> |
|  float | [**logScaleY**](#variable-logscaley)  <br> |
|  float | [**matrix**](#variable-matrix)  <br> |
|  float | [**minimumSize**](#variable-minimumsize)  <br> |
|  float | [**opacity**](#variable-opacity)  <br> |
|  float | [**rectCount**](#variable-rectcount)  <br> |
|  float | [**useVertexColor**](#variable-usevertexcolor)  <br> |
|  float | [**viewportSize**](#variable-viewportsize)  <br> |












































## Detailed Description


Members are ordered by alignment (mat4, vec4, vec2, float), so every offset is already aligned and the block needs no padding. 


    
## Public Attributes Documentation





### variable borderColor {#variable-bordercolor}

```C++
float QAccelPlot::Internal::RectUbo::borderColor[4];
```




<hr>




### variable borderWidth {#variable-borderwidth}

```C++
float QAccelPlot::Internal::RectUbo::borderWidth;
```




<hr>




### variable color {#variable-color}

```C++
float QAccelPlot::Internal::RectUbo::color[4];
```




<hr>




### variable domainMax {#variable-domainmax}

```C++
float QAccelPlot::Internal::RectUbo::domainMax[2];
```




<hr>




### variable domainMin {#variable-domainmin}

```C++
float QAccelPlot::Internal::RectUbo::domainMin[2];
```




<hr>




### variable hoverColor {#variable-hovercolor}

```C++
float QAccelPlot::Internal::RectUbo::hoverColor[4];
```




<hr>




### variable hoveredIndex {#variable-hoveredindex}

```C++
float QAccelPlot::Internal::RectUbo::hoveredIndex;
```




<hr>




### variable logScaleX {#variable-logscalex}

```C++
float QAccelPlot::Internal::RectUbo::logScaleX;
```




<hr>




### variable logScaleY {#variable-logscaley}

```C++
float QAccelPlot::Internal::RectUbo::logScaleY;
```




<hr>




### variable matrix {#variable-matrix}

```C++
float QAccelPlot::Internal::RectUbo::matrix[16];
```




<hr>




### variable minimumSize {#variable-minimumsize}

```C++
float QAccelPlot::Internal::RectUbo::minimumSize[2];
```




<hr>




### variable opacity {#variable-opacity}

```C++
float QAccelPlot::Internal::RectUbo::opacity;
```




<hr>




### variable rectCount {#variable-rectcount}

```C++
float QAccelPlot::Internal::RectUbo::rectCount;
```




<hr>




### variable useVertexColor {#variable-usevertexcolor}

```C++
float QAccelPlot::Internal::RectUbo::useVertexColor;
```




<hr>




### variable viewportSize {#variable-viewportsize}

```C++
float QAccelPlot::Internal::RectUbo::viewportSize[2];
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/materials/internal/RectUniforms.hpp`

