








# Struct QAccelPlot::LineVertex



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**LineVertex**](structQAccelPlot_1_1LineVertex.md)



_Vertex layout for line geometry, shared with the main thread for pre-built vertex caches._ 

* `#include <LineCurveLineRenderer.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  unsigned char | [**a**](#variable-a)  <br>_Alpha channel (used when_ `useVertexColor` _is true)._ |
|  float | [**arcLength**](#variable-arclength)  <br>_Cumulative screen-space arc length in pixels; 0 for solid lines._  |
|  unsigned char | [**b**](#variable-b)  <br>_Blue channel (used when_ `useVertexColor` _is true)._ |
|  unsigned char | [**g**](#variable-g)  <br>_Green channel (used when_ `useVertexColor` _is true)._ |
|  float | [**id**](#variable-id)  <br>_Point index in the data buffer._  |
|  unsigned char | [**r**](#variable-r)  <br>_Red channel (used when_ `useVertexColor` _is true)._ |
|  float | [**side**](#variable-side)  <br>_Side of the line (-0.5 or +0.5) for screen-space extrusion._  |












































## Public Attributes Documentation





### variable a {#variable-a}

_Alpha channel (used when_ `useVertexColor` _is true)._
```C++
unsigned char QAccelPlot::LineVertex::a;
```




<hr>




### variable arcLength {#variable-arclength}

_Cumulative screen-space arc length in pixels; 0 for solid lines._ 
```C++
float QAccelPlot::LineVertex::arcLength;
```




<hr>




### variable b {#variable-b}

_Blue channel (used when_ `useVertexColor` _is true)._
```C++
unsigned char QAccelPlot::LineVertex::b;
```




<hr>




### variable g {#variable-g}

_Green channel (used when_ `useVertexColor` _is true)._
```C++
unsigned char QAccelPlot::LineVertex::g;
```




<hr>




### variable id {#variable-id}

_Point index in the data buffer._ 
```C++
float QAccelPlot::LineVertex::id;
```




<hr>




### variable r {#variable-r}

_Red channel (used when_ `useVertexColor` _is true)._
```C++
unsigned char QAccelPlot::LineVertex::r;
```




<hr>




### variable side {#variable-side}

_Side of the line (-0.5 or +0.5) for screen-space extrusion._ 
```C++
float QAccelPlot::LineVertex::side;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/renderers/LineCurveLineRenderer.hpp`

