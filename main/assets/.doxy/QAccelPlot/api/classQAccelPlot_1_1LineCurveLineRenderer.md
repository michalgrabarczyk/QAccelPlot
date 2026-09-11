








# Class QAccelPlot::LineCurveLineRenderer



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**LineCurveLineRenderer**](classQAccelPlot_1_1LineCurveLineRenderer.md)



_Internal renderer responsible for building and updating QSGNode line geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._[More...](#detailed-description)

* `#include <LineCurveLineRenderer.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**buildVertexCache**](#function-buildvertexcache) (const std::vector&lt; float &gt; & data, int pointCount, std::vector&lt; char &gt; & cache) const<br>_Pre-builds the vertex buffer into_ _cache_ _on the calling thread (main thread safe)._ |
|  bool | [**contains**](#function-contains) (const QPointF & point, const [**CurveHitTestParams**](structQAccelPlot_1_1CurveHitTestParams.md) & params) const<br>_Returns_ `true` _if__point_ _lies within__params.hitThreshold_ _pixels of the curve._ |
|  QSGNode \* | [**paint**](#function-paint) (QSGNode \* oldNode, const [**LineCurveRenderParams**](structQAccelPlot_1_1LineCurveRenderParams.md) & params) const<br>_Updates the scene graph node tree for this curve._  |




























## Detailed Description


Supports solid and dashed lines, gradient stroke, gradient fill, and optional pre-built vertex caches assembled on the main thread for reduced render-thread stalls. 


    
## Public Functions Documentation





### function buildVertexCache {#function-buildvertexcache}

_Pre-builds the vertex buffer into_ _cache_ _on the calling thread (main thread safe)._
```C++
void QAccelPlot::LineCurveLineRenderer::buildVertexCache (
    const std::vector< float > & data,
    int pointCount,
    std::vector< char > & cache
) const
```



Writes neutral RGBA (1,1,1,1); solid and gradient shaders obtain their colors from their materials. 


        

<hr>




### function contains {#function-contains}

_Returns_ `true` _if__point_ _lies within__params.hitThreshold_ _pixels of the curve._
```C++
bool QAccelPlot::LineCurveLineRenderer::contains (
    const QPointF & point,
    const CurveHitTestParams & params
) const
```




<hr>




### function paint {#function-paint}

_Updates the scene graph node tree for this curve._ 
```C++
QSGNode * QAccelPlot::LineCurveLineRenderer::paint (
    QSGNode * oldNode,
    const LineCurveRenderParams & params
) const
```



When _params.vertexCache_ is non-null the vertex data is transferred via a fast `memcpy`; otherwise geometry is built in-place on the render thread. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/renderers/LineCurveLineRenderer.hpp`

