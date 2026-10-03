








# Struct QAccelPlot::InspectionRecord



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**InspectionRecord**](structQAccelPlot_1_1InspectionRecord.md)



_A native record of a series that is not a plain XY series, such as a bar, rectangle, or band._ 

* `#include <InspectionResult.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**InspectionStatus**](namespaceQAccelPlot_1_1InspectionNS.md#enum-status) | [**status**](#variable-status-22)   = `{InspectionStatus::NoMatch}`<br>_Query outcome._  |








## Public Properties

| Type | Name |
| ---: | :--- |
| property quint64 | [**dataRevision**](structQAccelPlot_1_1InspectionRecord.md#property-datarevision)  <br>_Data revision the query ran against._  |
| property QVariantMap | [**fields**](structQAccelPlot_1_1InspectionRecord.md#property-fields)  <br>_Record values keyed by the series' own field names._  |
| property int | [**index**](structQAccelPlot_1_1InspectionRecord.md#property-index)  <br>_Source index; -1 for an interpolated hit._  |
| property bool | [**interpolated**](structQAccelPlot_1_1InspectionRecord.md#property-interpolated)  <br>_True when the fields are interpolated between source records._  |
| property QML\_ANONYMOUSQAccelPlot::InspectionNS::Status | [**status**](structQAccelPlot_1_1InspectionRecord.md#property-status-12)  <br>_Query outcome._  |
| property bool | [**valid**](structQAccelPlot_1_1InspectionRecord.md#property-valid-12)  <br>_True when status is_ `Ready` _._ |








## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**valid**](#function-valid-22) () const<br>_Returns true when status is_ `Ready` _._ |




























## Public Attributes Documentation





### variable status {#variable-status-22}

_Query outcome._ 
```C++
InspectionStatus QAccelPlot::InspectionRecord::status;
```




<hr>
## Public Properties Documentation





### property dataRevision {#property-datarevision}

_Data revision the query ran against._ 
```C++
quint64 QAccelPlot::InspectionRecord::dataRevision;
```



Data revision. 


        

<hr>




### property fields {#property-fields}

_Record values keyed by the series' own field names._ 
```C++
QVariantMap QAccelPlot::InspectionRecord::fields;
```



Record values. 


        

<hr>




### property index {#property-index}

_Source index; -1 for an interpolated hit._ 
```C++
int QAccelPlot::InspectionRecord::index;
```



Source index. 


        

<hr>




### property interpolated {#property-interpolated}

_True when the fields are interpolated between source records._ 
```C++
bool QAccelPlot::InspectionRecord::interpolated;
```



Interpolated fields. 


        

<hr>




### property status {#property-status-12}

_Query outcome._ 
```C++
QML_ANONYMOUSQAccelPlot::InspectionNS::Status QAccelPlot::InspectionRecord::status;
```




<hr>




### property valid {#property-valid-12}

_True when status is_ `Ready` _._
```C++
bool QAccelPlot::InspectionRecord::valid;
```




<hr>
## Public Functions Documentation





### function valid {#function-valid-22}

_Returns true when status is_ `Ready` _._
```C++
bool QAccelPlot::InspectionRecord::valid () const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/InspectionResult.hpp`

