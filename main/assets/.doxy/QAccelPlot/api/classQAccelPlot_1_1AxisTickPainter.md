








# Class QAccelPlot::AxisTickPainter



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**AxisTickPainter**](classQAccelPlot_1_1AxisTickPainter.md)



_Internal helper that paints tick marks and labels onto a QPainter for a single_ [_**Axis**_](classQAccelPlot_1_1Axis.md) _._[More...](#detailed-description)

* `#include <AxisTickPainter.hpp>`

















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**PaintContext**](structQAccelPlot_1_1AxisTickPainter_1_1PaintContext.md) <br>_Groups the QPainter context arguments passed into sub-painting helpers._  |
| struct | [**Params**](structQAccelPlot_1_1AxisTickPainter_1_1Params.md) <br>_All inputs required for a single paint call, bundled to reduce parameter count._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef std::function&lt; qreal(qreal value, qreal length)&gt; | [**MapToPosition**](#typedef-maptoposition)  <br>_Function type that maps a data value to a pixel position along the axis._  |






















## Public Static Functions

| Type | Name |
| ---: | :--- |
|  qreal | [**computeNiceStep**](#function-computenicestep) (qreal viewportMin, qreal viewportMax, int tickCount) <br>_Returns a "nice" major tick step for the given_ _viewportMin_ _,__viewportMax_ _range and target__tickCount_ _._ |
|  void | [**paintTicks**](#function-paintticks) (QPainter \* painter, const QRectF & rect, qreal axisX, qreal axisY, const [**Params**](structQAccelPlot_1_1AxisTickPainter_1_1Params.md) & params, const [**MapToPosition**](classQAccelPlot_1_1AxisTickPainter.md#typedef-maptoposition) & mapToPosition) <br>_Paints all tick marks and labels onto_ _painter_ _using the supplied__params_ _._ |


























## Detailed Description


Used by `Axis::paint()`. Also provides `computeNiceStep()`, which is shared with `GridNode` to keep grid lines aligned with tick positions. 


    
## Public Types Documentation





### typedef MapToPosition {#typedef-maptoposition}

_Function type that maps a data value to a pixel position along the axis._ 
```C++
using QAccelPlot::AxisTickPainter::MapToPosition =  std::function<qreal(qreal value, qreal length)>;
```




<hr>
## Public Static Functions Documentation





### function computeNiceStep {#function-computenicestep}

_Returns a "nice" major tick step for the given_ _viewportMin_ _,__viewportMax_ _range and target__tickCount_ _._
```C++
static qreal QAccelPlot::AxisTickPainter::computeNiceStep (
    qreal viewportMin,
    qreal viewportMax,
    int tickCount
) 
```



Shared by both [**AxisTickPainter**](classQAccelPlot_1_1AxisTickPainter.md) and [**GridNode**](classQAccelPlot_1_1GridNode.md) so grid lines remain aligned with ticks. 


        

<hr>




### function paintTicks {#function-paintticks}

_Paints all tick marks and labels onto_ _painter_ _using the supplied__params_ _._
```C++
static void QAccelPlot::AxisTickPainter::paintTicks (
    QPainter * painter,
    const QRectF & rect,
    qreal axisX,
    qreal axisY,
    const Params & params,
    const MapToPosition & mapToPosition
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/axis/AxisTickPainter.hpp`

