








# Class QAccelPlot::LineCurveVertexCache



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**LineCurveVertexCache**](classQAccelPlot_1_1LineCurveVertexCache.md)



_Owns a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _'s pre-built vertex bytes and the metadata required to use them safely._[More...](#detailed-description)

* `#include <LineCurveVertexCache.hpp>`



















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**Layout**](#enum-layout)  <br>_Renderer geometry represented by the cached bytes._  |
| typedef std::function&lt; void(std::vector&lt; char &gt; &[**bytes**](classQAccelPlot_1_1LineCurveVertexCache.md#function-bytes), int firstPoint)&gt; | [**Writer**](#typedef-writer)  <br>_Function that writes the vertices of the points from_ _firstPoint_ _on into__bytes_ _, which is sized for all points._ |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  const std::vector&lt; char &gt; & | [**bytes**](#function-bytes) () noexcept const<br>_Returns the owned byte buffer, including retained invalid storage._  |
|  void | [**clear**](#function-clear) () noexcept<br>_Removes all cached bytes and resets the compatibility metadata._  |
|  const std::vector&lt; char &gt; \* | [**data**](#function-data) () noexcept const<br>_Returns the cached bytes when valid, or_ `nullptr` _otherwise._ |
|  void | [**invalidate**](#function-invalidate) () noexcept<br>_Marks the cache unusable while retaining its byte allocation._  |
|  bool | [**isReusableForDataChange**](#function-isreusablefordatachange) ([**Layout**](classQAccelPlot_1_1LineCurveVertexCache.md#enum-layout) requiredLayout, int pointCount) noexcept const<br>_Returns whether this cache can survive a data change with the requested layout._  |
|  [**Layout**](classQAccelPlot_1_1LineCurveVertexCache.md#enum-layout) | [**layout**](#function-layout) () noexcept const<br>_Returns the renderer geometry represented by the cache._  |
|  int | [**pointCount**](#function-pointcount) () noexcept const<br>_Returns the point count associated with the cache._  |
|  void | [**update**](#function-update) ([**Layout**](classQAccelPlot_1_1LineCurveVertexCache.md#enum-layout) layout, int pointCount, std::size\_t bytesPerPoint, const [**Writer**](classQAccelPlot_1_1LineCurveVertexCache.md#typedef-writer) & writer) <br>_Sizes the cache for_ _pointCount_ _points of__bytesPerPoint_ _bytes and writes the points it lacks with__writer_ _._ |
|  bool | [**valid**](#function-valid) () noexcept const<br>_Returns whether the cache contains compatible, usable data._  |




























## Detailed Description


A cache is valid only for the vertex layout and point count recorded when it was last updated. A vertex depends only on its point index, so an update to a new point count keeps the cached points and writes only the added ones. Invalidating a cache preserves its allocation for reuse; clearing it also releases the stored bytes. 


    
## Public Types Documentation





### enum Layout {#enum-layout}

_Renderer geometry represented by the cached bytes._ 
```C++
enum QAccelPlot::LineCurveVertexCache::Layout {
    None,
    Line,
    Points
};
```




<hr>




### typedef Writer {#typedef-writer}

_Function that writes the vertices of the points from_ _firstPoint_ _on into__bytes_ _, which is sized for all points._
```C++
using QAccelPlot::LineCurveVertexCache::Writer =  std::function<void(std::vector<char>& bytes, int firstPoint)>;
```




<hr>
## Public Functions Documentation





### function bytes {#function-bytes}

_Returns the owned byte buffer, including retained invalid storage._ 
```C++
const std::vector< char > & QAccelPlot::LineCurveVertexCache::bytes () noexcept const
```




<hr>




### function clear {#function-clear}

_Removes all cached bytes and resets the compatibility metadata._ 
```C++
void QAccelPlot::LineCurveVertexCache::clear () noexcept
```




<hr>




### function data {#function-data}

_Returns the cached bytes when valid, or_ `nullptr` _otherwise._
```C++
const std::vector< char > * QAccelPlot::LineCurveVertexCache::data () noexcept const
```




<hr>




### function invalidate {#function-invalidate}

_Marks the cache unusable while retaining its byte allocation._ 
```C++
void QAccelPlot::LineCurveVertexCache::invalidate () noexcept
```




<hr>




### function isReusableForDataChange {#function-isreusablefordatachange}

_Returns whether this cache can survive a data change with the requested layout._ 
```C++
bool QAccelPlot::LineCurveVertexCache::isReusableForDataChange (
    Layout requiredLayout,
    int pointCount
) noexcept const
```



Solid-line and marker cache vertices contain no XY values and can therefore be reused when the point count remains unchanged. 


        

<hr>




### function layout {#function-layout}

_Returns the renderer geometry represented by the cache._ 
```C++
Layout QAccelPlot::LineCurveVertexCache::layout () noexcept const
```




<hr>




### function pointCount {#function-pointcount}

_Returns the point count associated with the cache._ 
```C++
int QAccelPlot::LineCurveVertexCache::pointCount () noexcept const
```




<hr>




### function update {#function-update}

_Sizes the cache for_ _pointCount_ _points of__bytesPerPoint_ _bytes and writes the points it lacks with__writer_ _._
```C++
void QAccelPlot::LineCurveVertexCache::update (
    Layout layout,
    int pointCount,
    std::size_t bytesPerPoint,
    const Writer & writer
) 
```



A valid cache with the same _layout_ and _bytesPerPoint_ keeps its points, so _writer_ writes only the points past its previous point count, or none. Otherwise _writer_ writes all points. A _layout_ of `None`, or a zero _pointCount_ or _bytesPerPoint_, invalidates the cache. 


        

<hr>




### function valid {#function-valid}

_Returns whether the cache contains compatible, usable data._ 
```C++
bool QAccelPlot::LineCurveVertexCache::valid () noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/LineCurveVertexCache.hpp`

