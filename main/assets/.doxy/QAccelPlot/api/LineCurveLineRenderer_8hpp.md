








# File LineCurveLineRenderer.hpp



[**FileList**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**renderers**](dir_a5593d4bbe882811c43c55c842f746b7.md) **>** [**LineCurveLineRenderer.hpp**](LineCurveLineRenderer_8hpp.md)

[Go to the source code of this file](LineCurveLineRenderer_8hpp_source.md)



* `#include "QAccelPlot/axis/Axis.hpp"`
* `#include "QAccelPlot/effects/GradientColorTypes.hpp"`
* `#include "QAccelPlot/linestyles/LineStyle.hpp"`
* `#include "QAccelPlot/renderers/CurveRendererParams.hpp"`
* `#include "QAccelPlot/renderers/LineStroke.hpp"`
* `#include "QAccelPlot/series/LineCurveGapFilter.hpp"`
* `#include <QColor>`
* `#include <QPointF>`
* `#include <QSGGeometry>`
* `#include <QSGNode>`
* `#include <QVector2D>`
* `#include <vector>`















## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**QAccelPlot**](namespaceQAccelPlot.md) <br> |


## Classes

| Type | Name |
| ---: | :--- |
| struct | [**FillSamples**](structQAccelPlot_1_1FillSamples.md) <br>_Samples of a gradient fill: one group per valid-sample run, broken at gaps._  |
| class | [**LineCurveLineRenderer**](classQAccelPlot_1_1LineCurveLineRenderer.md) <br>_Internal renderer responsible for building and updating QSGNode line geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._ |
| struct | [**LineCurveRenderParams**](structQAccelPlot_1_1LineCurveRenderParams.md) <br>_Input parameters for_ [_**LineCurveLineRenderer::paint()**_](classQAccelPlot_1_1LineCurveLineRenderer.md#function-paint) _, assembled on the main thread._ |



















































------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/LineCurveLineRenderer.hpp`

