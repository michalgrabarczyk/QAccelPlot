








# Struct QAccelPlot::PlotSeries::DataPointInput



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**PlotSeries**](classQAccelPlot_1_1PlotSeries.md) **>** [**DataPointInput**](structQAccelPlot_1_1PlotSeries_1_1DataPointInput.md)



_Data points passed to_ `setData()` _or_`setDataF()` _, already checked._[More...](#detailed-description)

* `#include <PlotSeries.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**count**](#variable-count)   = `{0}`<br>_Number of data points._  |
|  const double \* | [**doubleArray**](#variable-doublearray)   = `{nullptr}`<br>_Double values to copy._  |
|  std::vector&lt; double &gt; | [**doubles**](#variable-doubles)  <br>_Moved double values, when_ `precise` _is set and_`doubleArray` _is null._ |
|  const float \* | [**floatArray**](#variable-floatarray)   = `{nullptr}`<br>_Float values to copy._  |
|  std::vector&lt; float &gt; | [**floats**](#variable-floats)  <br>_Moved float values, when_ `precise` _is not set and_`floatArray` _is null._ |
|  bool | [**precise**](#variable-precise)   = `{true}`<br>_Whether the values are doubles._  |
|  int | [**stride**](#variable-stride)   = `{0}`<br>_Values per data point._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  std::vector&lt; double &gt; | [**toDoubles**](#function-todoubles) () <br>_Returns the values as doubles, moved, copied, or converted._  |




























## Detailed Description


Holds either a moved buffer or an array to copy, of double or float values. 


    
## Public Attributes Documentation





### variable count {#variable-count}

_Number of data points._ 
```C++
int QAccelPlot::PlotSeries::DataPointInput::count;
```




<hr>




### variable doubleArray {#variable-doublearray}

_Double values to copy._ 
```C++
const double* QAccelPlot::PlotSeries::DataPointInput::doubleArray;
```




<hr>




### variable doubles {#variable-doubles}

_Moved double values, when_ `precise` _is set and_`doubleArray` _is null._
```C++
std::vector<double> QAccelPlot::PlotSeries::DataPointInput::doubles;
```




<hr>




### variable floatArray {#variable-floatarray}

_Float values to copy._ 
```C++
const float* QAccelPlot::PlotSeries::DataPointInput::floatArray;
```




<hr>




### variable floats {#variable-floats}

_Moved float values, when_ `precise` _is not set and_`floatArray` _is null._
```C++
std::vector<float> QAccelPlot::PlotSeries::DataPointInput::floats;
```




<hr>




### variable precise {#variable-precise}

_Whether the values are doubles._ 
```C++
bool QAccelPlot::PlotSeries::DataPointInput::precise;
```




<hr>




### variable stride {#variable-stride}

_Values per data point._ 
```C++
int QAccelPlot::PlotSeries::DataPointInput::stride;
```




<hr>
## Public Functions Documentation





### function toDoubles {#function-todoubles}

_Returns the values as doubles, moved, copied, or converted._ 
```C++
std::vector< double > QAccelPlot::PlotSeries::DataPointInput::toDoubles () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/PlotSeries.hpp`

