








# Struct QAccelPlot::DashedLineVertex



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**DashedLineVertex**](structQAccelPlot_1_1DashedLineVertex.md)



_Vertex of a dashed line ribbon: a_ `LineVertex` _and the screen-space arc length the dash pattern follows._

* `#include <LineStroke.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**arcLength**](#variable-arclength)  <br>_Cumulative screen-space arc length in pixels._  |
|  [**LineVertex**](structQAccelPlot_1_1LineVertex.md) | [**vertex**](#variable-vertex)  <br>_Data point index, path corner, join half, and ribbon side._  |












































## Public Attributes Documentation





### variable arcLength {#variable-arclength}

_Cumulative screen-space arc length in pixels._ 
```C++
float QAccelPlot::DashedLineVertex::arcLength;
```




<hr>




### variable vertex {#variable-vertex}

_Data point index, path corner, join half, and ribbon side._ 
```C++
LineVertex QAccelPlot::DashedLineVertex::vertex;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/LineStroke.hpp`

