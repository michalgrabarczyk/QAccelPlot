








# Class QAccelPlot::AxisTickPainter



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**AxisTickPainter**](classQAccelPlot_1_1AxisTickPainter.md)



_Internal helper that computes and paints tick marks and labels for a single_ [_**Axis**_](classQAccelPlot_1_1Axis.md) _._[More...](#detailed-description)

* `#include <AxisTickPainter.hpp>`

















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**PaintContext**](structQAccelPlot_1_1AxisTickPainter_1_1PaintContext.md) <br>_Groups the QPainter context arguments passed into sub-painting helpers._  |
| struct | [**Params**](structQAccelPlot_1_1AxisTickPainter_1_1Params.md) <br>_All style inputs required for a single paint call, bundled to reduce parameter count._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef std::function&lt; qreal(qreal value, qreal length)&gt; | [**MapToPosition**](#typedef-maptoposition)  <br>_Function type that maps a data value to a pixel position along the axis._  |






















## Public Static Functions

| Type | Name |
| ---: | :--- |
|  qreal | [**computeNiceStep**](#function-computenicestep) (qreal viewportMin, qreal viewportMax, int tickCount) <br>_Returns a "nice" major tick step for the given_ _viewportMin_ _,__viewportMax_ _range and target__tickCount_ _._ |
|  [**AxisTicks**](structQAccelPlot_1_1AxisTicks.md) | [**computeTicks**](#function-computeticks) (qreal viewportMin, qreal viewportMax, bool logScale, const [**AxisTicker**](classQAccelPlot_1_1AxisTicker.md) \* ticker) <br>_Returns the visible ticks, subticks and formatted labels for the_ _viewportMin_ _to__viewportMax_ _range._ |
|  void | [**paintTicks**](#function-paintticks) (QPainter \* painter, const QRectF & rect, qreal axisX, qreal axisY, const [**Params**](structQAccelPlot_1_1AxisTickPainter_1_1Params.md) & params, const [**AxisTicks**](structQAccelPlot_1_1AxisTicks.md) & ticks, const [**MapToPosition**](classQAccelPlot_1_1AxisTickPainter.md#typedef-maptoposition) & mapToPosition) <br>_Paints_ _ticks_ _and their labels onto__painter_ _, placing them according to__params_ _and__mapToPosition_ _._ |
|  QSizeF | [**tickLabelSize**](#function-ticklabelsize) (const QFontMetricsF & metrics, const QString & label) <br>_Returns the size of the rectangle_ `paintTicks()` _lays out for__label_ _, measured with__metrics_ _._ |


























## Detailed Description


Tick computation and painting are split: `computeTicks()` formats labels and must run on the formatter's (GUI) thread, while `paintTicks()` only draws precomputed ticks and is safe to call from `Axis::paint()` on the render thread. Also provides `computeNiceStep()`, which is shared with `GridNode` to keep grid lines aligned with tick positions. 


    
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




### function computeTicks {#function-computeticks}

_Returns the visible ticks, subticks and formatted labels for the_ _viewportMin_ _to__viewportMax_ _range._
```C++
static AxisTicks QAccelPlot::AxisTickPainter::computeTicks (
    qreal viewportMin,
    qreal viewportMax,
    bool logScale,
    const AxisTicker * ticker
) 
```



Uses log-scale placement when _logScale_ is set and both bounds are positive. Labels are produced by _ticker's_ formatter, which may invoke a QML/JS callback, so this must be called on the formatter's thread. 


        

<hr>




### function paintTicks {#function-paintticks}

_Paints_ _ticks_ _and their labels onto__painter_ _, placing them according to__params_ _and__mapToPosition_ _._
```C++
static void QAccelPlot::AxisTickPainter::paintTicks (
    QPainter * painter,
    const QRectF & rect,
    qreal axisX,
    qreal axisY,
    const Params & params,
    const AxisTicks & ticks,
    const MapToPosition & mapToPosition
) 
```




<hr>




### function tickLabelSize {#function-ticklabelsize}

_Returns the size of the rectangle_ `paintTicks()` _lays out for__label_ _, measured with__metrics_ _._
```C++
static QSizeF QAccelPlot::AxisTickPainter::tickLabelSize (
    const QFontMetricsF & metrics,
    const QString & label
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/axis/AxisTickPainter.hpp`

