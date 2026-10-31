








# Struct QAccelPlot::InspectionDataPoint



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**InspectionDataPoint**](structQAccelPlot_1_1InspectionDataPoint.md)



_One XY source data point returned by an inspection query._ [More...](#detailed-description)

* `#include <InspectionResult.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**InspectionStatus**](namespaceQAccelPlot_1_1InspectionNS.md#enum-status) | [**status**](#variable-status-22)   = `{InspectionStatus::NoMatch}`<br>_Query outcome._  |








## Public Properties

| Type | Name |
| ---: | :--- |
| property quint64 | [**dataRevision**](structQAccelPlot_1_1InspectionDataPoint.md#property-datarevision)  <br>_Data revision the query ran against._  |
| property qreal | [**distance**](structQAccelPlot_1_1InspectionDataPoint.md#property-distance)  <br>_Distance from the query position in logical pixels._  |
| property int | [**index**](structQAccelPlot_1_1InspectionDataPoint.md#property-index)  <br>_Source index, counting invalid records; -1 when nothing matched._  |
| property bool | [**interpolated**](structQAccelPlot_1_1InspectionDataPoint.md#property-interpolated)  <br>_True when the coordinates are interpolated between two data points; index is then the data point whose value is shown._  |
| property qreal | [**pixelX**](structQAccelPlot_1_1InspectionDataPoint.md#property-pixelx)  <br>_X in series-local logical pixels; NaN for an invalid record._  |
| property qreal | [**pixelY**](structQAccelPlot_1_1InspectionDataPoint.md#property-pixely)  <br>_Y in series-local logical pixels; NaN for an invalid record._  |
| property QML\_ANONYMOUSQAccelPlot::InspectionNS::Status | [**status**](structQAccelPlot_1_1InspectionDataPoint.md#property-status-12)  <br>_Query outcome._  |
| property bool | [**valid**](structQAccelPlot_1_1InspectionDataPoint.md#property-valid-12)  <br>_True when status is_ `Ready` _._ |
| property qreal | [**value**](structQAccelPlot_1_1InspectionDataPoint.md#property-value)  <br>[_**Colormap**_](classQAccelPlot_1_1Colormap.md) _value of a_`PointCloud` _point or a heatmap cell; NaN for other series and for interpolated points._ |
| property qreal | [**x**](structQAccelPlot_1_1InspectionDataPoint.md#property-x)  <br>_X in data units._  |
| property qreal | [**y**](structQAccelPlot_1_1InspectionDataPoint.md#property-y)  <br>_Y in data units._  |








## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**valid**](#function-valid-22) () const<br>_Returns true when status is_ `Ready` _._ |




























## Detailed Description


`x` and `y` are in the units of the series' axes. `pixelX` and `pixelY` place the same point in series-local logical pixels, measured from the top left of the plot area; map them with `series.mapToItem()` for another item. 


    
## Public Attributes Documentation





### variable status {#variable-status-22}

_Query outcome._ 
```C++
InspectionStatus QAccelPlot::InspectionDataPoint::status;
```




<hr>
## Public Properties Documentation





### property dataRevision {#property-datarevision}

_Data revision the query ran against._ 
```C++
quint64 QAccelPlot::InspectionDataPoint::dataRevision;
```



Data revision. 


        

<hr>




### property distance {#property-distance}

_Distance from the query position in logical pixels._ 
```C++
qreal QAccelPlot::InspectionDataPoint::distance;
```



Pixel distance. 


        

<hr>




### property index {#property-index}

_Source index, counting invalid records; -1 when nothing matched._ 
```C++
int QAccelPlot::InspectionDataPoint::index;
```



Source index. 


        

<hr>




### property interpolated {#property-interpolated}

_True when the coordinates are interpolated between two data points; index is then the data point whose value is shown._ 
```C++
bool QAccelPlot::InspectionDataPoint::interpolated;
```



Interpolated point.


That is the left neighbor, or on a step line the data point whose step or riser holds the point. 


        

<hr>




### property pixelX {#property-pixelx}

_X in series-local logical pixels; NaN for an invalid record._ 
```C++
qreal QAccelPlot::InspectionDataPoint::pixelX;
```



X in series-local pixels. 


        

<hr>




### property pixelY {#property-pixely}

_Y in series-local logical pixels; NaN for an invalid record._ 
```C++
qreal QAccelPlot::InspectionDataPoint::pixelY;
```



Y in series-local pixels. 


        

<hr>




### property status {#property-status-12}

_Query outcome._ 
```C++
QML_ANONYMOUSQAccelPlot::InspectionNS::Status QAccelPlot::InspectionDataPoint::status;
```




<hr>




### property valid {#property-valid-12}

_True when status is_ `Ready` _._
```C++
bool QAccelPlot::InspectionDataPoint::valid;
```




<hr>




### property value {#property-value}

[_**Colormap**_](classQAccelPlot_1_1Colormap.md) _value of a_`PointCloud` _point or a heatmap cell; NaN for other series and for interpolated points._
```C++
qreal QAccelPlot::InspectionDataPoint::value;
```



[**Colormap**](classQAccelPlot_1_1Colormap.md) value. 


        

<hr>




### property x {#property-x}

_X in data units._ 
```C++
qreal QAccelPlot::InspectionDataPoint::x;
```




<hr>




### property y {#property-y}

_Y in data units._ 
```C++
qreal QAccelPlot::InspectionDataPoint::y;
```




<hr>
## Public Functions Documentation





### function valid {#function-valid-22}

_Returns true when status is_ `Ready` _._
```C++
bool QAccelPlot::InspectionDataPoint::valid () const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/InspectionResult.hpp`

