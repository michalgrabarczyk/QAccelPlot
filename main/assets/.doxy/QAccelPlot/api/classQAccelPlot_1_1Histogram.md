








# Class QAccelPlot::Histogram



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Histogram**](classQAccelPlot_1_1Histogram.md)



_Samples counted into bins, convertible to_ `BarSeries` _data._[More...](#detailed-description)

* `#include <Histogram.hpp>`































## Public Properties

| Type | Name |
| ---: | :--- |
| property QML\_ANONYMOUSint | [**binCount**](classQAccelPlot_1_1Histogram.md#property-bincount-12)  <br>_Read-only: number of bins; 0 for an invalid histogram._  |
| property qreal | [**binWidth**](classQAccelPlot_1_1Histogram.md#property-binwidth-12)  <br>_Read-only: the width of every bin, or NaN when the bins are not uniform._  |
| property QList&lt; qreal &gt; | [**edges**](classQAccelPlot_1_1Histogram.md#property-edges-12)  <br>_Read-only: the_ `binCount` _+ 1 bin edges in ascending order._ |
| property qreal | [**total**](classQAccelPlot_1_1Histogram.md#property-total-12)  <br>_Read-only: sum of the bin counts, the number of samples inside the edges._  |
| property bool | [**uniform**](classQAccelPlot_1_1Histogram.md#property-uniform)  <br>_Read-only: true when all bins have the same width._  |
| property QList&lt; qreal &gt; | [**values**](classQAccelPlot_1_1Histogram.md#property-values-12)  <br>_Read-only: one value per bin, a count or, after_ `density()` _, a density._ |








## Public Functions

| Type | Name |
| ---: | :--- |
|  std::vector&lt; double &gt; | [**barData**](#function-bardata) () const<br>_Returns_ `binCount()` _interleaved (from, to, value) bars, one per bin, for_`BarSeries::setRangedData()` _._ |
|  Q\_INVOKABLE QVariantList | [**bars**](#function-bars) () const<br>_Returns one object per bin with_ `from` _,_`to` _, and_`value` _, for the QML_`BarSeries.setData()` _._ |
|  int | [**binCount**](#function-bincount-22) () const<br>_Returns the number of bins; 0 for an invalid histogram._  |
|  double | [**binWidth**](#function-binwidth-22) () const<br>_Returns the width of every bin, or NaN when the bins are not uniform._  |
|  Q\_INVOKABLE::QAccelPlot::Histogram | [**density**](#function-density) () const<br>_Returns a copy whose values are densities: count / (_ `total()` _× bin width)._ |
|  const std::vector&lt; double &gt; & | [**edges**](#function-edges-22) () const<br>_Returns the_ `binCount()` _+ 1 bin edges in ascending order._ |
|  bool | [**isUniform**](#function-isuniform) () const<br>_Returns true when all bins have the same width._  |
|  double | [**total**](#function-total-22) () const<br>_Returns the sum of the bin counts, the number of samples inside the edges._  |
|  const std::vector&lt; double &gt; & | [**values**](#function-values-22) () const<br>_Returns one value per bin, a count or, after_ `density()` _, a density._ |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  std::vector&lt; double &gt; | [**decadeEdges**](#function-decadeedges) (double min, double max) <br>_Returns the edges from_ _min_ _to__max_ _at 1, 2, ..., 9 times each power of ten: the ticks of a logarithmic axis._ |
|  [**Histogram**](classQAccelPlot_1_1Histogram.md) | [**fromCounts**](#function-fromcounts) (std::vector&lt; double &gt; edges, std::vector&lt; double &gt; counts) <br>_Wraps data that is already binned:_ _counts_ _has one finite, non-negative value per bin between__edges_ _._ |
|  [**Histogram**](classQAccelPlot_1_1Histogram.md) | [**fromSamples**](#function-fromsamples-16) (const double \* samples, std::size\_t sampleCount, int binCount) <br>_Counts_ _sampleCount_ ___samples_ _into__binCount_ _equal bins spanning the finite samples._ |
|  [**Histogram**](classQAccelPlot_1_1Histogram.md) | [**fromSamples**](#function-fromsamples-26) (const double \* samples, std::size\_t sampleCount, int binCount, double min, double max) <br>_Counts_ _sampleCount_ ___samples_ _into__binCount_ _equal bins from__min_ _to__max_ _._ |
|  [**Histogram**](classQAccelPlot_1_1Histogram.md) | [**fromSamples**](#function-fromsamples-36) (const double \* samples, std::size\_t sampleCount, std::vector&lt; double &gt; edges) <br>_Counts_ _sampleCount_ ___samples_ _into the bins between__edges_ _, which must be finite and strictly increasing._ |
|  [**Histogram**](classQAccelPlot_1_1Histogram.md) | [**fromSamples**](#function-fromsamples-46) (const float \* samples, std::size\_t sampleCount, int binCount) <br>_Float overload of_ `fromSamples` _(__samples_ _,__sampleCount_ _,__binCount_ _)._ |
|  [**Histogram**](classQAccelPlot_1_1Histogram.md) | [**fromSamples**](#function-fromsamples-56) (const float \* samples, std::size\_t sampleCount, int binCount, double min, double max) <br>_Float overload of_ `fromSamples` _(__samples_ _,__sampleCount_ _,__binCount_ _,__min_ _,__max_ _)._ |
|  [**Histogram**](classQAccelPlot_1_1Histogram.md) | [**fromSamples**](#function-fromsamples-66) (const float \* samples, std::size\_t sampleCount, std::vector&lt; double &gt; edges) <br>_Float overload of_ `fromSamples` _(__samples_ _,__sampleCount_ _,__edges_ _)._ |
|  std::vector&lt; double &gt; | [**linearEdges**](#function-linearedges) (double min, double max, int binCount) <br>_Returns_ _binCount_ _+ 1 equally spaced edges from__min_ _to__max_ _, or an empty vector for invalid arguments._ |
|  std::vector&lt; double &gt; | [**logEdges**](#function-logedges) (double min, double max, int binCount) <br>_Returns_ _binCount_ _+ 1 edges from__min_ _to__max_ _, equally spaced on a logarithmic axis._ |


























## Detailed Description


A bin spans from its lower edge up to, but not including, its upper edge; the last bin also includes its upper edge. NaN samples and samples outside the edges are not counted.


The functions are stateless, so a histogram can be built on a worker thread and its data passed to `postData()`.



```C++
const auto histogram = Histogram::fromSamples(samples.data(), samples.size(), 40);
bars->setRangedData(histogram.barData(), histogram.binCount());
```



In QML, histograms are created by the `Histogram` singleton, see [**HistogramFactory**](classQAccelPlot_1_1HistogramFactory.md).




**See also:** [**HistogramFactory**](classQAccelPlot_1_1HistogramFactory.md), [**BarSeries**](classQAccelPlot_1_1BarSeries.md) 



    
## Public Properties Documentation





### property binCount {#property-bincount-12}

_Read-only: number of bins; 0 for an invalid histogram._ 
```C++
QML_ANONYMOUSint QAccelPlot::Histogram::binCount;
```




<hr>




### property binWidth {#property-binwidth-12}

_Read-only: the width of every bin, or NaN when the bins are not uniform._ 
```C++
qreal QAccelPlot::Histogram::binWidth;
```




<hr>




### property edges {#property-edges-12}

_Read-only: the_ `binCount` _+ 1 bin edges in ascending order._
```C++
QList<qreal> QAccelPlot::Histogram::edges;
```




<hr>




### property total {#property-total-12}

_Read-only: sum of the bin counts, the number of samples inside the edges._ 
```C++
qreal QAccelPlot::Histogram::total;
```




<hr>




### property uniform {#property-uniform}

_Read-only: true when all bins have the same width._ 
```C++
bool QAccelPlot::Histogram::uniform;
```




<hr>




### property values {#property-values-12}

_Read-only: one value per bin, a count or, after_ `density()` _, a density._
```C++
QList<qreal> QAccelPlot::Histogram::values;
```




<hr>
## Public Functions Documentation





### function barData {#function-bardata}

_Returns_ `binCount()` _interleaved (from, to, value) bars, one per bin, for_`BarSeries::setRangedData()` _._
```C++
std::vector< double > QAccelPlot::Histogram::barData () const
```




<hr>




### function bars {#function-bars}

_Returns one object per bin with_ `from` _,_`to` _, and_`value` _, for the QML_`BarSeries.setData()` _._
```C++
Q_INVOKABLE QVariantList QAccelPlot::Histogram::bars () const
```




<hr>




### function binCount {#function-bincount-22}

_Returns the number of bins; 0 for an invalid histogram._ 
```C++
int QAccelPlot::Histogram::binCount () const
```




<hr>




### function binWidth {#function-binwidth-22}

_Returns the width of every bin, or NaN when the bins are not uniform._ 
```C++
double QAccelPlot::Histogram::binWidth () const
```




<hr>




### function density {#function-density}

_Returns a copy whose values are densities: count / (_ `total()` _× bin width)._
```C++
Q_INVOKABLE::QAccelPlot::Histogram QAccelPlot::Histogram::density () const
```



The bin areas sum to 1, so bins of different widths and data sets of different sizes are comparable. Returns the histogram unchanged when it already holds densities. 


        

<hr>




### function edges {#function-edges-22}

_Returns the_ `binCount()` _+ 1 bin edges in ascending order._
```C++
const std::vector< double > & QAccelPlot::Histogram::edges () const
```




<hr>




### function isUniform {#function-isuniform}

_Returns true when all bins have the same width._ 
```C++
bool QAccelPlot::Histogram::isUniform () const
```




<hr>




### function total {#function-total-22}

_Returns the sum of the bin counts, the number of samples inside the edges._ 
```C++
double QAccelPlot::Histogram::total () const
```




<hr>




### function values {#function-values-22}

_Returns one value per bin, a count or, after_ `density()` _, a density._
```C++
const std::vector< double > & QAccelPlot::Histogram::values () const
```




<hr>
## Public Static Functions Documentation





### function decadeEdges {#function-decadeedges}

_Returns the edges from_ _min_ _to__max_ _at 1, 2, ..., 9 times each power of ten: the ticks of a logarithmic axis._
```C++
static std::vector< double > QAccelPlot::Histogram::decadeEdges (
    double min,
    double max
) 
```



_min_ and _max_ are the first and the last edge. _min_ must be positive. Returns an empty vector for invalid arguments. The bins widen tenfold at each power of ten, so draw them as `density()`. 


        

<hr>




### function fromCounts {#function-fromcounts}

_Wraps data that is already binned:_ _counts_ _has one finite, non-negative value per bin between__edges_ _._
```C++
static Histogram QAccelPlot::Histogram::fromCounts (
    std::vector< double > edges,
    std::vector< double > counts
) 
```




<hr>




### function fromSamples {#function-fromsamples-16}

_Counts_ _sampleCount_ ___samples_ _into__binCount_ _equal bins spanning the finite samples._
```C++
static Histogram QAccelPlot::Histogram::fromSamples (
    const double * samples,
    std::size_t sampleCount,
    int binCount
) 
```



The range is 0 to 1 without finite samples, and widens by 0.5 on each side when all samples are equal. 


        

<hr>




### function fromSamples {#function-fromsamples-26}

_Counts_ _sampleCount_ ___samples_ _into__binCount_ _equal bins from__min_ _to__max_ _._
```C++
static Histogram QAccelPlot::Histogram::fromSamples (
    const double * samples,
    std::size_t sampleCount,
    int binCount,
    double min,
    double max
) 
```




<hr>




### function fromSamples {#function-fromsamples-36}

_Counts_ _sampleCount_ ___samples_ _into the bins between__edges_ _, which must be finite and strictly increasing._
```C++
static Histogram QAccelPlot::Histogram::fromSamples (
    const double * samples,
    std::size_t sampleCount,
    std::vector< double > edges
) 
```




<hr>




### function fromSamples {#function-fromsamples-46}

_Float overload of_ `fromSamples` _(__samples_ _,__sampleCount_ _,__binCount_ _)._
```C++
static Histogram QAccelPlot::Histogram::fromSamples (
    const float * samples,
    std::size_t sampleCount,
    int binCount
) 
```




<hr>




### function fromSamples {#function-fromsamples-56}

_Float overload of_ `fromSamples` _(__samples_ _,__sampleCount_ _,__binCount_ _,__min_ _,__max_ _)._
```C++
static Histogram QAccelPlot::Histogram::fromSamples (
    const float * samples,
    std::size_t sampleCount,
    int binCount,
    double min,
    double max
) 
```




<hr>




### function fromSamples {#function-fromsamples-66}

_Float overload of_ `fromSamples` _(__samples_ _,__sampleCount_ _,__edges_ _)._
```C++
static Histogram QAccelPlot::Histogram::fromSamples (
    const float * samples,
    std::size_t sampleCount,
    std::vector< double > edges
) 
```




<hr>




### function linearEdges {#function-linearedges}

_Returns_ _binCount_ _+ 1 equally spaced edges from__min_ _to__max_ _, or an empty vector for invalid arguments._
```C++
static std::vector< double > QAccelPlot::Histogram::linearEdges (
    double min,
    double max,
    int binCount
) 
```




<hr>




### function logEdges {#function-logedges}

_Returns_ _binCount_ _+ 1 edges from__min_ _to__max_ _, equally spaced on a logarithmic axis._
```C++
static std::vector< double > QAccelPlot::Histogram::logEdges (
    double min,
    double max,
    int binCount
) 
```



_min_ must be positive. Returns an empty vector for invalid arguments. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/data/Histogram.hpp`

