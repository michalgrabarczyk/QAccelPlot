








# Class QAccelPlot::PlotDragRect



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**PlotDragRect**](classQAccelPlot_1_1PlotDragRect.md)



_Rectangle dragged out inside the plot area, shared by rectangle zoom and data selection._ 

* `#include <PlotDragRect.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**active**](#function-active) () const<br>_Returns true between_ `begin()` _and_`end()` _._ |
|  void | [**begin**](#function-begin) (const QPointF & position) <br>_Starts a drag at_ _position_ _._ |
|  void | [**end**](#function-end) () <br>_Ends the drag._  |
|  void | [**moveTo**](#function-moveto) (const QPointF & position, const QRectF & bounds) <br>_Moves the free corner to_ _position_ _, clamped to__bounds_ _._ |
|  QRectF | [**rect**](#function-rect) () const<br>_Returns the normalized rectangle between the pressed and the free corner._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  bool | [**meetsMinimum**](#function-meetsminimum) (const QRectF & rect, qreal minimumSize, bool checkWidth=true, bool checkHeight=true) <br>_Returns true when_ _rect_ _is at least__minimumSize_ _in every checked dimension._ |
|  bool | [**modifiersMatch**](#function-modifiersmatch) (int pressed, int required) <br>_Returns true when the pressed modifiers are exactly the_ _required_ _ones._ |


























## Public Functions Documentation





### function active {#function-active}

_Returns true between_ `begin()` _and_`end()` _._
```C++
bool QAccelPlot::PlotDragRect::active () const
```




<hr>




### function begin {#function-begin}

_Starts a drag at_ _position_ _._
```C++
void QAccelPlot::PlotDragRect::begin (
    const QPointF & position
) 
```




<hr>




### function end {#function-end}

_Ends the drag._ 
```C++
void QAccelPlot::PlotDragRect::end () 
```




<hr>




### function moveTo {#function-moveto}

_Moves the free corner to_ _position_ _, clamped to__bounds_ _._
```C++
void QAccelPlot::PlotDragRect::moveTo (
    const QPointF & position,
    const QRectF & bounds
) 
```




<hr>




### function rect {#function-rect}

_Returns the normalized rectangle between the pressed and the free corner._ 
```C++
QRectF QAccelPlot::PlotDragRect::rect () const
```




<hr>
## Public Static Functions Documentation





### function meetsMinimum {#function-meetsminimum}

_Returns true when_ _rect_ _is at least__minimumSize_ _in every checked dimension._
```C++
static bool QAccelPlot::PlotDragRect::meetsMinimum (
    const QRectF & rect,
    qreal minimumSize,
    bool checkWidth=true,
    bool checkHeight=true
) 
```




<hr>




### function modifiersMatch {#function-modifiersmatch}

_Returns true when the pressed modifiers are exactly the_ _required_ _ones._
```C++
static bool QAccelPlot::PlotDragRect::modifiersMatch (
    int pressed,
    int required
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/PlotDragRect.hpp`

