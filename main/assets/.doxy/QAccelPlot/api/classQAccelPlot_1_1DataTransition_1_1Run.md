








# Class QAccelPlot::DataTransition::Run



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**DataTransition**](classQAccelPlot_1_1DataTransition.md) **>** [**Run**](classQAccelPlot_1_1DataTransition_1_1Run.md)



_One animation of a transition on one host element._ [More...](#detailed-description)

* `#include <DataTransition.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Run**](#function-run-12) () = default<br>_Constructs an idle run._  |
|   | [**Run**](#function-run-22) (const [**Run**](classQAccelPlot_1_1DataTransition_1_1Run.md) &) = delete<br> |
|  bool | [**active**](#function-active) () const<br>_Returns_ `true` _while the transition advances this run._ |
|  void | [**cancel**](#function-cancel) () <br>_Ends the run and discards its data._  |
|  bool | [**finish**](#function-finish) (std::vector&lt; double &gt; & outData, int & outPointCount) <br>_Ends the run and moves its target data into_ _outData_ _and__outPointCount_ _._ |
|  [**Run**](classQAccelPlot_1_1DataTransition_1_1Run.md) & | [**operator=**](#function-operator) (const [**Run**](classQAccelPlot_1_1DataTransition_1_1Run.md) &) = delete<br> |
|  bool | [**pending**](#function-pending) () const<br>_Returns_ `true` _while the run holds target data that_`finish()` _has not taken._ |
|  const std::vector&lt; double &gt; & | [**targetData**](#function-targetdata) () const<br>_Returns the target data. Valid while_ `pending()` _is_`true` _._ |
|  int | [**targetPointCount**](#function-targetpointcount) () const<br>_Returns the number of points in_ `targetData()` _._ |
|   | [**~Run**](#function-run) () <br>_Destroys the run, ending it on its transition._  |




























## Detailed Description


Holds the data the host animates from and to. A run is _active_ while the transition advances it, and _pending_ until the host takes the target data with `finish()`. A run ended by `DataTransition::cancel()` or by destroying the transition stays pending, so the host can still show the target data. Destroying a run ends it. 


    
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

_Returns_ `true` _while the transition advances this run._
```C++
bool QAccelPlot::DataTransition::Run::active () const
```




<hr>




### function cancel {#function-cancel}

_Ends the run and discards its data._ 
```C++
void QAccelPlot::DataTransition::Run::cancel () 
```




<hr>




### function finish {#function-finish}

_Ends the run and moves its target data into_ _outData_ _and__outPointCount_ _._
```C++
bool QAccelPlot::DataTransition::Run::finish (
    std::vector< double > & outData,
    int & outPointCount
) 
```





**Returns:**

`false`, leaving the outputs unchanged, when the run is not pending. 





        

<hr>




### function operator= {#function-operator}

```C++
Run & QAccelPlot::DataTransition::Run::operator= (
    const Run &
) = delete
```




<hr>




### function pending {#function-pending}

_Returns_ `true` _while the run holds target data that_`finish()` _has not taken._
```C++
bool QAccelPlot::DataTransition::Run::pending () const
```




<hr>




### function targetData {#function-targetdata}

_Returns the target data. Valid while_ `pending()` _is_`true` _._
```C++
const std::vector< double > & QAccelPlot::DataTransition::Run::targetData () const
```




<hr>




### function targetPointCount {#function-targetpointcount}

_Returns the number of points in_ `targetData()` _._
```C++
int QAccelPlot::DataTransition::Run::targetPointCount () const
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

