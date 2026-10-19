








# Struct QAccelPlot::Internal::HeatmapGridDimension



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**HeatmapGridDimension**](structQAccelPlot_1_1Internal_1_1HeatmapGridDimension.md)



_One dimension of a heatmap grid:_ `count` _cells between the edges_`from` _and_`to` _._

* `#include <HeatmapGrid.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**count**](#variable-count)  <br>_Number of cells._  |
|  double | [**from**](#variable-from)  <br>_Outer edge of cell 0._  |
|  bool | [**logSpacing**](#variable-logspacing)  <br>_Whether the cells are evenly spaced in log10 instead of linearly._  |
|  double | [**to**](#variable-to)  <br>_Outer edge of the last cell._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  double | [**cellCenter**](#function-cellcenter) (int index) const<br>_Returns the center of cell_ _index:_ _arithmetic under linear spacing, geometric under log spacing._ |
|  double | [**cellCoordinate**](#function-cellcoordinate) (double value) const<br>_Returns the fractional cell coordinate of data_ _value_ _, or NaN when no cell can contain it._ |
|  std::optional&lt; std::pair&lt; double, double &gt; &gt; | [**dataExtent**](#function-dataextent) (bool logAxis) const<br>_Returns the ordered extent of the grid on an axis that is logarithmic when_ _logAxis_ _is set._ |
|  bool | [**isValid**](#function-isvalid) () const<br>_Returns whether both edges are finite, and positive under log spacing._  |
|  [**HeatmapShaderMapping**](structQAccelPlot_1_1Internal_1_1HeatmapShaderMapping.md) | [**shaderMapping**](#function-shadermapping) (const [**AxisMapping**](structQAccelPlot_1_1AxisMapping.md) & axis, double length) const<br>_Returns the mapping from pixels along an axis of_ _length_ _pixels, mapped by__axis_ _, to cell coordinates._ |




























## Public Attributes Documentation





### variable count {#variable-count}

_Number of cells._ 
```C++
int QAccelPlot::Internal::HeatmapGridDimension::count;
```




<hr>




### variable from {#variable-from}

_Outer edge of cell 0._ 
```C++
double QAccelPlot::Internal::HeatmapGridDimension::from;
```




<hr>




### variable logSpacing {#variable-logspacing}

_Whether the cells are evenly spaced in log10 instead of linearly._ 
```C++
bool QAccelPlot::Internal::HeatmapGridDimension::logSpacing;
```




<hr>




### variable to {#variable-to}

_Outer edge of the last cell._ 
```C++
double QAccelPlot::Internal::HeatmapGridDimension::to;
```




<hr>
## Public Functions Documentation





### function cellCenter {#function-cellcenter}

_Returns the center of cell_ _index:_ _arithmetic under linear spacing, geometric under log spacing._
```C++
double QAccelPlot::Internal::HeatmapGridDimension::cellCenter (
    int index
) const
```




<hr>




### function cellCoordinate {#function-cellcoordinate}

_Returns the fractional cell coordinate of data_ _value_ _, or NaN when no cell can contain it._
```C++
double QAccelPlot::Internal::HeatmapGridDimension::cellCoordinate (
    double value
) const
```




<hr>




### function dataExtent {#function-dataextent}

_Returns the ordered extent of the grid on an axis that is logarithmic when_ _logAxis_ _is set._
```C++
std::optional< std::pair< double, double > > QAccelPlot::Internal::HeatmapGridDimension::dataExtent (
    bool logAxis
) const
```



On a log axis the extent starts at the first cell edge above zero. Returns `std::nullopt` for an invalid dimension, and on a log axis for a grid entirely at or below zero. 


        

<hr>




### function isValid {#function-isvalid}

_Returns whether both edges are finite, and positive under log spacing._ 
```C++
bool QAccelPlot::Internal::HeatmapGridDimension::isValid () const
```




<hr>




### function shaderMapping {#function-shadermapping}

_Returns the mapping from pixels along an axis of_ _length_ _pixels, mapped by__axis_ _, to cell coordinates._
```C++
HeatmapShaderMapping QAccelPlot::Internal::HeatmapGridDimension::shaderMapping (
    const AxisMapping & axis,
    double length
) const
```



Computed in double precision relative to pixel 0, so that the single-precision shader only works with cell coordinates near the visible cells. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/internal/HeatmapGrid.hpp`

