








# Struct QAccelPlot::InspectionBracket



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**InspectionBracket**](structQAccelPlot_1_1InspectionBracket.md)



_The valid data points on either side of a position on one axis._ 

* `#include <InspectionResult.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**InspectionStatus**](namespaceQAccelPlot_1_1InspectionNS.md#enum-status) | [**status**](#variable-status-22)   = `{InspectionStatus::NoMatch}`<br>_Query outcome._  |








## Public Properties

| Type | Name |
| ---: | :--- |
| property bool | [**adjacent**](structQAccelPlot_1_1InspectionBracket.md#property-adjacent)  <br>_True when both neighbors exist and their source indices are consecutive._  |
| property quint64 | [**dataRevision**](structQAccelPlot_1_1InspectionBracket.md#property-datarevision)  <br>_Data revision the query ran against._  |
| property [**InspectionDataPoint**](structQAccelPlot_1_1InspectionDataPoint.md) | [**interpolated**](structQAccelPlot_1_1InspectionBracket.md#property-interpolated)  <br>_Point on the line between adjacent neighbors: the straight on-screen segment, or the steps of a step line; invalid otherwise._  |
| property [**InspectionDataPoint**](structQAccelPlot_1_1InspectionDataPoint.md) | [**left**](structQAccelPlot_1_1InspectionBracket.md#property-left)  <br>_Valid data point with the largest coordinate at or below the position; invalid when there is none._  |
| property [**InspectionDataPoint**](structQAccelPlot_1_1InspectionDataPoint.md) | [**right**](structQAccelPlot_1_1InspectionBracket.md#property-right)  <br>_Valid data point with the smallest coordinate above the position; invalid when there is none._  |
| property QML\_ANONYMOUSInspectionNS::Status | [**status**](structQAccelPlot_1_1InspectionBracket.md#property-status-12)  <br>`Ready` _when at least one neighbor exists._ |
| property bool | [**valid**](structQAccelPlot_1_1InspectionBracket.md#property-valid-12)  <br>_True when status is_ `Ready` _._ |








## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**valid**](#function-valid-22) () const<br>_Returns true when status is_ `Ready` _._ |




























## Public Attributes Documentation





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




### property interpolated {#property-interpolated}

_Point on the line between adjacent neighbors: the straight on-screen segment, or the steps of a step line; invalid otherwise._ 
```C++
InspectionDataPoint QAccelPlot::InspectionBracket::interpolated;
```



Interpolated point between adjacent neighbors. 


        

<hr>




### property left {#property-left}

_Valid data point with the largest coordinate at or below the position; invalid when there is none._ 
```C++
InspectionDataPoint QAccelPlot::InspectionBracket::left;
```



Neighbor at or before the position. 


        

<hr>




### property right {#property-right}

_Valid data point with the smallest coordinate above the position; invalid when there is none._ 
```C++
InspectionDataPoint QAccelPlot::InspectionBracket::right;
```



Neighbor after the position. 


        

<hr>




### property status {#property-status-12}

`Ready` _when at least one neighbor exists._
```C++
QML_ANONYMOUSInspectionNS::Status QAccelPlot::InspectionBracket::status;
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

