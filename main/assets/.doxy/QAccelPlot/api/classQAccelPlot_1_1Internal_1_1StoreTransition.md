








# Class QAccelPlot::Internal::StoreTransition



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**StoreTransition**](classQAccelPlot_1_1Internal_1_1StoreTransition.md)



_Shows the runs of a_ `TransitionRunner` _as the drawn data points of a_`DataPointStore` _._[More...](#detailed-description)

* `#include <StoreTransition.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**StoreTransition**](#function-storetransition) ([**TransitionRunner**](classQAccelPlot_1_1TransitionRunner.md) & runner, [**DataPointStore**](classQAccelPlot_1_1DataPointStore.md) & store) <br>_Constructs the transition for_ _runner_ _and__store_ _, which must outlive it._ |
|  bool | [**advance**](#function-advance) () <br>_Draws the next frame of the run, or the stored data points again once it has ended._  |
|  void | [**discard**](#function-discard) () <br>_Ends the run and draws the stored data points again._  |
|  bool | [**end**](#function-end) () <br>_Ends a run that draws other data points than the stored ones, drawing the stored ones again._  |
|  bool | [**start**](#function-start) (std::vector&lt; double &gt; && data, int count, const [**TransitionRunner::Layout**](classQAccelPlot_1_1TransitionRunner.md#typedef-layout) & layout) <br>_Stores the_ _count_ _double data points moved from__data_ _and starts a run to them from the drawn ones._ |




























## Detailed Description


The store's data points have no attributes. The drawn data points change on `advance()`, `end()`, and `discard()`, so the series refreshes what depends on them, such as its hover lookup, after each. 


    
## Public Functions Documentation





### function StoreTransition {#function-storetransition}

_Constructs the transition for_ _runner_ _and__store_ _, which must outlive it._
```C++
QAccelPlot::Internal::StoreTransition::StoreTransition (
    TransitionRunner & runner,
    DataPointStore & store
) 
```




<hr>




### function advance {#function-advance}

_Draws the next frame of the run, or the stored data points again once it has ended._ 
```C++
bool QAccelPlot::Internal::StoreTransition::advance () 
```





**Returns:**

`false` when the drawn data points did not change. 





        

<hr>




### function discard {#function-discard}

_Ends the run and draws the stored data points again._ 
```C++
void QAccelPlot::Internal::StoreTransition::discard () 
```




<hr>




### function end {#function-end}

_Ends a run that draws other data points than the stored ones, drawing the stored ones again._ 
```C++
bool QAccelPlot::Internal::StoreTransition::end () 
```





**Returns:**

`false` when the stored data points were drawn already. 





        

<hr>




### function start {#function-start}

_Stores the_ _count_ _double data points moved from__data_ _and starts a run to them from the drawn ones._
```C++
bool QAccelPlot::Internal::StoreTransition::start (
    std::vector< double > && data,
    int count,
    const TransitionRunner::Layout & layout
) 
```





**Returns:**

`false` when no run started; call `discard()` then. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/internal/StoreTransition.hpp`

