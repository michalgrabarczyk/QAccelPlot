








# Class QAccelPlot::LineCurvePointRenderer



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**LineCurvePointRenderer**](classQAccelPlot_1_1LineCurvePointRenderer.md)



_Internal renderer responsible for building and updating QSGNode marker geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._[More...](#detailed-description)

* `#include <LineCurvePointRenderer.hpp>`

























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  constexpr std::size\_t | [**kVertexCacheBytesPerPoint**](#variable-kvertexcachebytesperpoint)   = `{[**kVerticesPerPoint**](classQAccelPlot_1_1LineCurvePointRenderer.md#variable-kverticesperpoint) \* sizeof([**PointVertex**](structQAccelPlot_1_1PointVertex.md))}`<br>_Vertex cache bytes per marker._  |
|  constexpr int | [**kVerticesPerPoint**](#variable-kverticesperpoint)   = `{6}`<br>_Number of vertices drawn per marker._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**contains**](#function-contains) (const QPointF & point, const [**CurveHitTestParams**](structQAccelPlot_1_1CurveHitTestParams.md) & params) const<br>_Returns_ `true` _if__point_ _lies within__params.hitThreshold_ _pixels of any marker._ |
|  QSGNode \* | [**paint**](#function-paint) (QSGNode \* oldNode, const [**PointCurveRenderParams**](structQAccelPlot_1_1PointCurveRenderParams.md) & params) const<br>_Updates the scene graph node tree for the point markers._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**writeVertexCache**](#function-writevertexcache) (std::vector&lt; char &gt; & cache, int firstPoint, int pointCount) <br>_Writes the vertices of markers [_ _firstPoint_ _,__pointCount_ _) into__cache_ _, which holds_`kVertexCacheBytesPerPoint` _bytes per marker._ |


























## Detailed Description


Renders per-point markers as two triangles per point. The vertex buffer depends only on the point count, so data updates of the same size only upload the data texture. 


    
## Public Static Attributes Documentation





### variable kVertexCacheBytesPerPoint {#variable-kvertexcachebytesperpoint}

_Vertex cache bytes per marker._ 
```C++
constexpr std::size_t QAccelPlot::LineCurvePointRenderer::kVertexCacheBytesPerPoint;
```




<hr>




### variable kVerticesPerPoint {#variable-kverticesperpoint}

_Number of vertices drawn per marker._ 
```C++
constexpr int QAccelPlot::LineCurvePointRenderer::kVerticesPerPoint;
```




<hr>
## Public Functions Documentation





### function contains {#function-contains}

_Returns_ `true` _if__point_ _lies within__params.hitThreshold_ _pixels of any marker._
```C++
bool QAccelPlot::LineCurvePointRenderer::contains (
    const QPointF & point,
    const CurveHitTestParams & params
) const
```




<hr>




### function paint {#function-paint}

_Updates the scene graph node tree for the point markers._ 
```C++
QSGNode * QAccelPlot::LineCurvePointRenderer::paint (
    QSGNode * oldNode,
    const PointCurveRenderParams & params
) const
```



The vertex buffer is written only when the point count changes, from _params.vertexCache_ when it matches. 


        

<hr>
## Public Static Functions Documentation





### function writeVertexCache {#function-writevertexcache}

_Writes the vertices of markers [_ _firstPoint_ _,__pointCount_ _) into__cache_ _, which holds_`kVertexCacheBytesPerPoint` _bytes per marker._
```C++
static void QAccelPlot::LineCurvePointRenderer::writeVertexCache (
    std::vector< char > & cache,
    int firstPoint,
    int pointCount
) 
```



Marker vertices hold no data, so the vertices before _firstPoint_ stay valid. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/LineCurvePointRenderer.hpp`

