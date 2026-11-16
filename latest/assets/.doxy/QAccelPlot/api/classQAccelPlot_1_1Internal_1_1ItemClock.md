








# Class QAccelPlot::Internal::ItemClock



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**ItemClock**](classQAccelPlot_1_1Internal_1_1ItemClock.md)



_The start time and movement of each item of one group at the frame._ 

* `#include <TransitionFrame.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**ItemClock**](#function-itemclock) (const [**TransitionStep**](structQAccelPlot_1_1Internal_1_1TransitionStep.md) & step, int itemCount, bool exit) <br>_Times a group of_ _itemCount_ _items under__step_ _; removed items when__exit_ _._ |
|  const [**FrontStarts**](classQAccelPlot_1_1Internal_1_1FrontStarts.md) \* | [**frontStarts**](#function-frontstarts) () noexcept const<br>_Returns where the front starts the items, or_ `nullptr` _without a front._ |
|  double | [**motion**](#function-motion) (int order) const<br>_Returns the progress of the movement of the item at position_ _order_ _, before easing._ |
|  double | [**start**](#function-start) (int order) const<br>_Returns the start time of the item at position_ _order_ _of the stagger order._ |
|  const [**ItemTiming**](structQAccelPlot_1_1Internal_1_1ItemTiming.md) & | [**timing**](#function-timing) () noexcept const<br>_Returns the timing of the group._  |




























## Public Functions Documentation





### function ItemClock {#function-itemclock}

_Times a group of_ _itemCount_ _items under__step_ _; removed items when__exit_ _._
```C++
QAccelPlot::Internal::ItemClock::ItemClock (
    const TransitionStep & step,
    int itemCount,
    bool exit
) 
```




<hr>




### function frontStarts {#function-frontstarts}

_Returns where the front starts the items, or_ `nullptr` _without a front._
```C++
const FrontStarts * QAccelPlot::Internal::ItemClock::frontStarts () noexcept const
```




<hr>




### function motion {#function-motion}

_Returns the progress of the movement of the item at position_ _order_ _, before easing._
```C++
double QAccelPlot::Internal::ItemClock::motion (
    int order
) const
```




<hr>




### function start {#function-start}

_Returns the start time of the item at position_ _order_ _of the stagger order._
```C++
double QAccelPlot::Internal::ItemClock::start (
    int order
) const
```




<hr>




### function timing {#function-timing}

_Returns the timing of the group._ 
```C++
const ItemTiming & QAccelPlot::Internal::ItemClock::timing () noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/transitions/internal/TransitionFrame.hpp`

