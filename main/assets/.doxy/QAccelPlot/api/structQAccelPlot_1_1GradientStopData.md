








# Struct QAccelPlot::GradientStopData



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**GradientStopData**](structQAccelPlot_1_1GradientStopData.md)



_A single color stop within a gradient definition._ 

* `#include <GradientColorTypes.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  QColor | [**color**](#variable-color)   = `{Qt::transparent}`<br>_Color at this stop._  |
|  float | [**position**](#variable-position)   = `{0.0f}`<br>_Normalized stop position in [0, 1]._  |












































## Public Attributes Documentation





### variable color {#variable-color}

_Color at this stop._ 
```C++
QColor QAccelPlot::GradientStopData::color;
```




<hr>




### variable position {#variable-position}

_Normalized stop position in [0, 1]._ 
```C++
float QAccelPlot::GradientStopData::position;
```




<hr>## Friends Documentation






### friend operator!= {#friend-operator}

_Returns_ `true` _if the stops differ in position or color._
```C++
inline bool QAccelPlot::GradientStopData::operator!= (
    const GradientStopData & lhs,
    const GradientStopData & rhs
) 
```




<hr>




### friend operator== {#friend-operator}

_Returns_ `true` _if both stops have the same position and color._
```C++
inline bool QAccelPlot::GradientStopData::operator== (
    const GradientStopData & lhs,
    const GradientStopData & rhs
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/effects/GradientColorTypes.hpp`

