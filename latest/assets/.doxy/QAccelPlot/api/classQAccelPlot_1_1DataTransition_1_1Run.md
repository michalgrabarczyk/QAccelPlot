








# Class QAccelPlot::DataTransition::Run



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**DataTransition**](classQAccelPlot_1_1DataTransition.md) **>** [**Run**](classQAccelPlot_1_1DataTransition_1_1Run.md)



_One animation of a transition on one series._ [More...](#detailed-description)

* `#include <DataTransition.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Run**](#function-run-12) () = default<br>_Constructs an idle run._  |
|   | [**Run**](#function-run-22) (const Run &) = delete<br> |
|  bool | [**active**](#function-active) () const<br>_Returns_ `true` _while the run is active._ |
|  void | [**end**](#function-end) () <br>_Ends the run._  |
|  [**Run**](classQAccelPlot_1_1DataTransition_1_1Run.md#function-run-12) & | [**operator=**](#function-operator) (const [**Run**](classQAccelPlot_1_1DataTransition_1_1Run.md#function-run-12) &) = delete<br> |
|  DataTransition \* | [**transition**](#function-transition) () const<br>_Returns the transition the run is active on, or_ `nullptr` _._ |
|   | [**~Run**](#function-run) () <br>_Destroys the run, ending it on its transition._  |




























## Detailed Description


A run is _active_ from `DataTransition::start()` until `end()`, `DataTransition::cancel()`, or the destruction of the transition. Destroying a run ends it. 


    
## Public Functions Documentation





### function Run {#function-run-12}

_Constructs an idle run._ 
```C++
QAccelPlot::DataTransition::Run::Run () = default
```




<hr>




### function Run {#function-run-22}

```C++
QAccelPlot::DataTransition::Run::Run (
    const Run &
) = delete
```




<hr>




### function active {#function-active}

_Returns_ `true` _while the run is active._
```C++
bool QAccelPlot::DataTransition::Run::active () const
```




<hr>




### function end {#function-end}

_Ends the run._ 
```C++
void QAccelPlot::DataTransition::Run::end () 
```




<hr>




### function operator= {#function-operator}

```C++
Run & QAccelPlot::DataTransition::Run::operator= (
    const Run &
) = delete
```




<hr>




### function transition {#function-transition}

_Returns the transition the run is active on, or_ `nullptr` _._
```C++
DataTransition * QAccelPlot::DataTransition::Run::transition () const
```




<hr>




### function ~Run {#function-run}

_Destroys the run, ending it on its transition._ 
```C++
QAccelPlot::DataTransition::Run::~Run () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/transitions/DataTransition.hpp`

