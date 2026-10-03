








# Struct QAccelPlot::InspectionBracket



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**InspectionBracket**](structQAccelPlot_1_1InspectionBracket.md)



_The valid samples on either side of a position on one axis._ 

* `#include <InspectionResult.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**InspectionSample**](structQAccelPlot_1_1InspectionSample.md) | [**interpolated**](#variable-interpolated-22)  <br>_Interpolated point between adjacent neighbors._  |
|  [**InspectionSample**](structQAccelPlot_1_1InspectionSample.md) | [**left**](#variable-left-22)  <br>_Neighbor at or before the position._  |
|  [**InspectionSample**](structQAccelPlot_1_1InspectionSample.md) | [**right**](#variable-right-22)  <br>_Neighbor after the position._  |
|  [**InspectionStatus**](namespaceQAccelPlot_1_1InspectionNS.md#enum-status) | [**status**](#variable-status-22)   = `{InspectionStatus::NoMatch}`<br>_Query outcome._  |








## Public Properties

| Type | Name |
| ---: | :--- |
| property bool | [**adjacent**](structQAccelPlot_1_1InspectionBracket.md#property-adjacent)  <br>_True when both neighbors exist and their source indices are consecutive._  |
| property quint64 | [**dataRevision**](structQAccelPlot_1_1InspectionBracket.md#property-datarevision)  <br>_Data revision the query ran against._  |
| property [**QAccelPlot::InspectionSample**](structQAccelPlot_1_1InspectionSample.md) | [**interpolated**](structQAccelPlot_1_1InspectionBracket.md#property-interpolated-12)  <br>_Point on the straight on-screen segment between adjacent neighbors; invalid otherwise._  |
| property [**QAccelPlot::InspectionSample**](structQAccelPlot_1_1InspectionSample.md) | [**left**](structQAccelPlot_1_1InspectionBracket.md#property-left-12)  <br>_Valid sample with the largest coordinate at or below the position; invalid when there is none._  |
| property [**QAccelPlot::InspectionSample**](structQAccelPlot_1_1InspectionSample.md) | [**right**](structQAccelPlot_1_1InspectionBracket.md#property-right-12)  <br>_Valid sample with the smallest coordinate above the position; invalid when there is none._  |
| property QML\_ANONYMOUSQAccelPlot::InspectionNS::Status | [**status**](structQAccelPlot_1_1InspectionBracket.md#property-status-12)  <br>`Ready` _when at least one neighbor exists._ |
| property bool | [**valid**](structQAccelPlot_1_1InspectionBracket.md#property-valid-12)  <br>_True when status is_ `Ready` _._ |








## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**valid**](#function-valid-22) () const<br>_Returns true when status is_ `Ready` _._ |




























## Public Attributes Documentation





### variable interpolated {#variable-interpolated-22}

_Interpolated point between adjacent neighbors._ 
```C++
InspectionSample QAccelPlot::InspectionBracket::interpolated;
```




<hr>




### variable left {#variable-left-22}

_Neighbor at or before the position._ 
```C++
InspectionSample QAccelPlot::InspectionBracket::left;
```




<hr>




### variable right {#variable-right-22}

_Neighbor after the position._ 
```C++
InspectionSample QAccelPlot::InspectionBracket::right;
```




<hr>




### variable status {#variable-status-22}

_Query outcome._ 
```C++
InspectionStatus QAccelPlot::InspectionBracket::status;
```




<hr>
## Public Properties Documentation





### property adjacent {#property-adjacent}

_True when both neighbors exist and their source indices are consecutive._ 
```C++
bool QAccelPlot::InspectionBracket::adjacent;
```



Consecutive source indices. 


        

<hr>




### property dataRevision {#property-datarevision}

_Data revision the query ran against._ 
```C++
quint64 QAccelPlot::InspectionBracket::dataRevision;
```



Data revision. 


        

<hr>




### property interpolated {#property-interpolated-12}

_Point on the straight on-screen segment between adjacent neighbors; invalid otherwise._ 
```C++
QAccelPlot::InspectionSample QAccelPlot::InspectionBracket::interpolated;
```




<hr>




### property left {#property-left-12}

_Valid sample with the largest coordinate at or below the position; invalid when there is none._ 
```C++
QAccelPlot::InspectionSample QAccelPlot::InspectionBracket::left;
```




<hr>




### property right {#property-right-12}

_Valid sample with the smallest coordinate above the position; invalid when there is none._ 
```C++
QAccelPlot::InspectionSample QAccelPlot::InspectionBracket::right;
```




<hr>




### property status {#property-status-12}

`Ready` _when at least one neighbor exists._
```C++
QML_ANONYMOUSQAccelPlot::InspectionNS::Status QAccelPlot::InspectionBracket::status;
```




<hr>




### property valid {#property-valid-12}

_True when status is_ `Ready` _._
```C++
bool QAccelPlot::InspectionBracket::valid;
```




<hr>
## Public Functions Documentation





### function valid {#function-valid-22}

_Returns true when status is_ `Ready` _._
```C++
bool QAccelPlot::InspectionBracket::valid () const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/InspectionResult.hpp`

