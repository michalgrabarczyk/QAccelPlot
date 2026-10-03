








# Class QAccelPlot::OverlayChildren



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**OverlayChildren**](classQAccelPlot_1_1OverlayChildren.md)



_Objects declared inside an inspection tool in QML._ [More...](#detailed-description)

* `#include <OverlayChildren.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**OverlayChildren**](#function-overlaychildren) (QObject & owner, QVariant tool, const char \* toolProperty) <br>_tool_ _holds__owner_ _and is written to the property__toolProperty_ _of every object._ |
|  QQmlListProperty&lt; QObject &gt; | [**list**](#function-list) () <br>_Returns the list property that appends to these objects._  |
|  void | [**setOverlay**](#function-setoverlay) (QQuickItem \* overlay) <br>_Sets the plot overlay whose children are stacked._  |




























## Detailed Description


Every object that has the tool's property receives the tool. Items among them that are children of the plot's overlay are kept stacked in declaration order above its other children. 


    
## Public Functions Documentation





### function OverlayChildren {#function-overlaychildren}

_tool_ _holds__owner_ _and is written to the property__toolProperty_ _of every object._
```C++
QAccelPlot::OverlayChildren::OverlayChildren (
    QObject & owner,
    QVariant tool,
    const char * toolProperty
) 
```




<hr>




### function list {#function-list}

_Returns the list property that appends to these objects._ 
```C++
QQmlListProperty< QObject > QAccelPlot::OverlayChildren::list () 
```




<hr>




### function setOverlay {#function-setoverlay}

_Sets the plot overlay whose children are stacked._ 
```C++
void QAccelPlot::OverlayChildren::setOverlay (
    QQuickItem * overlay
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/internal/OverlayChildren.hpp`

