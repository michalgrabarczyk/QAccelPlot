








# Struct QAccelPlot::AxisMapping



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**AxisMapping**](structQAccelPlot_1_1AxisMapping.md)



_Snapshot of an axis viewport that maps between data values and pixel positions._ [More...](#detailed-description)

* `#include <AxisMapping.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**flipped**](#variable-flipped)   = `{false}`<br>_Whether pixel positions grow against data values, as on vertical axes._  |
|  bool | [**logarithmic**](#variable-logarithmic)   = `{false}`<br>_Whether values are mapped through_ `log10` _._ |
|  double | [**origin**](#variable-origin)   = `{0.0}`<br>_Viewport minimum in mapped space:_ `log10` _on logarithmic axes._ |
|  double | [**span**](#variable-span)   = `{1.0}`<br>_Viewport extent in mapped space; negative for an inverted viewport._  |
|  bool | [**valid**](#variable-valid)   = `{false}`<br>_False for a collapsed, nonfinite, or nonpositive logarithmic viewport._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  double | [**toCoord**](#function-tocoord) (double pixel, double length) noexcept const<br>_Maps a_ _pixel_ _position along an axis of__length_ _pixels back to a data-space value._ |
|  double | [**toMapped**](#function-tomapped) (double value) noexcept const<br>_Returns_ _value_ _in mapped space._ |
|  double | [**toPixel**](#function-topixel) (double value, double length) noexcept const<br>_Maps a data-space_ _value_ _to a pixel position along an axis of__length_ _pixels._ |




























## Detailed Description


Obtained from `Axis::mapping()`. It is the single implementation of the axis mapping, shared by `Axis::coordToPixel()`, `Axis::pixelToCoord()`, and the inspection queries. 


    
## Public Attributes Documentation





### variable flipped {#variable-flipped}

_Whether pixel positions grow against data values, as on vertical axes._ 
```C++
bool QAccelPlot::AxisMapping::flipped;
```




<hr>




### variable logarithmic {#variable-logarithmic}

_Whether values are mapped through_ `log10` _._
```C++
bool QAccelPlot::AxisMapping::logarithmic;
```




<hr>




### variable origin {#variable-origin}

_Viewport minimum in mapped space:_ `log10` _on logarithmic axes._
```C++
double QAccelPlot::AxisMapping::origin;
```




<hr>




### variable span {#variable-span}

_Viewport extent in mapped space; negative for an inverted viewport._ 
```C++
double QAccelPlot::AxisMapping::span;
```




<hr>




### variable valid {#variable-valid}

_False for a collapsed, nonfinite, or nonpositive logarithmic viewport._ 
```C++
bool QAccelPlot::AxisMapping::valid;
```




<hr>
## Public Functions Documentation





### function toCoord {#function-tocoord}

_Maps a_ _pixel_ _position along an axis of__length_ _pixels back to a data-space value._
```C++
double QAccelPlot::AxisMapping::toCoord (
    double pixel,
    double length
) noexcept const
```




<hr>




### function toMapped {#function-tomapped}

_Returns_ _value_ _in mapped space._
```C++
double QAccelPlot::AxisMapping::toMapped (
    double value
) noexcept const
```




<hr>




### function toPixel {#function-topixel}

_Maps a data-space_ _value_ _to a pixel position along an axis of__length_ _pixels._
```C++
double QAccelPlot::AxisMapping::toPixel (
    double value,
    double length
) noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/axis/AxisMapping.hpp`

