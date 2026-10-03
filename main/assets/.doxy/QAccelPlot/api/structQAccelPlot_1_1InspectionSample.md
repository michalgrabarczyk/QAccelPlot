








# Struct QAccelPlot::InspectionSample



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**InspectionSample**](structQAccelPlot_1_1InspectionSample.md)



_One XY source sample returned by an inspection query._ 

* `#include <InspectionResult.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  QPointF | [**position**](#variable-position)   = `{std::numeric\_limits&lt;qreal&gt;::quiet\_NaN(), std::numeric\_limits&lt;qreal&gt;::quiet\_NaN()}`<br>_Data-space XY._  |
|  [**InspectionStatus**](namespaceQAccelPlot_1_1InspectionNS.md#enum-status) | [**status**](#variable-status-22)   = `{InspectionStatus::NoMatch}`<br>_Query outcome._  |








## Public Properties

| Type | Name |
| ---: | :--- |
| property quint64 | [**dataRevision**](structQAccelPlot_1_1InspectionSample.md#property-datarevision)  <br>_Data revision the query ran against._  |
| property qreal | [**distance**](structQAccelPlot_1_1InspectionSample.md#property-distance)  <br>_Distance from the query position in logical pixels._  |
| property int | [**index**](structQAccelPlot_1_1InspectionSample.md#property-index)  <br>_Source index, counting invalid records; -1 when nothing matched._  |
| property bool | [**interpolated**](structQAccelPlot_1_1InspectionSample.md#property-interpolated)  <br>_True when the coordinates are interpolated between two samples; index is then the left neighbor._  |
| property QPointF | [**pixelPosition**](structQAccelPlot_1_1InspectionSample.md#property-pixelposition)  <br>_Series-local position in logical pixels._  |
| property QML\_ANONYMOUSQAccelPlot::InspectionNS::Status | [**status**](structQAccelPlot_1_1InspectionSample.md#property-status-12)  <br>_Query outcome._  |
| property bool | [**valid**](structQAccelPlot_1_1InspectionSample.md#property-valid-12)  <br>_True when status is_ `Ready` _._ |
| property qreal | [**value**](structQAccelPlot_1_1InspectionSample.md#property-value)  <br>_Optional per-point scalar; NaN when the series has none._  |
| property qreal | [**x**](structQAccelPlot_1_1InspectionSample.md#property-x-12)  <br>_Data-space X coordinate._  |
| property qreal | [**y**](structQAccelPlot_1_1InspectionSample.md#property-y-12)  <br>_Data-space Y coordinate._  |








## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**valid**](#function-valid-22) () const<br>_Returns true when status is_ `Ready` _._ |
|  qreal | [**x**](#function-x-22) () const<br>_Returns the data-space X coordinate._  |
|  qreal | [**y**](#function-y-22) () const<br>_Returns the data-space Y coordinate._  |




























## Public Attributes Documentation





### variable position {#variable-position}

_Data-space XY._ 
```C++
QPointF QAccelPlot::InspectionSample::position;
```




<hr>




### variable status {#variable-status-22}

_Query outcome._ 
```C++
InspectionStatus QAccelPlot::InspectionSample::status;
```




<hr>
## Public Properties Documentation





### property dataRevision {#property-datarevision}

_Data revision the query ran against._ 
```C++
quint64 QAccelPlot::InspectionSample::dataRevision;
```



Data revision. 


        

<hr>




### property distance {#property-distance}

_Distance from the query position in logical pixels._ 
```C++
qreal QAccelPlot::InspectionSample::distance;
```



Pixel distance. 


        

<hr>




### property index {#property-index}

_Source index, counting invalid records; -1 when nothing matched._ 
```C++
int QAccelPlot::InspectionSample::index;
```



Source index. 


        

<hr>




### property interpolated {#property-interpolated}

_True when the coordinates are interpolated between two samples; index is then the left neighbor._ 
```C++
bool QAccelPlot::InspectionSample::interpolated;
```



Interpolated point. 


        

<hr>




### property pixelPosition {#property-pixelposition}

_Series-local position in logical pixels._ 
```C++
QPointF QAccelPlot::InspectionSample::pixelPosition;
```



Series-local pixels. 


        

<hr>




### property status {#property-status-12}

_Query outcome._ 
```C++
QML_ANONYMOUSQAccelPlot::InspectionNS::Status QAccelPlot::InspectionSample::status;
```




<hr>




### property valid {#property-valid-12}

_True when status is_ `Ready` _._
```C++
bool QAccelPlot::InspectionSample::valid;
```




<hr>




### property value {#property-value}

_Optional per-point scalar; NaN when the series has none._ 
```C++
qreal QAccelPlot::InspectionSample::value;
```



Optional scalar. 


        

<hr>




### property x {#property-x-12}

_Data-space X coordinate._ 
```C++
qreal QAccelPlot::InspectionSample::x;
```




<hr>




### property y {#property-y-12}

_Data-space Y coordinate._ 
```C++
qreal QAccelPlot::InspectionSample::y;
```




<hr>
## Public Functions Documentation





### function valid {#function-valid-22}

_Returns true when status is_ `Ready` _._
```C++
bool QAccelPlot::InspectionSample::valid () const
```




<hr>




### function x {#function-x-22}

_Returns the data-space X coordinate._ 
```C++
qreal QAccelPlot::InspectionSample::x () const
```




<hr>




### function y {#function-y-22}

_Returns the data-space Y coordinate._ 
```C++
qreal QAccelPlot::InspectionSample::y () const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/InspectionResult.hpp`

