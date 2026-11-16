








# Struct QAccelPlot::FillSamples



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**FillSamples**](structQAccelPlot_1_1FillSamples.md)



_Outline of a gradient fill in data space: one group of points per run of valid data points, broken at gaps._ [More...](#detailed-description)

* `#include <LineCurveLineRenderer.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::vector&lt; int &gt; | [**pointCounts**](#variable-pointcounts)  <br>_Number of points of each run; zero for runs too short to fill._  |
|  std::vector&lt; QPointF &gt; | [**points**](#variable-points)  <br>_Points of all runs, run after run._  |












































## Detailed Description


Each point is a sampled data point or, with a step mode, a step corner. 


    
## Public Attributes Documentation





### variable pointCounts {#variable-pointcounts}

_Number of points of each run; zero for runs too short to fill._ 
```C++
std::vector<int> QAccelPlot::FillSamples::pointCounts;
```




<hr>




### variable points {#variable-points}

_Points of all runs, run after run._ 
```C++
std::vector<QPointF> QAccelPlot::FillSamples::points;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/LineCurveLineRenderer.hpp`

