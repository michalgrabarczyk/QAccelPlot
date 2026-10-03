








# Struct QAccelPlot::InspectionSource



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**InspectionSource**](structQAccelPlot_1_1InspectionSource.md)



_Read-only view over the XY records a series exposes to inspection queries._ [More...](#detailed-description)

* `#include <InspectionTypes.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**count**](#variable-count)   = `{-1}`<br>_Number of records, or -1 when the series has no XY records._  |
|  const double \* | [**doubles**](#variable-doubles)   = `{nullptr}`<br>_Interleaved double-precision XY pairs; takes precedence over_ `floats` _._ |
|  int | [**floatStride**](#variable-floatstride)   = `{2}`<br>_Floats per record in_ `floats` _._ |
|  const float \* | [**floats**](#variable-floats)   = `{nullptr}`<br>_Interleaved single-precision records; X and Y lead each record._  |
|  bool | [**logX**](#variable-logx)   = `{false}`<br>_Whether X is on a logarithmic axis._  |
|  bool | [**logY**](#variable-logy)   = `{false}`<br>_Whether Y is on a logarithmic axis._  |
|  int | [**valueStride**](#variable-valuestride)   = `{1}`<br>_Floats between consecutive scalars in_ `values` _._ |
|  const float \* | [**values**](#variable-values)   = `{nullptr}`<br>_Optional per-record scalar._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  double | [**coordinate**](#function-coordinate) (InspectionAxis axis, int index) noexcept const<br>_Returns the coordinate of record_ _index_ _along__axis_ _._ |
|  bool | [**supported**](#function-supported) () noexcept const<br> |
|  bool | [**valid**](#function-valid) (int index) noexcept const<br>_Applies the invalid-sample contract shared with rendering._  |
|  double | [**value**](#function-value) (int index) noexcept const<br> |
|  double | [**x**](#function-x) (int index) noexcept const<br> |
|  double | [**y**](#function-y) (int index) noexcept const<br> |




























## Detailed Description


The pointers refer to the series' own buffers and are only valid until its next data change. 


    
## Public Attributes Documentation





### variable count {#variable-count}

_Number of records, or -1 when the series has no XY records._ 
```C++
int QAccelPlot::InspectionSource::count;
```




<hr>




### variable doubles {#variable-doubles}

_Interleaved double-precision XY pairs; takes precedence over_ `floats` _._
```C++
const double* QAccelPlot::InspectionSource::doubles;
```




<hr>




### variable floatStride {#variable-floatstride}

_Floats per record in_ `floats` _._
```C++
int QAccelPlot::InspectionSource::floatStride;
```




<hr>




### variable floats {#variable-floats}

_Interleaved single-precision records; X and Y lead each record._ 
```C++
const float* QAccelPlot::InspectionSource::floats;
```




<hr>




### variable logX {#variable-logx}

_Whether X is on a logarithmic axis._ 
```C++
bool QAccelPlot::InspectionSource::logX;
```




<hr>




### variable logY {#variable-logy}

_Whether Y is on a logarithmic axis._ 
```C++
bool QAccelPlot::InspectionSource::logY;
```




<hr>




### variable valueStride {#variable-valuestride}

_Floats between consecutive scalars in_ `values` _._
```C++
int QAccelPlot::InspectionSource::valueStride;
```




<hr>




### variable values {#variable-values}

_Optional per-record scalar._ 
```C++
const float* QAccelPlot::InspectionSource::values;
```




<hr>
## Public Functions Documentation





### function coordinate {#function-coordinate}

_Returns the coordinate of record_ _index_ _along__axis_ _._
```C++
double QAccelPlot::InspectionSource::coordinate (
    InspectionAxis axis,
    int index
) noexcept const
```




<hr>




### function supported {#function-supported}

```C++
bool QAccelPlot::InspectionSource::supported () noexcept const
```




<hr>




### function valid {#function-valid}

_Applies the invalid-sample contract shared with rendering._ 
```C++
bool QAccelPlot::InspectionSource::valid (
    int index
) noexcept const
```




<hr>




### function value {#function-value}

```C++
double QAccelPlot::InspectionSource::value (
    int index
) noexcept const
```




<hr>




### function x {#function-x}

```C++
double QAccelPlot::InspectionSource::x (
    int index
) noexcept const
```




<hr>




### function y {#function-y}

```C++
double QAccelPlot::InspectionSource::y (
    int index
) noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/internal/InspectionTypes.hpp`

