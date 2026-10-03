








# Struct QAccelPlot::InspectionRow



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**InspectionRow**](structQAccelPlot_1_1InspectionRow.md)



_Inspection result of one series, as shown by one row of an_ `InspectionRowModel` _._

* `#include <InspectionRowModel.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**hasSummary**](#variable-hassummary)   = `{false}`<br>_Whether_ `summary` _was requested._ |
|  QString | [**maximumText**](#variable-maximumtext)  <br>_Summary maximum formatted by the series' Y axis._  |
|  QString | [**meanText**](#variable-meantext)  <br>_Summary mean formatted by the series' Y axis._  |
|  QString | [**minimumText**](#variable-minimumtext)  <br>_Summary minimum formatted by the series' Y axis._  |
|  QPointF | [**pixelPosition**](#variable-pixelposition)  <br>_Sample position in plot-local logical pixels._  |
|  [**InspectionSample**](structQAccelPlot_1_1InspectionSample.md) | [**sample**](#variable-sample)  <br>_Sample at the cursor; invalid for selection rows._  |
|  QPointer&lt; [**PlotSeries**](classQAccelPlot_1_1PlotSeries.md) &gt; | [**series**](#variable-series)  <br>_Series the row describes._  |
|  [**InspectionSummary**](structQAccelPlot_1_1InspectionSummary.md) | [**summary**](#variable-summary)  <br>_Region statistics._  |
|  QString | [**xText**](#variable-xtext)  <br>_Sample X formatted by the series' X axis._  |
|  QString | [**yText**](#variable-ytext)  <br>_Sample Y formatted by the series' Y axis._  |












































## Public Attributes Documentation





### variable hasSummary {#variable-hassummary}

_Whether_ `summary` _was requested._
```C++
bool QAccelPlot::InspectionRow::hasSummary;
```




<hr>




### variable maximumText {#variable-maximumtext}

_Summary maximum formatted by the series' Y axis._ 
```C++
QString QAccelPlot::InspectionRow::maximumText;
```




<hr>




### variable meanText {#variable-meantext}

_Summary mean formatted by the series' Y axis._ 
```C++
QString QAccelPlot::InspectionRow::meanText;
```




<hr>




### variable minimumText {#variable-minimumtext}

_Summary minimum formatted by the series' Y axis._ 
```C++
QString QAccelPlot::InspectionRow::minimumText;
```




<hr>




### variable pixelPosition {#variable-pixelposition}

_Sample position in plot-local logical pixels._ 
```C++
QPointF QAccelPlot::InspectionRow::pixelPosition;
```




<hr>




### variable sample {#variable-sample}

_Sample at the cursor; invalid for selection rows._ 
```C++
InspectionSample QAccelPlot::InspectionRow::sample;
```




<hr>




### variable series {#variable-series}

_Series the row describes._ 
```C++
QPointer<PlotSeries> QAccelPlot::InspectionRow::series;
```




<hr>




### variable summary {#variable-summary}

_Region statistics._ 
```C++
InspectionSummary QAccelPlot::InspectionRow::summary;
```




<hr>




### variable xText {#variable-xtext}

_Sample X formatted by the series' X axis._ 
```C++
QString QAccelPlot::InspectionRow::xText;
```




<hr>




### variable yText {#variable-ytext}

_Sample Y formatted by the series' Y axis._ 
```C++
QString QAccelPlot::InspectionRow::yText;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/InspectionRowModel.hpp`

