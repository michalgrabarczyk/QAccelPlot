








# Class QAccelPlot::PlotView



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**PlotView**](classQAccelPlot_1_1PlotView.md)



_The main plot canvas QML item — hosts axes, curves, and a grid._ [More...](#detailed-description)

* `#include <PlotView.hpp>`



Inherits the following classes: QQuickItem




## Inheritance diagram

```mermaid
flowchart TB
  classQAccelPlot_1_1PlotView["QAccelPlot::PlotView"]

  external_base_classQAccelPlot_1_1PlotView_1["QQuickItem"]
  external_base_classQAccelPlot_1_1PlotView_1 --> classQAccelPlot_1_1PlotView

```
























## Public Properties

| Type | Name |
| ---: | :--- |
| property QColor | [**axesAreaColor**](classQAccelPlot_1_1PlotView.md#property-axesareacolor-12)  <br>_Background color of the axes surround area. Default:_ `Colors.dark.axesArea` _._ |
| property [**Border**](classQAccelPlot_1_1Border.md) \* | [**border**](classQAccelPlot_1_1PlotView.md#property-border-12)  <br>_Frame inside the plot area's edges, e.g._ `border.width: 2` _. Default: transparent, width 0._ |
| property QQmlListProperty&lt; [**Axis**](classQAccelPlot_1_1Axis.md) &gt; | [**extraAxes**](classQAccelPlot_1_1PlotView.md#property-extraaxes-12)  <br>_Additional axes beyond the primary four; each extra axis must supply its own side._  |
| property [**Grid**](classQAccelPlot_1_1Grid.md) \* | [**grid**](classQAccelPlot_1_1PlotView.md#property-grid-12)  <br>_Read-only constant: grid configuration object._  |
| property QQuickItem \* | [**overlay**](classQAccelPlot_1_1PlotView.md#property-overlay-12)  <br>_Read-only constant: item that fills the plot and is stacked above the series, the zoom rectangle, and the_ `Plot` _legend._ |
| property qreal | [**padding**](classQAccelPlot_1_1PlotView.md#property-padding-12)  <br>_Uniform padding in pixels between the plot area and the canvas edge. Default: 24._  |
| property QColor | [**plotAreaColor**](classQAccelPlot_1_1PlotView.md#property-plotareacolor-12)  <br>_Background color of the plot data area. Default:_ `Colors.dark.plotArea` _._ |
| property QRectF | [**plotRect**](classQAccelPlot_1_1PlotView.md#property-plotrect-12)  <br>_Read-only: plot area rectangle in item-local pixel coordinates._  |
| property [**PlotRectangleZoom**](classQAccelPlot_1_1PlotRectangleZoom.md) \* | [**rectangleZoom**](classQAccelPlot_1_1PlotView.md#property-rectanglezoom-12)  <br>_Rectangle zoom configuration and selection state._  |
| property QList&lt; [**PlotSeries**](classQAccelPlot_1_1PlotSeries.md) \* &gt; | [**series**](classQAccelPlot_1_1PlotView.md#property-series-12)  <br>_Read-only: all registered plot series._  |
| property [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**x2Axis**](classQAccelPlot_1_1PlotView.md#property-x2axis-12)  <br>_Optional secondary horizontal axis; assigning it sets its side to_ `Axis.Top` _._ |
| property [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**xAxis**](classQAccelPlot_1_1PlotView.md#property-xaxis-12)  <br>_Primary horizontal axis; assigning it sets its side to_ `Axis.Bottom` _._ |
| property [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**y2Axis**](classQAccelPlot_1_1PlotView.md#property-y2axis-12)  <br>_Optional secondary vertical axis; assigning it sets its side to_ `Axis.Right` _._ |
| property [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**yAxis**](classQAccelPlot_1_1PlotView.md#property-yaxis-12)  <br>_Primary vertical axis; assigning it sets its side to_ `Axis.Left` _._ |




## Public Signals

| Type | Name |
| ---: | :--- |
| signal void | [**axesAreaColorChanged**](classQAccelPlot_1_1PlotView.md#signal-axesareacolorchanged)  <br>_Emitted when the axesAreaColor property changes._  |
| signal void | [**escapePressed**](classQAccelPlot_1_1PlotView.md#signal-escapepressed)  <br>_Emitted when Escape is pressed while the plot has focus, so that tools can cancel their gestures._  |
| signal void | [**mouseDoubleClicked**](classQAccelPlot_1_1PlotView.md#signal-mousedoubleclicked) ([**PlotMouseEvent**](classQAccelPlot_1_1PlotMouseEvent.md) \* event) <br>_Emitted when a mouse button is double-clicked over the plot. Call_ `event->accept()` _to consume the event and suppress built-in handling (rescale all axes)._ |
| signal void | [**mouseMoved**](classQAccelPlot_1_1PlotView.md#signal-mousemoved) ([**PlotMouseEvent**](classQAccelPlot_1_1PlotMouseEvent.md) \* event) <br>_Emitted when the mouse is moved over the plot. Call_ `event->accept()` _to consume the event and suppress built-in handling (panning or selection updates)._ |
| signal void | [**mousePressed**](classQAccelPlot_1_1PlotView.md#signal-mousepressed) ([**PlotMouseEvent**](classQAccelPlot_1_1PlotMouseEvent.md) \* event) <br>_Emitted when a mouse button is pressed over the plot. Call_ `event->accept()` _to consume the event and suppress built-in handling (drag start)._ |
| signal void | [**mouseReleased**](classQAccelPlot_1_1PlotView.md#signal-mousereleased) ([**PlotMouseEvent**](classQAccelPlot_1_1PlotMouseEvent.md) \* event) <br>_Emitted when a mouse button is released over the plot. Call_ `event->accept()` _to consume further built-in handling. A left-button release always ends an active drag._ |
| signal void | [**paddingChanged**](classQAccelPlot_1_1PlotView.md#signal-paddingchanged)  <br>_Emitted when the padding property changes._  |
| signal void | [**plotAreaColorChanged**](classQAccelPlot_1_1PlotView.md#signal-plotareacolorchanged)  <br>_Emitted when the plotAreaColor property changes._  |
| signal void | [**plotRectChanged**](classQAccelPlot_1_1PlotView.md#signal-plotrectchanged)  <br>_Emitted when the plotRect changes (axis layout recalculated)._  |
| signal void | [**pointerGrabLost**](classQAccelPlot_1_1PlotView.md#signal-pointergrablost)  <br>_Emitted when the plot loses its mouse grab during a drag._  |
| signal void | [**seriesChanged**](classQAccelPlot_1_1PlotView.md#signal-serieschanged)  <br>_Emitted when the set of registered plot series changes._  |
| signal void | [**x2AxisChanged**](classQAccelPlot_1_1PlotView.md#signal-x2axischanged)  <br>_Emitted when the x2Axis property changes._  |
| signal void | [**xAxisChanged**](classQAccelPlot_1_1PlotView.md#signal-xaxischanged)  <br>_Emitted when the xAxis property changes._  |
| signal void | [**y2AxisChanged**](classQAccelPlot_1_1PlotView.md#signal-y2axischanged)  <br>_Emitted when the y2Axis property changes._  |
| signal void | [**yAxisChanged**](classQAccelPlot_1_1PlotView.md#signal-yaxischanged)  <br>_Emitted when the yAxis property changes._  |




## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**PlotView**](#function-plotview) (QQuickItem \* parent=nullptr) <br>_Constructs a_ [_**PlotView**_](classQAccelPlot_1_1PlotView.md) _with the given__parent_ _._ |
|  QColor | [**axesAreaColor**](#function-axesareacolor-22) () const<br>_Returns the axes surround background color._  |
|  [**Border**](classQAccelPlot_1_1Border.md) \* | [**border**](#function-border-22) () const<br>_Returns the plot-area frame settings. The object is owned by the plot._  |
|  Q\_INVOKABLE qreal | [**dataToPixelX**](#function-datatopixelx) (qreal dataValue) const<br>_Converts a horizontal data-space value to an item-local pixel X coordinate._  |
|  Q\_INVOKABLE qreal | [**dataToPixelY**](#function-datatopixely) (qreal dataValue) const<br>_Converts a vertical data-space value to an item-local pixel Y coordinate._  |
|  QQmlListProperty&lt; [**Axis**](classQAccelPlot_1_1Axis.md) &gt; | [**extraAxes**](#function-extraaxes-22) () <br>_Returns the QML list property for extra axes._  |
|  [**Grid**](classQAccelPlot_1_1Grid.md) \* | [**grid**](#function-grid-22) () const<br>_Returns the grid configuration object._  |
|  Q\_INVOKABLE bool | [**isInsidePlotArea**](#function-isinsideplotarea) (qreal x, qreal y) const<br>_Returns_ `true` _if the item-local point (__x_ _,__y_ _) lies inside the plot area._ |
|  QQuickItem \* | [**overlay**](#function-overlay-22) () const<br>_Returns the item that holds overlays drawn above the series._  |
|  qreal | [**padding**](#function-padding-22) () const<br>_Returns the uniform canvas padding in pixels._  |
|  Q\_INVOKABLE qreal | [**pixelToDataX**](#function-pixeltodatax) (qreal pixelX) const<br>_Converts an item-local pixel X coordinate to a horizontal data-space value._  |
|  Q\_INVOKABLE qreal | [**pixelToDataY**](#function-pixeltodatay) (qreal pixelY) const<br>_Converts an item-local pixel Y coordinate to a vertical data-space value._  |
|  QColor | [**plotAreaColor**](#function-plotareacolor-22) () const<br>_Returns the plot area background color._  |
|  QRectF | [**plotRect**](#function-plotrect-22) () const<br>_Returns the current plot area rectangle in item-local coordinates._  |
|  [**PlotRectangleZoom**](classQAccelPlot_1_1PlotRectangleZoom.md) \* | [**rectangleZoom**](#function-rectanglezoom-22) () const<br>_Returns the rectangle zoom configuration object._  |
|  Q\_INVOKABLE void | [**rescaleAllAxes**](#function-rescaleallaxes) () <br>_Calls_ `Axis::rescaleToData()` _on all attached axes._ |
|  QList&lt; [**PlotSeries**](classQAccelPlot_1_1PlotSeries.md) \* &gt; | [**series**](#function-series-22) () const<br>_Returns all plot-series children currently registered with this canvas._  |
|  void | [**setAxesAreaColor**](#function-setaxesareacolor) (const QColor & c) <br>_Sets the axes surround background color to_ _c_ _._ |
|  void | [**setPadding**](#function-setpadding) (qreal p) <br>_Sets the canvas padding to_ _p_ _pixels._ |
|  void | [**setPlotAreaColor**](#function-setplotareacolor) (const QColor & c) <br>_Sets the plot area background color to_ _c_ _._ |
|  void | [**setX2Axis**](#function-setx2axis) ([**Axis**](classQAccelPlot_1_1Axis.md) \* axis) <br>_Sets the secondary horizontal axis to_ _axis_ _and assigns it to the top side._ |
|  void | [**setXAxis**](#function-setxaxis) ([**Axis**](classQAccelPlot_1_1Axis.md) \* axis) <br>_Sets the primary horizontal axis to_ _axis_ _and assigns it to the bottom side._ |
|  void | [**setY2Axis**](#function-sety2axis) ([**Axis**](classQAccelPlot_1_1Axis.md) \* axis) <br>_Sets the secondary vertical axis to_ _axis_ _and assigns it to the right side._ |
|  void | [**setYAxis**](#function-setyaxis) ([**Axis**](classQAccelPlot_1_1Axis.md) \* axis) <br>_Sets the primary vertical axis to_ _axis_ _and assigns it to the left side._ |
|  [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**x2Axis**](#function-x2axis-22) () const<br>_Returns the secondary horizontal axis, or_ `nullptr` _if not set._ |
|  [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**xAxis**](#function-xaxis-22) () const<br>_Returns the primary horizontal axis._  |
|  [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**y2Axis**](#function-y2axis-22) () const<br>_Returns the secondary vertical axis, or_ `nullptr` _if not set._ |
|  [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**yAxis**](#function-yaxis-22) () const<br>_Returns the primary vertical axis._  |
|  Q\_INVOKABLE bool | [**zoomToRect**](#function-zoomtorect) (const QRectF & rect) <br>_Zooms all attached axes to an item-local pixel rectangle and returns whether it was applied. Normalizes and clips the rectangle to plotRect; rejects selections smaller than rectangleZoom.minimumSize or ranges that are nonfinite, collapsed, or nonpositive on logarithmic axes. Does not require enabled._  |
|   | [**~PlotView**](#function-plotview) () override<br>_Destroys the plot after disconnecting attached axis signals._  |




























## Detailed Description


[**PlotView**](classQAccelPlot_1_1PlotView.md) manages the layout of up to four axes (X, Y, X2, Y2) plus any number of additional axes, and renders `LineCurve` children via the Qt Scene Graph. Data-to-pixel and pixel-to-data conversion helpers are exposed as invokable methods so overlay items can align themselves to the plot coordinate system.




**
**


```C++
PlotView {
    Axis { id: xAxis }
    Axis { id: yAxis }
    xAxis: xAxis
    yAxis: yAxis
    LineCurve { xAxis: xAxis; yAxis: yAxis; color: "steelblue" }
}
```





**See also:** [**Axis**](classQAccelPlot_1_1Axis.md), [**LineCurve**](classQAccelPlot_1_1LineCurve.md), [**Grid**](classQAccelPlot_1_1Grid.md) 



    
## Public Properties Documentation





### property axesAreaColor {#property-axesareacolor-12}

_Background color of the axes surround area. Default:_ `Colors.dark.axesArea` _._
```C++
QColor QAccelPlot::PlotView::axesAreaColor;
```




<hr>




### property border {#property-border-12}

_Frame inside the plot area's edges, e.g._ `border.width: 2` _. Default: transparent, width 0._
```C++
Border* QAccelPlot::PlotView::border;
```




<hr>




### property extraAxes {#property-extraaxes-12}

_Additional axes beyond the primary four; each extra axis must supply its own side._ 
```C++
QQmlListProperty<Axis> QAccelPlot::PlotView::extraAxes;
```




<hr>




### property grid {#property-grid-12}

_Read-only constant: grid configuration object._ 
```C++
Grid* QAccelPlot::PlotView::grid;
```




<hr>




### property overlay {#property-overlay-12}

_Read-only constant: item that fills the plot and is stacked above the series, the zoom rectangle, and the_ `Plot` _legend._
```C++
QQuickItem* QAccelPlot::PlotView::overlay;
```



`Crosshair`, `InspectionMarkers`, `InspectionTooltip`, and the rectangle of a `SelectionTool` are its children. 


        

<hr>




### property padding {#property-padding-12}

_Uniform padding in pixels between the plot area and the canvas edge. Default: 24._ 
```C++
qreal QAccelPlot::PlotView::padding;
```




<hr>




### property plotAreaColor {#property-plotareacolor-12}

_Background color of the plot data area. Default:_ `Colors.dark.plotArea` _._
```C++
QColor QAccelPlot::PlotView::plotAreaColor;
```




<hr>




### property plotRect {#property-plotrect-12}

_Read-only: plot area rectangle in item-local pixel coordinates._ 
```C++
QRectF QAccelPlot::PlotView::plotRect;
```




<hr>




### property rectangleZoom {#property-rectanglezoom-12}

_Rectangle zoom configuration and selection state._ 
```C++
PlotRectangleZoom* QAccelPlot::PlotView::rectangleZoom;
```




<hr>




### property series {#property-series-12}

_Read-only: all registered plot series._ 
```C++
QList<PlotSeries*> QAccelPlot::PlotView::series;
```




<hr>




### property x2Axis {#property-x2axis-12}

_Optional secondary horizontal axis; assigning it sets its side to_ `Axis.Top` _._
```C++
Axis* QAccelPlot::PlotView::x2Axis;
```




<hr>




### property xAxis {#property-xaxis-12}

_Primary horizontal axis; assigning it sets its side to_ `Axis.Bottom` _._
```C++
Axis* QAccelPlot::PlotView::xAxis;
```




<hr>




### property y2Axis {#property-y2axis-12}

_Optional secondary vertical axis; assigning it sets its side to_ `Axis.Right` _._
```C++
Axis* QAccelPlot::PlotView::y2Axis;
```




<hr>




### property yAxis {#property-yaxis-12}

_Primary vertical axis; assigning it sets its side to_ `Axis.Left` _._
```C++
Axis* QAccelPlot::PlotView::yAxis;
```




<hr>
## Public Signals Documentation





### signal axesAreaColorChanged {#signal-axesareacolorchanged}

_Emitted when the axesAreaColor property changes._ 
```C++
void QAccelPlot::PlotView::axesAreaColorChanged;
```




<hr>




### signal escapePressed {#signal-escapepressed}

_Emitted when Escape is pressed while the plot has focus, so that tools can cancel their gestures._ 
```C++
void QAccelPlot::PlotView::escapePressed;
```




<hr>




### signal mouseDoubleClicked {#signal-mousedoubleclicked}

_Emitted when a mouse button is double-clicked over the plot. Call_ `event->accept()` _to consume the event and suppress built-in handling (rescale all axes)._
```C++
void QAccelPlot::PlotView::mouseDoubleClicked;
```




<hr>




### signal mouseMoved {#signal-mousemoved}

_Emitted when the mouse is moved over the plot. Call_ `event->accept()` _to consume the event and suppress built-in handling (panning or selection updates)._
```C++
void QAccelPlot::PlotView::mouseMoved;
```




<hr>




### signal mousePressed {#signal-mousepressed}

_Emitted when a mouse button is pressed over the plot. Call_ `event->accept()` _to consume the event and suppress built-in handling (drag start)._
```C++
void QAccelPlot::PlotView::mousePressed;
```




<hr>




### signal mouseReleased {#signal-mousereleased}

_Emitted when a mouse button is released over the plot. Call_ `event->accept()` _to consume further built-in handling. A left-button release always ends an active drag._
```C++
void QAccelPlot::PlotView::mouseReleased;
```




<hr>




### signal paddingChanged {#signal-paddingchanged}

_Emitted when the padding property changes._ 
```C++
void QAccelPlot::PlotView::paddingChanged;
```




<hr>




### signal plotAreaColorChanged {#signal-plotareacolorchanged}

_Emitted when the plotAreaColor property changes._ 
```C++
void QAccelPlot::PlotView::plotAreaColorChanged;
```




<hr>




### signal plotRectChanged {#signal-plotrectchanged}

_Emitted when the plotRect changes (axis layout recalculated)._ 
```C++
void QAccelPlot::PlotView::plotRectChanged;
```




<hr>




### signal pointerGrabLost {#signal-pointergrablost}

_Emitted when the plot loses its mouse grab during a drag._ 
```C++
void QAccelPlot::PlotView::pointerGrabLost;
```




<hr>




### signal seriesChanged {#signal-serieschanged}

_Emitted when the set of registered plot series changes._ 
```C++
void QAccelPlot::PlotView::seriesChanged;
```




<hr>




### signal x2AxisChanged {#signal-x2axischanged}

_Emitted when the x2Axis property changes._ 
```C++
void QAccelPlot::PlotView::x2AxisChanged;
```




<hr>




### signal xAxisChanged {#signal-xaxischanged}

_Emitted when the xAxis property changes._ 
```C++
void QAccelPlot::PlotView::xAxisChanged;
```




<hr>




### signal y2AxisChanged {#signal-y2axischanged}

_Emitted when the y2Axis property changes._ 
```C++
void QAccelPlot::PlotView::y2AxisChanged;
```




<hr>




### signal yAxisChanged {#signal-yaxischanged}

_Emitted when the yAxis property changes._ 
```C++
void QAccelPlot::PlotView::yAxisChanged;
```




<hr>
## Public Functions Documentation





### function PlotView {#function-plotview}

_Constructs a_ [_**PlotView**_](classQAccelPlot_1_1PlotView.md) _with the given__parent_ _._
```C++
explicit QAccelPlot::PlotView::PlotView (
    QQuickItem * parent=nullptr
) 
```




<hr>




### function axesAreaColor {#function-axesareacolor-22}

_Returns the axes surround background color._ 
```C++
QColor QAccelPlot::PlotView::axesAreaColor () const
```




<hr>




### function border {#function-border-22}

_Returns the plot-area frame settings. The object is owned by the plot._ 
```C++
Border * QAccelPlot::PlotView::border () const
```




<hr>




### function dataToPixelX {#function-datatopixelx}

_Converts a horizontal data-space value to an item-local pixel X coordinate._ 
```C++
Q_INVOKABLE qreal QAccelPlot::PlotView::dataToPixelX (
    qreal dataValue
) const
```




<hr>




### function dataToPixelY {#function-datatopixely}

_Converts a vertical data-space value to an item-local pixel Y coordinate._ 
```C++
Q_INVOKABLE qreal QAccelPlot::PlotView::dataToPixelY (
    qreal dataValue
) const
```




<hr>




### function extraAxes {#function-extraaxes-22}

_Returns the QML list property for extra axes._ 
```C++
QQmlListProperty< Axis > QAccelPlot::PlotView::extraAxes () 
```




<hr>




### function grid {#function-grid-22}

_Returns the grid configuration object._ 
```C++
Grid * QAccelPlot::PlotView::grid () const
```




<hr>




### function isInsidePlotArea {#function-isinsideplotarea}

_Returns_ `true` _if the item-local point (__x_ _,__y_ _) lies inside the plot area._
```C++
Q_INVOKABLE bool QAccelPlot::PlotView::isInsidePlotArea (
    qreal x,
    qreal y
) const
```




<hr>




### function overlay {#function-overlay-22}

_Returns the item that holds overlays drawn above the series._ 
```C++
QQuickItem * QAccelPlot::PlotView::overlay () const
```




<hr>




### function padding {#function-padding-22}

_Returns the uniform canvas padding in pixels._ 
```C++
qreal QAccelPlot::PlotView::padding () const
```




<hr>




### function pixelToDataX {#function-pixeltodatax}

_Converts an item-local pixel X coordinate to a horizontal data-space value._ 
```C++
Q_INVOKABLE qreal QAccelPlot::PlotView::pixelToDataX (
    qreal pixelX
) const
```




<hr>




### function pixelToDataY {#function-pixeltodatay}

_Converts an item-local pixel Y coordinate to a vertical data-space value._ 
```C++
Q_INVOKABLE qreal QAccelPlot::PlotView::pixelToDataY (
    qreal pixelY
) const
```




<hr>




### function plotAreaColor {#function-plotareacolor-22}

_Returns the plot area background color._ 
```C++
QColor QAccelPlot::PlotView::plotAreaColor () const
```




<hr>




### function plotRect {#function-plotrect-22}

_Returns the current plot area rectangle in item-local coordinates._ 
```C++
QRectF QAccelPlot::PlotView::plotRect () const
```




<hr>




### function rectangleZoom {#function-rectanglezoom-22}

_Returns the rectangle zoom configuration object._ 
```C++
PlotRectangleZoom * QAccelPlot::PlotView::rectangleZoom () const
```




<hr>




### function rescaleAllAxes {#function-rescaleallaxes}

_Calls_ `Axis::rescaleToData()` _on all attached axes._
```C++
Q_INVOKABLE void QAccelPlot::PlotView::rescaleAllAxes () 
```




<hr>




### function series {#function-series-22}

_Returns all plot-series children currently registered with this canvas._ 
```C++
QList< PlotSeries * > QAccelPlot::PlotView::series () const
```




<hr>




### function setAxesAreaColor {#function-setaxesareacolor}

_Sets the axes surround background color to_ _c_ _._
```C++
void QAccelPlot::PlotView::setAxesAreaColor (
    const QColor & c
) 
```




<hr>




### function setPadding {#function-setpadding}

_Sets the canvas padding to_ _p_ _pixels._
```C++
void QAccelPlot::PlotView::setPadding (
    qreal p
) 
```




<hr>




### function setPlotAreaColor {#function-setplotareacolor}

_Sets the plot area background color to_ _c_ _._
```C++
void QAccelPlot::PlotView::setPlotAreaColor (
    const QColor & c
) 
```




<hr>




### function setX2Axis {#function-setx2axis}

_Sets the secondary horizontal axis to_ _axis_ _and assigns it to the top side._
```C++
void QAccelPlot::PlotView::setX2Axis (
    Axis * axis
) 
```




<hr>




### function setXAxis {#function-setxaxis}

_Sets the primary horizontal axis to_ _axis_ _and assigns it to the bottom side._
```C++
void QAccelPlot::PlotView::setXAxis (
    Axis * axis
) 
```




<hr>




### function setY2Axis {#function-sety2axis}

_Sets the secondary vertical axis to_ _axis_ _and assigns it to the right side._
```C++
void QAccelPlot::PlotView::setY2Axis (
    Axis * axis
) 
```




<hr>




### function setYAxis {#function-setyaxis}

_Sets the primary vertical axis to_ _axis_ _and assigns it to the left side._
```C++
void QAccelPlot::PlotView::setYAxis (
    Axis * axis
) 
```




<hr>




### function x2Axis {#function-x2axis-22}

_Returns the secondary horizontal axis, or_ `nullptr` _if not set._
```C++
Axis * QAccelPlot::PlotView::x2Axis () const
```




<hr>




### function xAxis {#function-xaxis-22}

_Returns the primary horizontal axis._ 
```C++
Axis * QAccelPlot::PlotView::xAxis () const
```




<hr>




### function y2Axis {#function-y2axis-22}

_Returns the secondary vertical axis, or_ `nullptr` _if not set._
```C++
Axis * QAccelPlot::PlotView::y2Axis () const
```




<hr>




### function yAxis {#function-yaxis-22}

_Returns the primary vertical axis._ 
```C++
Axis * QAccelPlot::PlotView::yAxis () const
```




<hr>




### function zoomToRect {#function-zoomtorect}

_Zooms all attached axes to an item-local pixel rectangle and returns whether it was applied. Normalizes and clips the rectangle to plotRect; rejects selections smaller than rectangleZoom.minimumSize or ranges that are nonfinite, collapsed, or nonpositive on logarithmic axes. Does not require enabled._ 
```C++
Q_INVOKABLE bool QAccelPlot::PlotView::zoomToRect (
    const QRectF & rect
) 
```




<hr>




### function ~PlotView {#function-plotview}

_Destroys the plot after disconnecting attached axis signals._ 
```C++
QAccelPlot::PlotView::~PlotView () override
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/PlotView.hpp`

