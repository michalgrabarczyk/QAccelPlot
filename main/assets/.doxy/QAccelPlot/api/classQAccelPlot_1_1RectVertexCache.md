








# Class QAccelPlot::RectVertexCache



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**RectVertexCache**](classQAccelPlot_1_1RectVertexCache.md)



_Vertices of a series that draws each rectangle as a quad positioned from a data texture._ [More...](#detailed-description)

* `#include <RectVertexCache.hpp>`



















## Public Types

| Type | Name |
| ---: | :--- |
| typedef std::function&lt; QColor(int index)&gt; | [**ColorFunction**](#typedef-colorfunction)  <br>_Returns the color of rectangle_ _index_ _._ |






## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  constexpr int | [**kVerticesPerRect**](#variable-kverticesperrect)   = `{6}`<br>_Number of vertices per rectangle._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**copyTo**](#function-copyto) (QSGGeometry & geometry) const<br>_Reallocates_ _geometry_ _to_`vertexCount` _vertices and copies the vertices into it._ |
|  void | [**invalidate**](#function-invalidate) () noexcept<br>_Marks the vertices stale, keeping their allocation for the next rebuild._  |
|  void | [**rebuild**](#function-rebuild) (int rectCount, const [**ColorFunction**](classQAccelPlot_1_1RectVertexCache.md#typedef-colorfunction) & colorAt) <br>_Rebuilds the vertices of_ _rectCount_ _rectangles, keeping the allocation when it fits._ |
|  bool | [**valid**](#function-valid) () noexcept const<br>_Returns whether the vertices match the last rebuild._  |
|  int | [**vertexCount**](#function-vertexcount) () noexcept const<br>_Returns the number of cached vertices._  |




























## Detailed Description


Each rectangle has 6 vertices (two triangles) in the `DataTextureMaterial::attributeSet()` layout: rectangle index, quad corner (0–5), and color. The vertices hold no coordinates, so only a rectangle count or color change requires a rebuild. 


    
## Public Types Documentation





### typedef ColorFunction {#typedef-colorfunction}

_Returns the color of rectangle_ _index_ _._
```C++
using QAccelPlot::RectVertexCache::ColorFunction =  std::function<QColor(int index)>;
```




<hr>
## Public Static Attributes Documentation





### variable kVerticesPerRect {#variable-kverticesperrect}

_Number of vertices per rectangle._ 
```C++
constexpr int QAccelPlot::RectVertexCache::kVerticesPerRect;
```




<hr>
## Public Functions Documentation





### function copyTo {#function-copyto}

_Reallocates_ _geometry_ _to_`vertexCount` _vertices and copies the vertices into it._
```C++
void QAccelPlot::RectVertexCache::copyTo (
    QSGGeometry & geometry
) const
```



_geometry_ must use the `DataTextureMaterial::attributeSet()` layout. 


        

<hr>




### function invalidate {#function-invalidate}

_Marks the vertices stale, keeping their allocation for the next rebuild._ 
```C++
void QAccelPlot::RectVertexCache::invalidate () noexcept
```




<hr>




### function rebuild {#function-rebuild}

_Rebuilds the vertices of_ _rectCount_ _rectangles, keeping the allocation when it fits._
```C++
void QAccelPlot::RectVertexCache::rebuild (
    int rectCount,
    const ColorFunction & colorAt
) 
```



Vertex colors come from _colorAt_, or are white when it is empty, e.g. when the shader uses a uniform color instead. 


        

<hr>




### function valid {#function-valid}

_Returns whether the vertices match the last rebuild._ 
```C++
bool QAccelPlot::RectVertexCache::valid () noexcept const
```




<hr>




### function vertexCount {#function-vertexcount}

_Returns the number of cached vertices._ 
```C++
int QAccelPlot::RectVertexCache::vertexCount () noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/RectVertexCache.hpp`

