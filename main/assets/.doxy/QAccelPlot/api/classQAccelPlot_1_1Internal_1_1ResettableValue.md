








# Class QAccelPlot::Internal::ResettableValue

**template &lt;typename T&gt;**



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**ResettableValue**](classQAccelPlot_1_1Internal_1_1ResettableValue.md)



_A property value that is either set explicitly or follows a default._ [More...](#detailed-description)

* `#include <ResettableValue.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**ResettableValue**](#function-resettablevalue) (T fallback=T{}) <br>_Constructs an unset value that follows_ _fallback_ _._ |
|  bool | [**isSet**](#function-isset) () const<br>_Returns whether a value is set explicitly._  |
|  bool | [**reset**](#function-reset) () <br>_Unsets the value, so it follows the default again; returns whether the value in effect changed._  |
|  bool | [**set**](#function-set) (const T & value) <br>_Sets the value to_ _value_ _; returns whether the value in effect changed._ |
|  bool | [**setFallback**](#function-setfallback) (const T & fallback) <br>_Sets the default to_ _fallback_ _; returns whether the value in effect changed._ |
|  const T & | [**value**](#function-value) () const<br>_Returns the explicit value when set, otherwise the default._  |




























## Detailed Description


Backs a QML property with a `RESET` function: `value()` is the value in effect, and each mutator returns whether it changed, so the owner knows when to emit the change signal. 


    
## Public Functions Documentation





### function ResettableValue {#function-resettablevalue}

_Constructs an unset value that follows_ _fallback_ _._
```C++
inline explicit QAccelPlot::Internal::ResettableValue::ResettableValue (
    T fallback=T{}
) 
```




<hr>




### function isSet {#function-isset}

_Returns whether a value is set explicitly._ 
```C++
inline bool QAccelPlot::Internal::ResettableValue::isSet () const
```




<hr>




### function reset {#function-reset}

_Unsets the value, so it follows the default again; returns whether the value in effect changed._ 
```C++
inline bool QAccelPlot::Internal::ResettableValue::reset () 
```




<hr>




### function set {#function-set}

_Sets the value to_ _value_ _; returns whether the value in effect changed._
```C++
inline bool QAccelPlot::Internal::ResettableValue::set (
    const T & value
) 
```




<hr>




### function setFallback {#function-setfallback}

_Sets the default to_ _fallback_ _; returns whether the value in effect changed._
```C++
inline bool QAccelPlot::Internal::ResettableValue::setFallback (
    const T & fallback
) 
```




<hr>




### function value {#function-value}

_Returns the explicit value when set, otherwise the default._ 
```C++
inline const T & QAccelPlot::Internal::ResettableValue::value () const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/internal/ResettableValue.hpp`

