








# Class QAccelPlot::LineCurveLineRenderer



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**LineCurveLineRenderer**](classQAccelPlot_1_1LineCurveLineRenderer.md)



_Internal renderer responsible for building and updating QSGNode line geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._[More...](#detailed-description)

* `#include <LineCurveLineRenderer.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**contains**](#function-contains) (const QPointF & point, const [**CurveHitTestParams**](structQAccelPlot_1_1CurveHitTestParams.md) & params) const<br>_Returns_ `true` _if__point_ _lies within__params.hitThreshold_ _pixels of the curve._ |
|  QSGNode \* | [**paint**](#function-paint) (QSGNode \* oldNode, const [**LineCurveRenderParams**](structQAccelPlot_1_1LineCurveRenderParams.md) & params) const<br>_Updates the scene graph node tree for this curve._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  std::shared\_ptr&lt; [**DataTexture**](classQAccelPlot_1_1DataTexture.md) &gt; | [**dataTexture**](#function-datatexture) (QSGNode \* node) <br>_Returns the data texture of the line in_ _node_ _, a tree returned by_[_**paint()**_](classQAccelPlot_1_1LineCurveLineRenderer.md#function-paint) _, or_`nullptr` _before an upload._ |
|  std::size\_t | [**vertexCacheBytesPerPoint**](#function-vertexcachebytesperpoint) ([**StepMode**](namespaceQAccelPlot_1_1StepModeNS.md#enum-mode) stepMode) <br>_Returns the vertex cache bytes per data point of a line drawn with_ _stepMode_ _._ |
|  void | [**writeVertexCache**](#function-writevertexcache) (std::vector&lt; char &gt; & cache, int firstPoint, int pointCount, [**StepMode**](namespaceQAccelPlot_1_1StepModeNS.md#enum-mode) stepMode) <br>_Writes the vertices of data points [_ _firstPoint_ _,__pointCount_ _) of a line drawn with__stepMode_ _into__cache_ _._ |


























## Detailed Description


Supports solid and dashed lines, gradient stroke, gradient fill, and optional pre-built vertex caches assembled on the main thread for reduced render-thread stalls. 


    
## Public Functions Documentation





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
## Public Static Functions Documentation





### function dataTexture {#function-datatexture}

_Returns the data texture of the line in_ _node_ _, a tree returned by_[_**paint()**_](classQAccelPlot_1_1LineCurveLineRenderer.md#function-paint) _, or_`nullptr` _before an upload._
```C++
static std::shared_ptr< DataTexture > QAccelPlot::LineCurveLineRenderer::dataTexture (
    QSGNode * node
) 
```




<hr>




### function vertexCacheBytesPerPoint {#function-vertexcachebytesperpoint}

_Returns the vertex cache bytes per data point of a line drawn with_ _stepMode_ _._
```C++
static std::size_t QAccelPlot::LineCurveLineRenderer::vertexCacheBytesPerPoint (
    StepMode stepMode
) 
```




<hr>




### function writeVertexCache {#function-writevertexcache}

_Writes the vertices of data points [_ _firstPoint_ _,__pointCount_ _) of a line drawn with__stepMode_ _into__cache_ _._
```C++
static void QAccelPlot::LineCurveLineRenderer::writeVertexCache (
    std::vector< char > & cache,
    int firstPoint,
    int pointCount,
    StepMode stepMode
) 
```



_cache_ holds [**vertexCacheBytesPerPoint()**](classQAccelPlot_1_1LineCurveLineRenderer.md#function-vertexcachebytesperpoint) bytes for each of _pointCount_ data points. A vertex depends only on its data point index and step corner, never on data values, so the vertices before _firstPoint_ stay valid. Writes neutral RGBA (1,1,1,1); solid and gradient shaders obtain their colors from their materials. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/LineCurveLineRenderer.hpp`

