








# Class QAccelPlot::Internal::RectangleZoomOverlay



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**RectangleZoomOverlay**](classQAccelPlot_1_1Internal_1_1RectangleZoomOverlay.md)








Inherits the following classes: [QAccelPlot::Internal::OutlinedRectangle](classQAccelPlot_1_1Internal_1_1OutlinedRectangle.md)




## Inheritance diagram

```mermaid
flowchart TB
  classQAccelPlot_1_1Internal_1_1RectangleZoomOverlay["QAccelPlot::Internal::RectangleZoomOverlay"]

  classQAccelPlot_1_1Internal_1_1OutlinedRectangle["Internal::OutlinedRectangle"]
  classQAccelPlot_1_1Internal_1_1OutlinedRectangle --> classQAccelPlot_1_1Internal_1_1RectangleZoomOverlay
  click classQAccelPlot_1_1Internal_1_1OutlinedRectangle "../classQAccelPlot_1_1Internal_1_1OutlinedRectangle/" "Open Internal::OutlinedRectangle"

  external_base_classQAccelPlot_1_1Internal_1_1OutlinedRectangle_1["QQuickItem"]
  external_base_classQAccelPlot_1_1Internal_1_1OutlinedRectangle_1 --> classQAccelPlot_1_1Internal_1_1OutlinedRectangle

```




















































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**RectangleZoomOverlay**](#function-rectanglezoomoverlay) (QQuickItem \* parent, [**PlotRectangleZoom**](classQAccelPlot_1_1PlotRectangleZoom.md) \* configuration) <br> |


## Public Functions inherited from QAccelPlot::Internal::OutlinedRectangle

See [QAccelPlot::Internal::OutlinedRectangle](classQAccelPlot_1_1Internal_1_1OutlinedRectangle.md)

| Type | Name |
| ---: | :--- |
|   | [**OutlinedRectangle**](classQAccelPlot_1_1Internal_1_1OutlinedRectangle.md#function-outlinedrectangle) (QQuickItem \* parent=nullptr) <br> |
|  void | [**setStyle**](classQAccelPlot_1_1Internal_1_1OutlinedRectangle.md#function-setstyle) (const QColor & fill, const QColor & borderColor, qreal borderWidth) <br>_Sets the_ _fill_ _color and an outline of__borderColor_ _,__borderWidth_ _logical pixels wide._ |
















































## Protected Functions inherited from QAccelPlot::Internal::OutlinedRectangle

See [QAccelPlot::Internal::OutlinedRectangle](classQAccelPlot_1_1Internal_1_1OutlinedRectangle.md)

| Type | Name |
| ---: | :--- |
|  QSGNode \* | [**updatePaintNode**](classQAccelPlot_1_1Internal_1_1OutlinedRectangle.md#function-updatepaintnode) (QSGNode \* oldNode, UpdatePaintNodeData \*) override<br> |






## Public Functions Documentation





### function RectangleZoomOverlay {#function-rectanglezoomoverlay}

```C++
QAccelPlot::Internal::RectangleZoomOverlay::RectangleZoomOverlay (
    QQuickItem * parent,
    PlotRectangleZoom * configuration
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/internal/RectangleZoomOverlay.hpp`

