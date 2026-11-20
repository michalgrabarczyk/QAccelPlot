








# Struct QAccelPlot::Internal::TransitionLayout



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**TransitionLayout**](structQAccelPlot_1_1Internal_1_1TransitionLayout.md)



_How a series' items animate._ 

* `#include <TransitionFrame.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  double | [**baseline**](#variable-baseline)   = `{0.0}`<br>_Value items grow from when the_ `GrowTransition` _has no baseline._ |
|  int | [**baselineColumn**](#variable-baselinecolumn)   = `{-1}`<br>_Column whose value each item grows from when the_ `GrowTransition` _has no baseline; -1 uses_`baseline` _._ |
|  bool | [**continuous**](#variable-continuous)   = `{false}`<br>_A_ `DrawTransition` _reveals the last item partway, as a line segment._ |
|  int | [**stride**](#variable-stride)   = `{2}`<br>_Values per item._  |
|  std::vector&lt; int &gt; | [**valueColumns**](#variable-valuecolumns)   = `{1}`<br>_Columns a_ `GrowTransition` _grows._ |
|  [**GrowViewport**](structQAccelPlot_1_1Internal_1_1GrowViewport.md) | [**viewport**](#variable-viewport)   = `{}`<br>_Viewport that places the values of_ `baselineColumn` _._ |












































## Public Attributes Documentation





### variable baseline {#variable-baseline}

_Value items grow from when the_ `GrowTransition` _has no baseline._
```C++
double QAccelPlot::Internal::TransitionLayout::baseline;
```




<hr>




### variable baselineColumn {#variable-baselinecolumn}

_Column whose value each item grows from when the_ `GrowTransition` _has no baseline; -1 uses_`baseline` _._
```C++
int QAccelPlot::Internal::TransitionLayout::baselineColumn;
```



`viewport` places each item's value with `placeGrowBaseline()`. 


        

<hr>




### variable continuous {#variable-continuous}

_A_ `DrawTransition` _reveals the last item partway, as a line segment._
```C++
bool QAccelPlot::Internal::TransitionLayout::continuous;
```




<hr>




### variable stride {#variable-stride}

_Values per item._ 
```C++
int QAccelPlot::Internal::TransitionLayout::stride;
```




<hr>




### variable valueColumns {#variable-valuecolumns}

_Columns a_ `GrowTransition` _grows._
```C++
std::vector<int> QAccelPlot::Internal::TransitionLayout::valueColumns;
```




<hr>




### variable viewport {#variable-viewport}

_Viewport that places the values of_ `baselineColumn` _._
```C++
GrowViewport QAccelPlot::Internal::TransitionLayout::viewport;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/transitions/internal/TransitionFrame.hpp`

