








# Struct QAccelPlot::DataPointStore::AttributeView



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**DataPointStore**](classQAccelPlot_1_1DataPointStore.md) **>** [**AttributeView**](structQAccelPlot_1_1DataPointStore_1_1AttributeView.md)



_Read-only view over the float attributes of the stored data points._ 

* `#include <DataPointStore.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  const float \* | [**data**](#variable-data)   = `{nullptr}`<br>_First attribute of the first data point, or_ `nullptr` _without attributes._ |
|  int | [**stride**](#variable-stride)   = `{0}`<br>_Floats between the attributes of consecutive data points._  |












































## Public Attributes Documentation





### variable data {#variable-data}

_First attribute of the first data point, or_ `nullptr` _without attributes._
```C++
const float* QAccelPlot::DataPointStore::AttributeView::data;
```




<hr>




### variable stride {#variable-stride}

_Floats between the attributes of consecutive data points._ 
```C++
int QAccelPlot::DataPointStore::AttributeView::stride;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/DataPointStore.hpp`

