








# Class QAccelPlot::BandEdgeRenderer



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**BandEdgeRenderer**](classQAccelPlot_1_1BandEdgeRenderer.md)



_Internal renderer for the lower or upper edge line of a_ `BandSeries` _._[More...](#detailed-description)

* `#include <BandEdgeRenderer.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**BandEdgeRenderer**](#function-bandedgerenderer) ([**BandEdgeMaterial::Edge**](classQAccelPlot_1_1BandEdgeMaterial.md#enum-edge) edge) <br>_Constructs a renderer for the_ _edge_ _line._ |
|  QSGGeometryNode \* | [**paint**](#function-paint) (QSGGeometryNode \* oldNode, const [**BandEdgeRenderParams**](structQAccelPlot_1_1BandEdgeRenderParams.md) & params) const<br>_Updates_ _oldNode_ _, or creates the edge line node when it is_`nullptr` _, and returns it._ |




























## Detailed Description


Draws one line ribbon node that samples the band's data texture through a `BandEdgeMaterial`. 


    
## Public Functions Documentation





### function BandEdgeRenderer {#function-bandedgerenderer}

_Constructs a renderer for the_ _edge_ _line._
```C++
explicit QAccelPlot::BandEdgeRenderer::BandEdgeRenderer (
    BandEdgeMaterial::Edge edge
) 
```




<hr>




### function paint {#function-paint}

_Updates_ _oldNode_ _, or creates the edge line node when it is_`nullptr` _, and returns it._
```C++
QSGGeometryNode * QAccelPlot::BandEdgeRenderer::paint (
    QSGGeometryNode * oldNode,
    const BandEdgeRenderParams & params
) const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/BandEdgeRenderer.hpp`

