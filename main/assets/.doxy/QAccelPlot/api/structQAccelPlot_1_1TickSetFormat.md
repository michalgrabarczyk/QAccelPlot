








# Struct QAccelPlot::TickSetFormat



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**TickSetFormat**](structQAccelPlot_1_1TickSetFormat.md)



_What a formatter decided for a whole_ `TickSet` _that a single value formatted later must follow._[More...](#detailed-description)

* `#include <TickLabelFormatter.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**prefixExponent**](#variable-prefixexponent)   = `{0}`<br>_Power of ten of the unit prefix shared by the tick labels; 0 for none._  |












































## Detailed Description


The axis keeps it with its latest ticks and passes it to `TickLabelFormatter::formatValue()`. 


    
## Public Attributes Documentation





### variable prefixExponent {#variable-prefixexponent}

_Power of ten of the unit prefix shared by the tick labels; 0 for none._ 
```C++
int QAccelPlot::TickSetFormat::prefixExponent;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/formatters/TickLabelFormatter.hpp`

