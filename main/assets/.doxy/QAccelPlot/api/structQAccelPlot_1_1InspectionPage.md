








# Struct QAccelPlot::InspectionPage



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**InspectionPage**](structQAccelPlot_1_1InspectionPage.md)



_One page of source indices inside a region._ 

* `#include <InspectionResult.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**InspectionStatus**](namespaceQAccelPlot_1_1InspectionNS.md#enum-status) | [**status**](#variable-status-22)   = `{InspectionStatus::NoMatch}`<br>_Query outcome._  |








## Public Properties

| Type | Name |
| ---: | :--- |
| property quint64 | [**dataRevision**](structQAccelPlot_1_1InspectionPage.md#property-datarevision)  <br>_Data revision the indices refer to._  |
| property bool | [**hasMore**](structQAccelPlot_1_1InspectionPage.md#property-hasmore)  <br>_True when matches remain after this page._  |
| property QList&lt; int &gt; | [**indices**](structQAccelPlot_1_1InspectionPage.md#property-indices)  <br>_Matching source indices._  |
| property int | [**limit**](structQAccelPlot_1_1InspectionPage.md#property-limit)  <br>_Effective page size after clamping to the maximum._  |
| property int | [**offset**](structQAccelPlot_1_1InspectionPage.md#property-offset)  <br>_Number of matches skipped before this page._  |
| property bool | [**sourceOrder**](structQAccelPlot_1_1InspectionPage.md#property-sourceorder)  <br>_True when pages are in ascending source order; false for index traversal order._  |
| property QML\_ANONYMOUSQAccelPlot::InspectionNS::Status | [**status**](structQAccelPlot_1_1InspectionPage.md#property-status-12)  <br>_Query outcome;_ `Ready` _also for an empty page of a matching region._ |
| property int | [**total**](structQAccelPlot_1_1InspectionPage.md#property-total)  <br>_Number of matching samples in the whole region._  |
| property bool | [**valid**](structQAccelPlot_1_1InspectionPage.md#property-valid-12)  <br>_True when status is_ `Ready` _._ |








## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**valid**](#function-valid-22) () const<br>_Returns true when status is_ `Ready` _._ |




























## Public Attributes Documentation





### variable status {#variable-status-22}

_Query outcome._ 
```C++
InspectionStatus QAccelPlot::InspectionPage::status;
```




<hr>
## Public Properties Documentation





### property dataRevision {#property-datarevision}

_Data revision the indices refer to._ 
```C++
quint64 QAccelPlot::InspectionPage::dataRevision;
```



Data revision. 


        

<hr>




### property hasMore {#property-hasmore}

_True when matches remain after this page._ 
```C++
bool QAccelPlot::InspectionPage::hasMore;
```



More pages follow. 


        

<hr>




### property indices {#property-indices}

_Matching source indices._ 
```C++
QList< int > QAccelPlot::InspectionPage::indices;
```




<hr>




### property limit {#property-limit}

_Effective page size after clamping to the maximum._ 
```C++
int QAccelPlot::InspectionPage::limit;
```



Effective page size. 


        

<hr>




### property offset {#property-offset}

_Number of matches skipped before this page._ 
```C++
int QAccelPlot::InspectionPage::offset;
```



Skipped matches. 


        

<hr>




### property sourceOrder {#property-sourceorder}

_True when pages are in ascending source order; false for index traversal order._ 
```C++
bool QAccelPlot::InspectionPage::sourceOrder;
```



Ascending source order. 


        

<hr>




### property status {#property-status-12}

_Query outcome;_ `Ready` _also for an empty page of a matching region._
```C++
QML_ANONYMOUSQAccelPlot::InspectionNS::Status QAccelPlot::InspectionPage::status;
```




<hr>




### property total {#property-total}

_Number of matching samples in the whole region._ 
```C++
int QAccelPlot::InspectionPage::total;
```



Matches in the region. 


        

<hr>




### property valid {#property-valid-12}

_True when status is_ `Ready` _._
```C++
bool QAccelPlot::InspectionPage::valid;
```




<hr>
## Public Functions Documentation





### function valid {#function-valid-22}

_Returns true when status is_ `Ready` _._
```C++
bool QAccelPlot::InspectionPage::valid () const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/InspectionResult.hpp`

