








# Struct QAccelPlot::Internal::TransitionSlots



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**TransitionSlots**](structQAccelPlot_1_1Internal_1_1TransitionSlots.md)



_Slots a data change animates._ 

* `#include <TransitionFrame.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**enter**](#variable-enter)   = `{false}`<br>_Added items run the enter transition._  |
|  bool | [**exit**](#variable-exit)   = `{false}`<br>_Removed items run the exit transition._  |
|  bool | [**update**](#variable-update)   = `{false}`<br>_Kept items, or all items when_ `redraw` _, run the update transition._ |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**any**](#function-any) () noexcept const<br>_Returns_ `true` _when some slot runs._ |




























## Public Attributes Documentation





### variable enter {#variable-enter}

_Added items run the enter transition._ 
```C++
bool QAccelPlot::Internal::TransitionSlots::enter;
```




<hr>




### variable exit {#variable-exit}

_Removed items run the exit transition._ 
```C++
bool QAccelPlot::Internal::TransitionSlots::exit;
```




<hr>




### variable update {#variable-update}

_Kept items, or all items when_ `redraw` _, run the update transition._
```C++
bool QAccelPlot::Internal::TransitionSlots::update;
```




<hr>
## Public Functions Documentation





### function any {#function-any}

_Returns_ `true` _when some slot runs._
```C++
inline bool QAccelPlot::Internal::TransitionSlots::any () noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/transitions/internal/TransitionFrame.hpp`

