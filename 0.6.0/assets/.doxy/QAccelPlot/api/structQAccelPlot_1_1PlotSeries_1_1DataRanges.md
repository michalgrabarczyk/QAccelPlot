








# Struct QAccelPlot::PlotSeries::DataRanges



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**PlotSeries**](classQAccelPlot_1_1PlotSeries.md) **>** [**DataRanges**](structQAccelPlot_1_1PlotSeries_1_1DataRanges.md)



_Extents of a series in both dimensions._ [More...](#detailed-description)

* `#include <PlotSeries.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::optional&lt; [**DataExtent**](structQAccelPlot_1_1PlotSeries_1_1DataExtent.md) &gt; | [**x**](#variable-x)  <br>_Extent reported to the horizontal axis._  |
|  std::optional&lt; [**DataExtent**](structQAccelPlot_1_1PlotSeries_1_1DataExtent.md) &gt; | [**y**](#variable-y)  <br>_Extent reported to the vertical axis._  |












































## Detailed Description


A dimension without a valid coordinate is unset; an extent that is not finite or not ordered counts as unset. 


    
## Public Attributes Documentation





### variable x {#variable-x}

_Extent reported to the horizontal axis._ 
```C++
std::optional<DataExtent> QAccelPlot::PlotSeries::DataRanges::x;
```




<hr>




### variable y {#variable-y}

_Extent reported to the vertical axis._ 
```C++
std::optional<DataExtent> QAccelPlot::PlotSeries::DataRanges::y;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/PlotSeries.hpp`

