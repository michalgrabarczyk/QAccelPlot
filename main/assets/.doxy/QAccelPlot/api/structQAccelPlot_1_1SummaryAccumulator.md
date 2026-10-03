








# Struct QAccelPlot::SummaryAccumulator



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**SummaryAccumulator**](structQAccelPlot_1_1SummaryAccumulator.md)



_Running sample-weighted Y statistics that can be merged across disjoint sample sets._ 

* `#include <InspectionTypes.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**count**](#variable-count)   = `{0}`<br> |
|  double | [**m2**](#variable-m2)   = `{0.0}`<br> |
|  double | [**maximum**](#variable-maximum)   = `{std::numeric\_limits&lt;double&gt;::quiet\_NaN()}`<br> |
|  int | [**maximumIndex**](#variable-maximumindex)   = `{-1}`<br> |
|  double | [**mean**](#variable-mean)   = `{0.0}`<br> |
|  double | [**minimum**](#variable-minimum)   = `{std::numeric\_limits&lt;double&gt;::quiet\_NaN()}`<br> |
|  int | [**minimumIndex**](#variable-minimumindex)   = `{-1}`<br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**add**](#function-add) (double y, int index) noexcept<br> |
|  void | [**merge**](#function-merge) (const [**SummaryAccumulator**](structQAccelPlot_1_1SummaryAccumulator.md) & other) noexcept<br> |
|  double | [**standardDeviation**](#function-standarddeviation) () noexcept const<br> |




























## Public Attributes Documentation





### variable count {#variable-count}

```C++
int QAccelPlot::SummaryAccumulator::count;
```




<hr>




### variable m2 {#variable-m2}

```C++
double QAccelPlot::SummaryAccumulator::m2;
```




<hr>




### variable maximum {#variable-maximum}

```C++
double QAccelPlot::SummaryAccumulator::maximum;
```




<hr>




### variable maximumIndex {#variable-maximumindex}

```C++
int QAccelPlot::SummaryAccumulator::maximumIndex;
```




<hr>




### variable mean {#variable-mean}

```C++
double QAccelPlot::SummaryAccumulator::mean;
```




<hr>




### variable minimum {#variable-minimum}

```C++
double QAccelPlot::SummaryAccumulator::minimum;
```




<hr>




### variable minimumIndex {#variable-minimumindex}

```C++
int QAccelPlot::SummaryAccumulator::minimumIndex;
```




<hr>
## Public Functions Documentation





### function add {#function-add}

```C++
void QAccelPlot::SummaryAccumulator::add (
    double y,
    int index
) noexcept
```




<hr>




### function merge {#function-merge}

```C++
void QAccelPlot::SummaryAccumulator::merge (
    const SummaryAccumulator & other
) noexcept
```




<hr>




### function standardDeviation {#function-standarddeviation}

```C++
double QAccelPlot::SummaryAccumulator::standardDeviation () noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/internal/InspectionTypes.hpp`

