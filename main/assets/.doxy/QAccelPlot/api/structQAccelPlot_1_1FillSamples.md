








# Struct QAccelPlot::FillSamples



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**FillSamples**](structQAccelPlot_1_1FillSamples.md)



_Samples of a gradient fill: one group per valid-sample run, broken at gaps._ 

* `#include <LineCurveLineRenderer.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::vector&lt; int &gt; | [**indices**](#variable-indices)  <br>_Source indices of all sampled runs, run after run._  |
|  std::vector&lt; int &gt; | [**sampledCounts**](#variable-sampledcounts)  <br>_Number of samples of each run; zero for runs too short to fill._  |












































## Public Attributes Documentation





### variable indices {#variable-indices}

_Source indices of all sampled runs, run after run._ 
```C++
std::vector<int> QAccelPlot::FillSamples::indices;
```




<hr>




### variable sampledCounts {#variable-sampledcounts}

_Number of samples of each run; zero for runs too short to fill._ 
```C++
std::vector<int> QAccelPlot::FillSamples::sampledCounts;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/LineCurveLineRenderer.hpp`

