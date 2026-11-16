








# Struct QAccelPlot::PlotSeries::DataBounds



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**PlotSeries**](classQAccelPlot_1_1PlotSeries.md) **>** [**DataBounds**](structQAccelPlot_1_1PlotSeries_1_1DataBounds.md)



_Extents of a data update that the caller already knows._ [More...](#detailed-description)

* `#include <PlotSeries.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  qreal | [**xMax**](#variable-xmax)  <br>_Largest value on the horizontal axis._  |
|  qreal | [**xMin**](#variable-xmin)  <br>_Smallest value on the horizontal axis._  |
|  qreal | [**yMax**](#variable-ymax)  <br>_Largest value on the vertical axis._  |
|  qreal | [**yMin**](#variable-ymin)  <br>_Smallest value on the vertical axis._  |












































## Detailed Description


Passed with the data, they are reported to the axes in place of a scan over the records. A dimension whose bounds are not finite or not ordered reports no extent. 


    
## Public Attributes Documentation





### variable xMax {#variable-xmax}

_Largest value on the horizontal axis._ 
```C++
qreal QAccelPlot::PlotSeries::DataBounds::xMax;
```




<hr>




### variable xMin {#variable-xmin}

_Smallest value on the horizontal axis._ 
```C++
qreal QAccelPlot::PlotSeries::DataBounds::xMin;
```




<hr>




### variable yMax {#variable-ymax}

_Largest value on the vertical axis._ 
```C++
qreal QAccelPlot::PlotSeries::DataBounds::yMax;
```




<hr>




### variable yMin {#variable-ymin}

_Smallest value on the vertical axis._ 
```C++
qreal QAccelPlot::PlotSeries::DataBounds::yMin;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/PlotSeries.hpp`

