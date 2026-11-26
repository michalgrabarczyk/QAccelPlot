








# Struct QAccelPlot::Internal::UnitPrefix



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**UnitPrefix**](structQAccelPlot_1_1Internal_1_1UnitPrefix.md)



_An SI unit prefix: its output symbol and its power of ten._ 

* `#include <UnitPrefix.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**exponent**](#variable-exponent)   = `{0}`<br>_Power of ten the prefix stands for, such as 3 for "k"._  |
|  QString | [**symbol**](#variable-symbol)  <br>_Output symbol, such as "k" or "µ" (U+00B5); empty for the bare unit._  |












































## Public Attributes Documentation





### variable exponent {#variable-exponent}

_Power of ten the prefix stands for, such as 3 for "k"._ 
```C++
int QAccelPlot::Internal::UnitPrefix::exponent;
```




<hr>




### variable symbol {#variable-symbol}

_Output symbol, such as "k" or "µ" (U+00B5); empty for the bare unit._ 
```C++
QString QAccelPlot::Internal::UnitPrefix::symbol;
```




<hr>## Friends Documentation






### friend operator!= {#friend-operator}

_Returns whether the prefixes differ in symbol or power of ten._ 
```C++
inline bool QAccelPlot::Internal::UnitPrefix::operator!= (
    const UnitPrefix & left,
    const UnitPrefix & right
) 
```




<hr>




### friend operator== {#friend-operator}

_Returns whether both prefixes have the same symbol and power of ten._ 
```C++
inline bool QAccelPlot::Internal::UnitPrefix::operator== (
    const UnitPrefix & left,
    const UnitPrefix & right
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/formatters/internal/UnitPrefix.hpp`

