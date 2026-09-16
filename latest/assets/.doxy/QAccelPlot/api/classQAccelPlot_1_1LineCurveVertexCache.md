








# Class QAccelPlot::LineCurveVertexCache



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**LineCurveVertexCache**](classQAccelPlot_1_1LineCurveVertexCache.md)



_Owns a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _'s pre-built vertex bytes and the metadata required to use them safely._[More...](#detailed-description)

* `#include <LineCurveVertexCache.hpp>`



















## Public Types

| Type | Name |
| ---: | :--- |
| typedef std::function&lt; void(std::vector&lt; char &gt; &)&gt; | [**Builder**](#typedef-builder)  <br>_Function that writes a complete cache into the supplied reusable byte buffer._  |
| enum  | [**Layout**](#enum-layout)  <br>_Renderer geometry represented by the cached bytes._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  const std::vector&lt; char &gt; & | [**bytes**](#function-bytes) () noexcept const<br>_Returns the owned byte buffer, including retained invalid storage._  |
|  void | [**clear**](#function-clear) () noexcept<br>_Removes all cached bytes and resets the compatibility metadata._  |
|  const std::vector&lt; char &gt; \* | [**data**](#function-data) () noexcept const<br>_Returns the cached bytes when valid, or_ `nullptr` _otherwise._ |
|  bool | [**install**](#function-install) (std::vector&lt; char &gt; && bytes, [**Layout**](classQAccelPlot_1_1LineCurveVertexCache.md#enum-layout) layout, int pointCount, std::size\_t expectedSize) <br>_Installs externally built cache_ _bytes_ _._ |
|  void | [**invalidate**](#function-invalidate) () noexcept<br>_Marks the cache unusable while retaining its byte allocation._  |
|  bool | [**isReusableForDataChange**](#function-isreusablefordatachange) ([**Layout**](classQAccelPlot_1_1LineCurveVertexCache.md#enum-layout) requiredLayout, int pointCount) noexcept const<br>_Returns whether this cache can survive a data change with the requested layout._  |
|  [**Layout**](classQAccelPlot_1_1LineCurveVertexCache.md#enum-layout) | [**layout**](#function-layout) () noexcept const<br>_Returns the renderer geometry represented by the cache._  |
|  int | [**pointCount**](#function-pointcount) () noexcept const<br>_Returns the point count associated with the cache._  |
|  void | [**rebuild**](#function-rebuild) ([**Layout**](classQAccelPlot_1_1LineCurveVertexCache.md#enum-layout) layout, int pointCount, const [**Builder**](classQAccelPlot_1_1LineCurveVertexCache.md#typedef-builder) & builder) <br>_Rebuilds the cache with_ _builder_ _while retaining reusable allocation capacity._ |
|  bool | [**valid**](#function-valid) () noexcept const<br>_Returns whether the cache contains compatible, usable data._  |




























## Detailed Description


A cache is valid only for the vertex layout and point count recorded when it was installed or rebuilt. Invalidating a cache preserves its allocation for reuse; clearing it also releases the stored bytes. 


    
## Public Types Documentation





### typedef Builder {#typedef-builder}

_Function that writes a complete cache into the supplied reusable byte buffer._ 
```C++
using QAccelPlot::LineCurveVertexCache::Builder =  std::function<void(std::vector<char>&)>;
```




<hr>




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




### function install {#function-install}

_Installs externally built cache_ _bytes_ _._
```C++
bool QAccelPlot::LineCurveVertexCache::install (
    std::vector< char > && bytes,
    Layout layout,
    int pointCount,
    std::size_t expectedSize
) 
```



Returns `true` when the byte count matches _expectedSize_ and the supplied metadata describes a usable cache. On failure, the cache is cleared. 


        

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



Solid-line cache vertices contain no XY values and can therefore be reused when the point count remains unchanged. 


        

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




### function rebuild {#function-rebuild}

_Rebuilds the cache with_ _builder_ _while retaining reusable allocation capacity._
```C++
void QAccelPlot::LineCurveVertexCache::rebuild (
    Layout layout,
    int pointCount,
    const Builder & builder
) 
```



The rebuilt cache is valid only when the builder produces bytes and the supplied _layout_ and _pointCount_ are usable. 


        

<hr>




### function valid {#function-valid}

_Returns whether the cache contains compatible, usable data._ 
```C++
bool QAccelPlot::LineCurveVertexCache::valid () noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/series/LineCurveVertexCache.hpp`

