








# Struct QAccelPlot::InspectionMetric



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**InspectionMetric**](structQAccelPlot_1_1InspectionMetric.md)



_Pixel mapping of a series' plot area, used to measure on-screen distances._ 

* `#include <InspectionTypes.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  double | [**height**](#variable-height)   = `{0.0}`<br> |
|  double | [**width**](#variable-width)   = `{0.0}`<br> |
|  [**AxisMapping**](structQAccelPlot_1_1AxisMapping.md) | [**x**](#variable-x)  <br> |
|  [**AxisMapping**](structQAccelPlot_1_1AxisMapping.md) | [**y**](#variable-y)  <br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  double | [**coord**](#function-coord) (InspectionAxis axis, double pixel) noexcept const<br>_Maps a series-local_ _pixel_ _on__axis_ _to a data value._ |
|  double | [**pixel**](#function-pixel) (InspectionAxis axis, double value) noexcept const<br>_Maps a data_ _value_ _on__axis_ _to a series-local pixel._ |
|  double | [**pixelX**](#function-pixelx) (double value) noexcept const<br> |
|  double | [**pixelY**](#function-pixely) (double value) noexcept const<br> |




























## Public Attributes Documentation





### variable height {#variable-height}

```C++
double QAccelPlot::InspectionMetric::height;
```




<hr>




### variable width {#variable-width}

```C++
double QAccelPlot::InspectionMetric::width;
```




<hr>




### variable x {#variable-x}

```C++
AxisMapping QAccelPlot::InspectionMetric::x;
```




<hr>




### variable y {#variable-y}

```C++
AxisMapping QAccelPlot::InspectionMetric::y;
```




<hr>
## Public Functions Documentation





### function coord {#function-coord}

_Maps a series-local_ _pixel_ _on__axis_ _to a data value._
```C++
double QAccelPlot::InspectionMetric::coord (
    InspectionAxis axis,
    double pixel
) noexcept const
```




<hr>




### function pixel {#function-pixel}

_Maps a data_ _value_ _on__axis_ _to a series-local pixel._
```C++
double QAccelPlot::InspectionMetric::pixel (
    InspectionAxis axis,
    double value
) noexcept const
```




<hr>




### function pixelX {#function-pixelx}

```C++
double QAccelPlot::InspectionMetric::pixelX (
    double value
) noexcept const
```




<hr>




### function pixelY {#function-pixely}

```C++
double QAccelPlot::InspectionMetric::pixelY (
    double value
) noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/internal/InspectionTypes.hpp`

