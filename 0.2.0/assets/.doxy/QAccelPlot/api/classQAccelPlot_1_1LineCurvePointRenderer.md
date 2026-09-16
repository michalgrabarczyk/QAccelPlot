








# Class QAccelPlot::LineCurvePointRenderer



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**LineCurvePointRenderer**](classQAccelPlot_1_1LineCurvePointRenderer.md)



_Internal renderer responsible for building and updating QSGNode marker geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._[More...](#detailed-description)

* `#include <LineCurvePointRenderer.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**buildVertexCache**](#function-buildvertexcache) (const std::vector&lt; float &gt; & data, int pointCount, std::vector&lt; char &gt; & cache) const<br>_Pre-builds the vertex buffer into_ _cache_ _on the calling thread (main thread safe)._ |
|  bool | [**contains**](#function-contains) (const QPointF & point, const [**CurveHitTestParams**](structQAccelPlot_1_1CurveHitTestParams.md) & params) const<br>_Returns_ `true` _if__point_ _lies within__params.hitThreshold_ _pixels of any marker._ |
|  QSGNode \* | [**paint**](#function-paint) (QSGNode \* oldNode, const [**PointCurveRenderParams**](structQAccelPlot_1_1PointCurveRenderParams.md) & params) const<br>_Updates the scene graph node tree for the point markers._  |




























## Detailed Description


Renders per-point markers using a single GPU-side quad per point. Supports pre-built vertex caches assembled on the main thread. 


    
## Public Functions Documentation





### function buildVertexCache {#function-buildvertexcache}

_Pre-builds the vertex buffer into_ _cache_ _on the calling thread (main thread safe)._
```C++
void QAccelPlot::LineCurvePointRenderer::buildVertexCache (
    const std::vector< float > & data,
    int pointCount,
    std::vector< char > & cache
) const
```




<hr>




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



When _params.vertexCache_ is non-null the vertex data is transferred via a fast `memcpy`. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/renderers/LineCurvePointRenderer.hpp`

