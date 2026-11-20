








# Struct QAccelPlot::PlotSeries::DataUpdate



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**PlotSeries**](classQAccelPlot_1_1PlotSeries.md) **>** [**DataUpdate**](structQAccelPlot_1_1PlotSeries_1_1DataUpdate.md)



_A change of the data points, as passed to_ `updateDataRangeInputs()` _and_`onDataPointsChanged()` _._

* `#include <PlotSeries.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**DataChange**](classQAccelPlot_1_1PlotSeries.md#enum-datachange) | [**change**](#variable-change)   = `{DataChange::Replaced}`<br>_How the data points changed._  |
|  int | [**index**](#variable-index)   = `{-1}`<br>_The appended or modified data point; -1 for_ `DataChange::Replaced` _._ |
|  const double \* | [**previousCoordinates**](#variable-previouscoordinates)   = `{nullptr}`<br>_The_ `columnCount()` _coordinates the modified data point had;_`nullptr` _unless_`DataChange::Modified` _._ |
|  int | [**previousCount**](#variable-previouscount)   = `{0}`<br>_The data point count before the change._  |












































## Public Attributes Documentation





### variable change {#variable-change}

_How the data points changed._ 
```C++
DataChange QAccelPlot::PlotSeries::DataUpdate::change;
```




<hr>




### variable index {#variable-index}

_The appended or modified data point; -1 for_ `DataChange::Replaced` _._
```C++
int QAccelPlot::PlotSeries::DataUpdate::index;
```




<hr>




### variable previousCoordinates {#variable-previouscoordinates}

_The_ `columnCount()` _coordinates the modified data point had;_`nullptr` _unless_`DataChange::Modified` _._
```C++
const double* QAccelPlot::PlotSeries::DataUpdate::previousCoordinates;
```



Only valid until `onDataPointsChanged()` returns. 


        

<hr>




### variable previousCount {#variable-previouscount}

_The data point count before the change._ 
```C++
int QAccelPlot::PlotSeries::DataUpdate::previousCount;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/PlotSeries.hpp`

