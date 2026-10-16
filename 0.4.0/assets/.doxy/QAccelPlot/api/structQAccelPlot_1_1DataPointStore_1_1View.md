








# Struct QAccelPlot::DataPointStore::View



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**DataPointStore**](classQAccelPlot_1_1DataPointStore.md) **>** [**View**](structQAccelPlot_1_1DataPointStore_1_1View.md)



_Read-only view over interleaved data points in float or double precision._ [More...](#detailed-description)

* `#include <DataPointStore.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**count**](#variable-count)   = `{0}`<br>_Number of data points._  |
|  const float \* | [**floats**](#variable-floats)   = `{nullptr}`<br>_Float data points, or_ `nullptr` _when_`precise` _is set._ |
|  const double \* | [**precise**](#variable-precise)   = `{nullptr}`<br>_Double data points, or_ `nullptr` _when_`floats` _is set._ |
|  int | [**stride**](#variable-stride)   = `{0}`<br>_Values per data point in the set buffer._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  double | [**value**](#function-value) (int index, int column) noexcept const<br>_Returns value_ _column_ _of data point__index_ _without narrowing double data._ |




























## Detailed Description


Only valid until the next change of the store. 


    
## Public Attributes Documentation





### variable count {#variable-count}

_Number of data points._ 
```C++
int QAccelPlot::DataPointStore::View::count;
```




<hr>




### variable floats {#variable-floats}

_Float data points, or_ `nullptr` _when_`precise` _is set._
```C++
const float* QAccelPlot::DataPointStore::View::floats;
```




<hr>




### variable precise {#variable-precise}

_Double data points, or_ `nullptr` _when_`floats` _is set._
```C++
const double* QAccelPlot::DataPointStore::View::precise;
```




<hr>




### variable stride {#variable-stride}

_Values per data point in the set buffer._ 
```C++
int QAccelPlot::DataPointStore::View::stride;
```




<hr>
## Public Functions Documentation





### function value {#function-value}

_Returns value_ _column_ _of data point__index_ _without narrowing double data._
```C++
double QAccelPlot::DataPointStore::View::value (
    int index,
    int column
) noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/DataPointStore.hpp`

