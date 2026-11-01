








# Struct QAccelPlot::PointVertex



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**PointVertex**](structQAccelPlot_1_1PointVertex.md)



_Placeholder vertex of a marker quad, shared with the main thread for vertex caches._ [More...](#detailed-description)

* `#include <LineCurvePointRenderer.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  unsigned char | [**padding**](#variable-padding)  <br>_Ignored; written as zero._  |












































## Detailed Description


The marker shader reads the point from the data texture and derives the point index and quad corner from the vertex index, so it ignores the vertex content. 


    
## Public Attributes Documentation





### variable padding {#variable-padding}

_Ignored; written as zero._ 
```C++
unsigned char QAccelPlot::PointVertex::padding[4];
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/LineCurvePointRenderer.hpp`

