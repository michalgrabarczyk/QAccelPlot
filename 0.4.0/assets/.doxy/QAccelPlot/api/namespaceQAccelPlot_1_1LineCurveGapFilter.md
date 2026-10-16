








# Namespace QAccelPlot::LineCurveGapFilter



[**Namespace List**](namespaces.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**LineCurveGapFilter**](namespaceQAccelPlot_1_1LineCurveGapFilter.md)



_Stateless helpers implementing the invalid-data-point contract shared by_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _subsystems._[More...](#detailed-description)








































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**appendEnvelopeSamples**](#function-appendenvelopesamples) (const [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) & data, const [**DataPointRun**](structQAccelPlot_1_1DataPointRun.md) & run, int sampledCount, std::vector&lt; int &gt; & indices) <br>_Appends_ _sampledCount_ _source indices of__run_ _in__data_ _to__indices_ _, in index order._ |
|  std::vector&lt; [**DataPointRun**](structQAccelPlot_1_1DataPointRun.md) &gt; | [**findValidRuns**](#function-findvalidruns) (const [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) & data, int pointCount, bool logScaleX, bool logScaleY) <br>_Returns the maximal runs of consecutive valid data points in_ _data_ _._ |
|  bool | [**isValidPoint**](#function-isvalidpoint) (const [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) & data, int index, bool logScaleX, bool logScaleY) <br>_Returns_ `true` _when both coordinates of data point__index_ _in__data_ _are valid._ |
|  std::vector&lt; int &gt; | [**planRunSampling**](#function-planrunsampling) (const std::vector&lt; [**DataPointRun**](structQAccelPlot_1_1DataPointRun.md) &gt; & runs, int maxSampledPoints) <br>_Distributes a budget of_ _maxSampledPoints_ _across__runs_ _._ |




























## Detailed Description


A data point is invalid when either coordinate is non-finite, or is not strictly positive on a logarithmic dimension. See `isValidCoordinate()` in `MathUtils.hpp`. 


    
## Public Functions Documentation





### function appendEnvelopeSamples {#function-appendenvelopesamples}

_Appends_ _sampledCount_ _source indices of__run_ _in__data_ _to__indices_ _, in index order._
```C++
void QAccelPlot::LineCurveGapFilter::appendEnvelopeSamples (
    const CurveDataView & data,
    const DataPointRun & run,
    int sampledCount,
    std::vector< int > & indices
) 
```



Keeps every sample when _sampledCount_ covers the run. Otherwise keeps the run's endpoints and the lowest and highest Y data point of equally sized buckets, so the decimated curve keeps its peaks. 


        

<hr>




### function findValidRuns {#function-findvalidruns}

_Returns the maximal runs of consecutive valid data points in_ _data_ _._
```C++
std::vector< DataPointRun > QAccelPlot::LineCurveGapFilter::findValidRuns (
    const CurveDataView & data,
    int pointCount,
    bool logScaleX,
    bool logScaleY
) 
```




<hr>




### function isValidPoint {#function-isvalidpoint}

_Returns_ `true` _when both coordinates of data point__index_ _in__data_ _are valid._
```C++
bool QAccelPlot::LineCurveGapFilter::isValidPoint (
    const CurveDataView & data,
    int index,
    bool logScaleX,
    bool logScaleY
) 
```




<hr>




### function planRunSampling {#function-planrunsampling}

_Distributes a budget of_ _maxSampledPoints_ _across__runs_ _._
```C++
std::vector< int > QAccelPlot::LineCurveGapFilter::planRunSampling (
    const std::vector< DataPointRun > & runs,
    int maxSampledPoints
) 
```



Returns one sampled count per run. Runs with fewer than two samples receive zero (they cannot form an area). Every other run keeps at least its first and last data point, so the total may exceed the budget by at most two data points per run when a curve contains very many gaps. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/LineCurveGapFilter.hpp`

