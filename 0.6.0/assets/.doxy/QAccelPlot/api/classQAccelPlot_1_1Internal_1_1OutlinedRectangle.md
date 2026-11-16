








# Class QAccelPlot::Internal::OutlinedRectangle



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**OutlinedRectangle**](classQAccelPlot_1_1Internal_1_1OutlinedRectangle.md)



_Item that fills its bounds and outlines them inside its edges._ 

* `#include <OutlinedRectangle.hpp>`



Inherits the following classes: QQuickItem


Inherited by the following classes: [QAccelPlot::Internal::RectangleZoomOverlay](classQAccelPlot_1_1Internal_1_1RectangleZoomOverlay.md)


## Inheritance diagram

```mermaid
flowchart TB
  classQAccelPlot_1_1Internal_1_1OutlinedRectangle["QAccelPlot::Internal::OutlinedRectangle"]

  external_base_classQAccelPlot_1_1Internal_1_1OutlinedRectangle_1["QQuickItem"]
  external_base_classQAccelPlot_1_1Internal_1_1OutlinedRectangle_1 --> classQAccelPlot_1_1Internal_1_1OutlinedRectangle

  classQAccelPlot_1_1Internal_1_1RectangleZoomOverlay["Internal::RectangleZoomOverlay"]
  classQAccelPlot_1_1Internal_1_1OutlinedRectangle --> classQAccelPlot_1_1Internal_1_1RectangleZoomOverlay
  click classQAccelPlot_1_1Internal_1_1RectangleZoomOverlay "../classQAccelPlot_1_1Internal_1_1RectangleZoomOverlay/" "Open Internal::RectangleZoomOverlay"

```
































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**OutlinedRectangle**](#function-outlinedrectangle) (QQuickItem \* parent=nullptr) <br> |
|  void | [**setStyle**](#function-setstyle) (const QColor & fill, const QColor & borderColor, qreal borderWidth) <br>_Sets the_ _fill_ _color and an outline of__borderColor_ _,__borderWidth_ _logical pixels wide._ |
























## Protected Functions

| Type | Name |
| ---: | :--- |
|  QSGNode \* | [**updatePaintNode**](#function-updatepaintnode) (QSGNode \* oldNode, UpdatePaintNodeData \*) override<br> |




## Public Functions Documentation





### function OutlinedRectangle {#function-outlinedrectangle}

```C++
explicit QAccelPlot::Internal::OutlinedRectangle::OutlinedRectangle (
    QQuickItem * parent=nullptr
) 
```




<hr>




### function setStyle {#function-setstyle}

_Sets the_ _fill_ _color and an outline of__borderColor_ _,__borderWidth_ _logical pixels wide._
```C++
void QAccelPlot::Internal::OutlinedRectangle::setStyle (
    const QColor & fill,
    const QColor & borderColor,
    qreal borderWidth
) 
```




<hr>
## Protected Functions Documentation





### function updatePaintNode {#function-updatepaintnode}

```C++
QSGNode * QAccelPlot::Internal::OutlinedRectangle::updatePaintNode (
    QSGNode * oldNode,
    UpdatePaintNodeData *
) override
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/internal/OutlinedRectangle.hpp`

