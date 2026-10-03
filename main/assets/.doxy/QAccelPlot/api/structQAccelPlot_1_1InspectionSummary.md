








# Struct QAccelPlot::InspectionSummary



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**InspectionSummary**](structQAccelPlot_1_1InspectionSummary.md)



_Sample-weighted Y statistics over the valid samples inside a region._ 

* `#include <InspectionResult.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**InspectionStatus**](namespaceQAccelPlot_1_1InspectionNS.md#enum-status) | [**status**](#variable-status-22)   = `{InspectionStatus::NoMatch}`<br>_Query outcome._  |








## Public Properties

| Type | Name |
| ---: | :--- |
| property int | [**count**](structQAccelPlot_1_1InspectionSummary.md#property-count)  <br>_Number of matching samples._  |
| property quint64 | [**dataRevision**](structQAccelPlot_1_1InspectionSummary.md#property-datarevision)  <br>_Data revision the query ran against._  |
| property qreal | [**maximum**](structQAccelPlot_1_1InspectionSummary.md#property-maximum)  <br>_Largest Y; NaN for an empty region._  |
| property int | [**maximumIndex**](structQAccelPlot_1_1InspectionSummary.md#property-maximumindex)  <br>_Source index of the largest Y, highest index on ties._  |
| property qreal | [**mean**](structQAccelPlot_1_1InspectionSummary.md#property-mean)  <br>_Mean Y; NaN for an empty region._  |
| property qreal | [**minimum**](structQAccelPlot_1_1InspectionSummary.md#property-minimum)  <br>_Smallest Y; NaN for an empty region._  |
| property int | [**minimumIndex**](structQAccelPlot_1_1InspectionSummary.md#property-minimumindex)  <br>_Source index of the smallest Y, highest index on ties._  |
| property qreal | [**standardDeviation**](structQAccelPlot_1_1InspectionSummary.md#property-standarddeviation)  <br>_Population standard deviation of Y; NaN for an empty region._  |
| property QML\_ANONYMOUSQAccelPlot::InspectionNS::Status | [**status**](structQAccelPlot_1_1InspectionSummary.md#property-status-12)  <br>_Query outcome;_ `NoMatch` _for an empty region._ |
| property bool | [**valid**](structQAccelPlot_1_1InspectionSummary.md#property-valid-12)  <br>_True when status is_ `Ready` _._ |








## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**valid**](#function-valid-22) () const<br>_Returns true when status is_ `Ready` _._ |




























## Public Attributes Documentation





### variable status {#variable-status-22}

_Query outcome._ 
```C++
InspectionStatus QAccelPlot::InspectionSummary::status;
```




<hr>
## Public Properties Documentation





### property count {#property-count}

_Number of matching samples._ 
```C++
int QAccelPlot::InspectionSummary::count;
```



Matching samples. 


        

<hr>




### property dataRevision {#property-datarevision}

_Data revision the query ran against._ 
```C++
quint64 QAccelPlot::InspectionSummary::dataRevision;
```



Data revision. 


        

<hr>




### property maximum {#property-maximum}

_Largest Y; NaN for an empty region._ 
```C++
qreal QAccelPlot::InspectionSummary::maximum;
```



Largest Y. 


        

<hr>




### property maximumIndex {#property-maximumindex}

_Source index of the largest Y, highest index on ties._ 
```C++
int QAccelPlot::InspectionSummary::maximumIndex;
```



Index of the largest Y. 


        

<hr>




### property mean {#property-mean}

_Mean Y; NaN for an empty region._ 
```C++
qreal QAccelPlot::InspectionSummary::mean;
```



Mean Y. 


        

<hr>




### property minimum {#property-minimum}

_Smallest Y; NaN for an empty region._ 
```C++
qreal QAccelPlot::InspectionSummary::minimum;
```



Smallest Y. 


        

<hr>




### property minimumIndex {#property-minimumindex}

_Source index of the smallest Y, highest index on ties._ 
```C++
int QAccelPlot::InspectionSummary::minimumIndex;
```



Index of the smallest Y. 


        

<hr>




### property standardDeviation {#property-standarddeviation}

_Population standard deviation of Y; NaN for an empty region._ 
```C++
qreal QAccelPlot::InspectionSummary::standardDeviation;
```



Population standard deviation. 


        

<hr>




### property status {#property-status-12}

_Query outcome;_ `NoMatch` _for an empty region._
```C++
QML_ANONYMOUSQAccelPlot::InspectionNS::Status QAccelPlot::InspectionSummary::status;
```




<hr>




### property valid {#property-valid-12}

_True when status is_ `Ready` _._
```C++
bool QAccelPlot::InspectionSummary::valid;
```




<hr>
## Public Functions Documentation





### function valid {#function-valid-22}

_Returns true when status is_ `Ready` _._
```C++
bool QAccelPlot::InspectionSummary::valid () const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/InspectionResult.hpp`

