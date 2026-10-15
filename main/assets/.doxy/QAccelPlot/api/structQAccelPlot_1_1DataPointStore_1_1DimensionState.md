








# Struct QAccelPlot::DataPointStore::DimensionState



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**DataPointStore**](classQAccelPlot_1_1DataPointStore.md) **>** [**DimensionState**](structQAccelPlot_1_1DataPointStore_1_1DimensionState.md)



_Scale and viewport of one dimension, as its axis currently shows it._ 

* `#include <DataPointStore.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**logScale**](#variable-logscale)   = `{false}`<br>_Whether the dimension is logarithmic._  |
|  std::optional&lt; [**Viewport**](structQAccelPlot_1_1DataPointStore_1_1Viewport.md) &gt; | [**viewport**](#variable-viewport)  <br>_The axis viewport, or_ `std::nullopt` _without an axis._ |












































## Public Attributes Documentation





### variable logScale {#variable-logscale}

_Whether the dimension is logarithmic._ 
```C++
bool QAccelPlot::DataPointStore::DimensionState::logScale;
```




<hr>




### variable viewport {#variable-viewport}

_The axis viewport, or_ `std::nullopt` _without an axis._
```C++
std::optional<Viewport> QAccelPlot::DataPointStore::DimensionState::viewport;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/DataPointStore.hpp`

