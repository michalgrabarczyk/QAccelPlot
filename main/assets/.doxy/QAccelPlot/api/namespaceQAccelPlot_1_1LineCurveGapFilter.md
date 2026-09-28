








# Namespace QAccelPlot::LineCurveGapFilter



[**Namespace List**](namespaces.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**LineCurveGapFilter**](namespaceQAccelPlot_1_1LineCurveGapFilter.md)



_Stateless helpers implementing the invalid-sample contract shared by_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _subsystems._[More...](#detailed-description)








































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**appendEnvelopeSamples**](#function-appendenvelopesamples) (const [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) & data, const [**SampleRun**](structQAccelPlot_1_1SampleRun.md) & run, int sampledCount, std::vector&lt; int &gt; & indices) <br>_Appends_ _sampledCount_ _source indices of__run_ _in__data_ _to__indices_ _, in index order._ |
|  int | [**compactValidPoints**](#function-compactvalidpoints) (const std::vector&lt; float &gt; & data, int pointCount, bool logScaleX, bool logScaleY, std::vector&lt; float &gt; & output) <br>_Copies the valid samples of interleaved float_ _data_ _into__output_ _._ |
|  int | [**compactValidPoints**](#function-compactvalidpoints) (const std::vector&lt; double &gt; & data, const std::vector&lt; float &gt; & renderData, int pointCount, bool logScaleX, bool logScaleY, std::vector&lt; double &gt; & output, std::vector&lt; float &gt; & renderOutput) <br>_Copies the valid samples of interleaved double_ _data_ _into__output_ _and the matching entries of the parallel float__renderData_ _into__renderOutput_ _._ |
|  int | [**countInvalidPoints**](#function-countinvalidpoints) (const [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) & data, int pointCount, bool logScaleX, bool logScaleY) <br>_Returns the number of invalid samples among the first_ _pointCount_ _samples of__data_ _._ |
|  std::vector&lt; [**SampleRun**](structQAccelPlot_1_1SampleRun.md) &gt; | [**findValidRuns**](#function-findvalidruns) (const [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) & data, int pointCount, bool logScaleX, bool logScaleY) <br>_Returns the maximal runs of consecutive valid samples in_ _data_ _._ |
|  bool | [**isValidPoint**](#function-isvalidpoint) (const [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) & data, int index, bool logScaleX, bool logScaleY) <br>_Returns_ `true` _when both coordinates of sample__index_ _in__data_ _are valid._ |
|  std::vector&lt; int &gt; | [**planRunSampling**](#function-planrunsampling) (const std::vector&lt; [**SampleRun**](structQAccelPlot_1_1SampleRun.md) &gt; & runs, int maxSampledPoints) <br>_Distributes a budget of_ _maxSampledPoints_ _across__runs_ _._ |




























## Detailed Description


A sample is invalid when either coordinate is non-finite, or is not strictly positive on a logarithmic dimension. See `isValidSample()` in `MathUtils.hpp`. 


    
## Public Functions Documentation





### function appendEnvelopeSamples {#function-appendenvelopesamples}

_Appends_ _sampledCount_ _source indices of__run_ _in__data_ _to__indices_ _, in index order._
```C++
void QAccelPlot::LineCurveGapFilter::appendEnvelopeSamples (
    const CurveDataView & data,
    const SampleRun & run,
    int sampledCount,
    std::vector< int > & indices
) 
```



Keeps every sample when _sampledCount_ covers the run. Otherwise keeps the run's endpoints and the lowest and highest Y sample of equally sized buckets, so the decimated curve keeps its peaks. 


        

<hr>




### function compactValidPoints {#function-compactvalidpoints}

_Copies the valid samples of interleaved float_ _data_ _into__output_ _._
```C++
int QAccelPlot::LineCurveGapFilter::compactValidPoints (
    const std::vector< float > & data,
    int pointCount,
    bool logScaleX,
    bool logScaleY,
    std::vector< float > & output
) 
```



Returns the number of samples written. _output_ is resized to exactly that many XY pairs. 


        

<hr>




### function compactValidPoints {#function-compactvalidpoints}

_Copies the valid samples of interleaved double_ _data_ _into__output_ _and the matching entries of the parallel float__renderData_ _into__renderOutput_ _._
```C++
int QAccelPlot::LineCurveGapFilter::compactValidPoints (
    const std::vector< double > & data,
    const std::vector< float > & renderData,
    int pointCount,
    bool logScaleX,
    bool logScaleY,
    std::vector< double > & output,
    std::vector< float > & renderOutput
) 
```



Validity is decided from the double values. Returns the number of samples written. 


        

<hr>




### function countInvalidPoints {#function-countinvalidpoints}

_Returns the number of invalid samples among the first_ _pointCount_ _samples of__data_ _._
```C++
int QAccelPlot::LineCurveGapFilter::countInvalidPoints (
    const CurveDataView & data,
    int pointCount,
    bool logScaleX,
    bool logScaleY
) 
```




<hr>




### function findValidRuns {#function-findvalidruns}

_Returns the maximal runs of consecutive valid samples in_ _data_ _._
```C++
std::vector< SampleRun > QAccelPlot::LineCurveGapFilter::findValidRuns (
    const CurveDataView & data,
    int pointCount,
    bool logScaleX,
    bool logScaleY
) 
```




<hr>




### function isValidPoint {#function-isvalidpoint}

_Returns_ `true` _when both coordinates of sample__index_ _in__data_ _are valid._
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
    const std::vector< SampleRun > & runs,
    int maxSampledPoints
) 
```



Returns one sampled count per run. Runs with fewer than two samples receive zero (they cannot form an area). Every other run keeps at least its first and last sample, so the total may exceed the budget by at most two samples per run when a curve contains very many gaps. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/LineCurveGapFilter.hpp`

