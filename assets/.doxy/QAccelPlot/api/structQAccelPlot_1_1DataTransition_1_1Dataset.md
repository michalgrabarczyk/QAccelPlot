








# Struct QAccelPlot::DataTransition::Dataset



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**DataTransition**](classQAccelPlot_1_1DataTransition.md) **>** [**Dataset**](structQAccelPlot_1_1DataTransition_1_1Dataset.md)



_Data a transition animates:_ `count` _items of_`stride` _values each, e.g. XY points with a stride of 2._

* `#include <DataTransition.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**count**](#variable-count)   = `{0}`<br>_Number of items in_ `values` _._ |
|  int | [**stride**](#variable-stride)   = `{2}`<br>_Number of values per item._  |
|  std::vector&lt; double &gt; | [**values**](#variable-values)  <br>_The values, item after item._  |












































## Public Attributes Documentation





### variable count {#variable-count}

_Number of items in_ `values` _._
```C++
int QAccelPlot::DataTransition::Dataset::count;
```




<hr>




### variable stride {#variable-stride}

_Number of values per item._ 
```C++
int QAccelPlot::DataTransition::Dataset::stride;
```




<hr>




### variable values {#variable-values}

_The values, item after item._ 
```C++
std::vector<double> QAccelPlot::DataTransition::Dataset::values;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/transitions/DataTransition.hpp`

