








# Struct QAccelPlot::CurveChunk



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**CurveChunk**](structQAccelPlot_1_1CurveChunk.md)



_Axis-aligned bounding box (AABB) for a contiguous block of curve points, used for hit-test culling._ [More...](#detailed-description)

* `#include <CurveRendererParams.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**count**](#variable-count)  <br>_Number of points owned by this chunk._  |
|  float | [**maxX**](#variable-maxx)  <br>_Maximum X of the data-space AABB (extended to bridging points)._  |
|  float | [**maxY**](#variable-maxy)  <br>_Maximum Y of the data-space AABB (extended to bridging points)._  |
|  float | [**minX**](#variable-minx)  <br>_Minimum X of the data-space AABB (extended to bridging points)._  |
|  float | [**minY**](#variable-miny)  <br>_Minimum Y of the data-space AABB (extended to bridging points)._  |
|  int | [**start**](#variable-start)  <br>_Index of the first point in the data buffer._  |












































## Detailed Description


The AABB is stored in data-space so it never needs rebuilding during pan/zoom — only when the underlying data changes. It intentionally extends one point beyond each chunk boundary so that line segments bridging adjacent chunks are always covered. 


    
## Public Attributes Documentation





### variable count {#variable-count}

_Number of points owned by this chunk._ 
```C++
int QAccelPlot::CurveChunk::count;
```




<hr>




### variable maxX {#variable-maxx}

_Maximum X of the data-space AABB (extended to bridging points)._ 
```C++
float QAccelPlot::CurveChunk::maxX;
```




<hr>




### variable maxY {#variable-maxy}

_Maximum Y of the data-space AABB (extended to bridging points)._ 
```C++
float QAccelPlot::CurveChunk::maxY;
```




<hr>




### variable minX {#variable-minx}

_Minimum X of the data-space AABB (extended to bridging points)._ 
```C++
float QAccelPlot::CurveChunk::minX;
```




<hr>




### variable minY {#variable-miny}

_Minimum Y of the data-space AABB (extended to bridging points)._ 
```C++
float QAccelPlot::CurveChunk::minY;
```




<hr>




### variable start {#variable-start}

_Index of the first point in the data buffer._ 
```C++
int QAccelPlot::CurveChunk::start;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/renderers/CurveRendererParams.hpp`

