








# Struct QAccelPlot::LineVertex



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**LineVertex**](structQAccelPlot_1_1LineVertex.md)



_Vertex of a solid line ribbon, shared with the main thread for pre-built vertex caches._ [More...](#detailed-description)

* `#include <LineStroke.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::uint32\_t | [**packed**](#variable-packed)  <br>_Bits 0–27 hold the data point index, bits 28–29 the path corner, bit 30 marks the outgoing join half, and bit 31 the negative ribbon side._  |


## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  int | [**kMaximumCorner**](#variable-kmaximumcorner)   = `3`<br>_Largest path corner a vertex holds._  |
|  int | [**kMaximumIndex**](#variable-kmaximumindex)   = `(1 &lt;&lt; 28) - 1`<br>_Largest data point index a vertex holds._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  int | [**corner**](#function-corner) () noexcept const<br>_Returns the step corner after the data point; 0 for the point itself._  |
|  int | [**index**](#function-index) () noexcept const<br>_Returns the data point index._  |
|  bool | [**negativeSide**](#function-negativeside) () noexcept const<br>_Returns whether the vertex lies on the negative ribbon side._  |
|  bool | [**outgoing**](#function-outgoing) () noexcept const<br>_Returns whether the vertex starts the segment after a split join._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**LineVertex**](structQAccelPlot_1_1LineVertex.md) | [**make**](#function-make) (int index, int corner, bool negativeSide, bool outgoing=false) noexcept<br>_Returns the vertex of data point_ _index_ _at step corner__corner_ _(0 for the point itself) on the negative ribbon side when__negativeSide_ _._ |


























## Detailed Description


Packs a data point index, a path corner, a join half, and a ribbon side into four bytes, which the shaders read as normalized bytes (see line\_vertex\_input.glsl). 


    
## Public Attributes Documentation





### variable packed {#variable-packed}

_Bits 0–27 hold the data point index, bits 28–29 the path corner, bit 30 marks the outgoing join half, and bit 31 the negative ribbon side._ 
```C++
std::uint32_t QAccelPlot::LineVertex::packed;
```




<hr>
## Public Static Attributes Documentation





### variable kMaximumCorner {#variable-kmaximumcorner}

_Largest path corner a vertex holds._ 
```C++
int QAccelPlot::LineVertex::kMaximumCorner;
```




<hr>




### variable kMaximumIndex {#variable-kmaximumindex}

_Largest data point index a vertex holds._ 
```C++
int QAccelPlot::LineVertex::kMaximumIndex;
```




<hr>
## Public Functions Documentation





### function corner {#function-corner}

_Returns the step corner after the data point; 0 for the point itself._ 
```C++
int QAccelPlot::LineVertex::corner () noexcept const
```




<hr>




### function index {#function-index}

_Returns the data point index._ 
```C++
int QAccelPlot::LineVertex::index () noexcept const
```




<hr>




### function negativeSide {#function-negativeside}

_Returns whether the vertex lies on the negative ribbon side._ 
```C++
bool QAccelPlot::LineVertex::negativeSide () noexcept const
```




<hr>




### function outgoing {#function-outgoing}

_Returns whether the vertex starts the segment after a split join._ 
```C++
bool QAccelPlot::LineVertex::outgoing () noexcept const
```




<hr>
## Public Static Functions Documentation





### function make {#function-make}

_Returns the vertex of data point_ _index_ _at step corner__corner_ _(0 for the point itself) on the negative ribbon side when__negativeSide_ _._
```C++
static LineVertex QAccelPlot::LineVertex::make (
    int index,
    int corner,
    bool negativeSide,
    bool outgoing=false
) noexcept
```



_outgoing_ marks a vertex that starts the segment after a split join; see verticesPerPathPoint(). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/LineStroke.hpp`

