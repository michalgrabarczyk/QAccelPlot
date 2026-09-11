








# Struct QAccelPlot::DashParameters



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**DashParameters**](structQAccelPlot_1_1DashParameters.md)



_Plain-data snapshot of dash rendering parameters._ [More...](#detailed-description)

* `#include <LineStyle.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**enabled**](#variable-enabled)   = `{false}`<br>`true` _when dashed rendering is active._ |
|  float | [**offset**](#variable-offset)   = `{0.0f}`<br>_Phase offset into the dash pattern in pixels._  |
|  float | [**pattern**](#variable-pattern)   = `{}`<br>_Alternating dash/gap lengths (up to 8 entries)._  |
|  int | [**patternSize**](#variable-patternsize)   = `{0}`<br>_Number of valid entries in_ `pattern` _._ |
|  float | [**period**](#variable-period)   = `{0.0f}`<br>_Total dash+gap cycle length in pixels._  |












































## Detailed Description


Extracted from a `DashLine` on the main thread so the line renderer never needs to touch the QObject hierarchy directly. 


    
## Public Attributes Documentation





### variable enabled {#variable-enabled}

`true` _when dashed rendering is active._
```C++
bool QAccelPlot::DashParameters::enabled;
```




<hr>




### variable offset {#variable-offset}

_Phase offset into the dash pattern in pixels._ 
```C++
float QAccelPlot::DashParameters::offset;
```




<hr>




### variable pattern {#variable-pattern}

_Alternating dash/gap lengths (up to 8 entries)._ 
```C++
float QAccelPlot::DashParameters::pattern[8];
```




<hr>




### variable patternSize {#variable-patternsize}

_Number of valid entries in_ `pattern` _._
```C++
int QAccelPlot::DashParameters::patternSize;
```




<hr>




### variable period {#variable-period}

_Total dash+gap cycle length in pixels._ 
```C++
float QAccelPlot::DashParameters::period;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/linestyles/LineStyle.hpp`

