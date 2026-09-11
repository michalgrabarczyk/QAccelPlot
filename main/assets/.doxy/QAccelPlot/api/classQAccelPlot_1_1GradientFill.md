








# Class QAccelPlot::GradientFill



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**GradientFill**](classQAccelPlot_1_1GradientFill.md)



_A_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _effect that fills the area under the curve with a color gradient._[More...](#detailed-description)

* `#include <GradientFill.hpp>`



Inherits the following classes: [QAccelPlot::LineCurveEffect](classQAccelPlot_1_1LineCurveEffect.md)




## Inheritance diagram

```mermaid
flowchart TB
  classQAccelPlot_1_1GradientFill["QAccelPlot::GradientFill"]

  classQAccelPlot_1_1LineCurveEffect["QAccelPlot::LineCurveEffect"]
  classQAccelPlot_1_1LineCurveEffect --> classQAccelPlot_1_1GradientFill
  click classQAccelPlot_1_1LineCurveEffect "../classQAccelPlot_1_1LineCurveEffect/" "Open QAccelPlot::LineCurveEffect"

  external_base_classQAccelPlot_1_1LineCurveEffect_1["QObject"]
  external_base_classQAccelPlot_1_1LineCurveEffect_1 --> classQAccelPlot_1_1LineCurveEffect

```




































## Public Properties

| Type | Name |
| ---: | :--- |
| property [**GradientFillBaseline**](namespaceQAccelPlot_1_1GradientFillBaselineNS.md#enum-mode) | [**baseline**](classQAccelPlot_1_1GradientFill.md#property-baseline-12)  <br>_Where the filled area's baseline starts (axis minimum or a fixed value)._  |
| property qreal | [**baselineValue**](classQAccelPlot_1_1GradientFill.md#property-baselinevalue-12)  <br>_Fixed baseline data value used when_ `baseline` _is_`GradientFillBaseline.Value` _._ |
| property [**GradientDirection**](namespaceQAccelPlot_1_1GradientDirectionNS.md#enum-direction) | [**direction**](classQAccelPlot_1_1GradientFill.md#property-direction-12)  <br>[_**Axis**_](classQAccelPlot_1_1Axis.md) _along which the gradient color varies (Horizontal or Vertical)._ |
| property QObject \* | [**gradient**](classQAccelPlot_1_1GradientFill.md#property-gradient-12)  <br>_A Qt_ `Gradient` _(or compatible) object supplying the color stops._ |
| property qreal | [**gradientValueMax**](classQAccelPlot_1_1GradientFill.md#property-gradientvaluemax-12)  <br>_Data-space value that maps to gradient position 1.0. Only used when gradientValueMaxSource is Fixed._  |
| property [**GradientValueSource**](namespaceQAccelPlot_1_1GradientValueSourceNS.md#enum-source) | [**gradientValueMaxSource**](classQAccelPlot_1_1GradientFill.md#property-gradientvaluemaxsource-12)  <br>_How the gradient normalization maximum is determined; see_ `GradientValueSource` _._ |
| property qreal | [**gradientValueMin**](classQAccelPlot_1_1GradientFill.md#property-gradientvaluemin-12)  <br>_Data-space value that maps to gradient position 0.0. Only used when gradientValueMinSource is Fixed._  |
| property [**GradientValueSource**](namespaceQAccelPlot_1_1GradientValueSourceNS.md#enum-source) | [**gradientValueMinSource**](classQAccelPlot_1_1GradientFill.md#property-gradientvalueminsource-12)  <br>_How the gradient normalization minimum is determined; see_ `GradientValueSource` _._ |
| property qreal | [**opacity**](classQAccelPlot_1_1GradientFill.md#property-opacity-12)  <br>_Overall opacity of the fill area in [0, 1]. Default: 0.35._  |


## Public Properties inherited from QAccelPlot::LineCurveEffect

See [QAccelPlot::LineCurveEffect](classQAccelPlot_1_1LineCurveEffect.md)

| Type | Name |
| ---: | :--- |
| property QML\_ANONYMOUSbool | [**enabled**](classQAccelPlot_1_1LineCurveEffect.md#property-enabled-12)  <br>_Whether this effect is active. Disabled effects are ignored during rendering._  |






## Public Signals

| Type | Name |
| ---: | :--- |
| signal void | [**baselineChanged**](classQAccelPlot_1_1GradientFill.md#signal-baselinechanged)  <br>_Emitted when the baseline property changes._  |
| signal void | [**baselineValueChanged**](classQAccelPlot_1_1GradientFill.md#signal-baselinevaluechanged)  <br>_Emitted when the baselineValue property changes._  |
| signal void | [**directionChanged**](classQAccelPlot_1_1GradientFill.md#signal-directionchanged)  <br>_Emitted when the direction property changes._  |
| signal void | [**gradientChanged**](classQAccelPlot_1_1GradientFill.md#signal-gradientchanged)  <br>_Emitted when the gradient property changes._  |
| signal void | [**gradientValueMaxChanged**](classQAccelPlot_1_1GradientFill.md#signal-gradientvaluemaxchanged)  <br>_Emitted when the gradientValueMax property changes._  |
| signal void | [**gradientValueMaxSourceChanged**](classQAccelPlot_1_1GradientFill.md#signal-gradientvaluemaxsourcechanged)  <br>_Emitted when the gradientValueMaxSource property changes._  |
| signal void | [**gradientValueMinChanged**](classQAccelPlot_1_1GradientFill.md#signal-gradientvalueminchanged)  <br>_Emitted when the gradientValueMin property changes._  |
| signal void | [**gradientValueMinSourceChanged**](classQAccelPlot_1_1GradientFill.md#signal-gradientvalueminsourcechanged)  <br>_Emitted when the gradientValueMinSource property changes._  |
| signal void | [**opacityChanged**](classQAccelPlot_1_1GradientFill.md#signal-opacitychanged)  <br>_Emitted when the opacity property changes._  |


## Public Signals inherited from QAccelPlot::LineCurveEffect

See [QAccelPlot::LineCurveEffect](classQAccelPlot_1_1LineCurveEffect.md)

| Type | Name |
| ---: | :--- |
| signal void | [**effectChanged**](classQAccelPlot_1_1LineCurveEffect.md#signal-effectchanged)  <br>_Emitted when any effect property changes, requesting a curve redraw._  |
| signal void | [**enabledChanged**](classQAccelPlot_1_1LineCurveEffect.md#signal-enabledchanged)  <br>_Emitted when the enabled property changes._  |






## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**GradientFill**](#function-gradientfill) (QObject \* parent=nullptr) <br>_Constructs a_ [_**GradientFill**_](classQAccelPlot_1_1GradientFill.md) _with the given__parent_ _._ |
|  [**GradientFillBaseline**](namespaceQAccelPlot_1_1GradientFillBaselineNS.md#enum-mode) | [**baseline**](#function-baseline-22) () const<br>_Returns the baseline mode._  |
|  qreal | [**baselineValue**](#function-baselinevalue-22) () const<br>_Returns the fixed baseline data value._  |
|  [**GradientDirection**](namespaceQAccelPlot_1_1GradientDirectionNS.md#enum-direction) | [**direction**](#function-direction-22) () const<br>_Returns the gradient direction._  |
|  QObject \* | [**gradient**](#function-gradient-22) () const<br>_Returns the Qt Gradient object supplying color stops._  |
|  qreal | [**gradientValueMax**](#function-gradientvaluemax-22) () const<br>_Returns the maximum data value for gradient normalization._  |
|  [**GradientValueSource**](namespaceQAccelPlot_1_1GradientValueSourceNS.md#enum-source) | [**gradientValueMaxSource**](#function-gradientvaluemaxsource-22) () const<br>_Returns how the gradient normalization maximum is determined._  |
|  qreal | [**gradientValueMin**](#function-gradientvaluemin-22) () const<br>_Returns the minimum data value for gradient normalization._  |
|  [**GradientValueSource**](namespaceQAccelPlot_1_1GradientValueSourceNS.md#enum-source) | [**gradientValueMinSource**](#function-gradientvalueminsource-22) () const<br>_Returns how the gradient normalization minimum is determined._  |
|  qreal | [**opacity**](#function-opacity-22) () const<br>_Returns the fill opacity._  |
|  [**GradientFillPayload**](structQAccelPlot_1_1GradientFillPayload.md) | [**payload**](#function-payload) () const<br>_Returns a render-thread-safe snapshot of all fill parameters._  |
|  void | [**setBaseline**](#function-setbaseline) ([**GradientFillBaseline**](namespaceQAccelPlot_1_1GradientFillBaselineNS.md#enum-mode) baseline) <br>_Sets the baseline mode to_ _baseline_ _._ |
|  void | [**setBaselineValue**](#function-setbaselinevalue) (qreal value) <br>_Sets the fixed baseline data value to_ _value_ _._ |
|  void | [**setDirection**](#function-setdirection) ([**GradientDirection**](namespaceQAccelPlot_1_1GradientDirectionNS.md#enum-direction) direction) <br>_Sets the gradient direction to_ _direction_ _._ |
|  void | [**setGradient**](#function-setgradient) (QObject \* gradient) <br>_Sets the Qt Gradient object to_ _gradient_ _._ |
|  void | [**setGradientValueMax**](#function-setgradientvaluemax) (qreal value) <br>_Sets the maximum data value for gradient normalization to_ _value_ _._ |
|  void | [**setGradientValueMaxSource**](#function-setgradientvaluemaxsource) ([**GradientValueSource**](namespaceQAccelPlot_1_1GradientValueSourceNS.md#enum-source) source) <br>_Sets the gradient normalization maximum source to_ _source_ _._ |
|  void | [**setGradientValueMin**](#function-setgradientvaluemin) (qreal value) <br>_Sets the minimum data value for gradient normalization to_ _value_ _._ |
|  void | [**setGradientValueMinSource**](#function-setgradientvalueminsource) ([**GradientValueSource**](namespaceQAccelPlot_1_1GradientValueSourceNS.md#enum-source) source) <br>_Sets the gradient normalization minimum source to_ _source_ _._ |
|  void | [**setOpacity**](#function-setopacity) (qreal value) <br>_Sets the fill opacity to_ _value_ _._ |


## Public Functions inherited from QAccelPlot::LineCurveEffect

See [QAccelPlot::LineCurveEffect](classQAccelPlot_1_1LineCurveEffect.md)

| Type | Name |
| ---: | :--- |
|   | [**LineCurveEffect**](classQAccelPlot_1_1LineCurveEffect.md#function-linecurveeffect) (QObject \* parent=nullptr) <br>_Constructs an_ [_**LineCurveEffect**_](classQAccelPlot_1_1LineCurveEffect.md) _with the given__parent_ _._ |
|  bool | [**enabled**](classQAccelPlot_1_1LineCurveEffect.md#function-enabled-22) () const<br>_Returns_ `true` _if the effect is active._ |
|  void | [**setEnabled**](classQAccelPlot_1_1LineCurveEffect.md#function-setenabled) (bool enabled) <br>_Sets the effect's enabled state to_ _enabled_ _._ |






















































## Detailed Description


Attach to `LineCurve::effects` to render a shaded fill between the curve and a configurable baseline. Both the fill direction and color stops are taken from a Qt `Gradient` object assigned to `gradient`.




**See also:** [**GradientStroke**](classQAccelPlot_1_1GradientStroke.md), [**LineCurveEffect**](classQAccelPlot_1_1LineCurveEffect.md), [**LineCurve**](classQAccelPlot_1_1LineCurve.md) 



    
## Public Properties Documentation





### property baseline {#property-baseline-12}

_Where the filled area's baseline starts (axis minimum or a fixed value)._ 
```C++
GradientFillBaseline QAccelPlot::GradientFill::baseline;
```




<hr>




### property baselineValue {#property-baselinevalue-12}

_Fixed baseline data value used when_ `baseline` _is_`GradientFillBaseline.Value` _._
```C++
qreal QAccelPlot::GradientFill::baselineValue;
```




<hr>




### property direction {#property-direction-12}

[_**Axis**_](classQAccelPlot_1_1Axis.md) _along which the gradient color varies (Horizontal or Vertical)._
```C++
GradientDirection QAccelPlot::GradientFill::direction;
```




<hr>




### property gradient {#property-gradient-12}

_A Qt_ `Gradient` _(or compatible) object supplying the color stops._
```C++
QObject* QAccelPlot::GradientFill::gradient;
```




<hr>




### property gradientValueMax {#property-gradientvaluemax-12}

_Data-space value that maps to gradient position 1.0. Only used when gradientValueMaxSource is Fixed._ 
```C++
qreal QAccelPlot::GradientFill::gradientValueMax;
```




<hr>




### property gradientValueMaxSource {#property-gradientvaluemaxsource-12}

_How the gradient normalization maximum is determined; see_ `GradientValueSource` _._
```C++
GradientValueSource QAccelPlot::GradientFill::gradientValueMaxSource;
```




<hr>




### property gradientValueMin {#property-gradientvaluemin-12}

_Data-space value that maps to gradient position 0.0. Only used when gradientValueMinSource is Fixed._ 
```C++
qreal QAccelPlot::GradientFill::gradientValueMin;
```




<hr>




### property gradientValueMinSource {#property-gradientvalueminsource-12}

_How the gradient normalization minimum is determined; see_ `GradientValueSource` _._
```C++
GradientValueSource QAccelPlot::GradientFill::gradientValueMinSource;
```




<hr>




### property opacity {#property-opacity-12}

_Overall opacity of the fill area in [0, 1]. Default: 0.35._ 
```C++
qreal QAccelPlot::GradientFill::opacity;
```




<hr>
## Public Signals Documentation





### signal baselineChanged {#signal-baselinechanged}

_Emitted when the baseline property changes._ 
```C++
void QAccelPlot::GradientFill::baselineChanged;
```




<hr>




### signal baselineValueChanged {#signal-baselinevaluechanged}

_Emitted when the baselineValue property changes._ 
```C++
void QAccelPlot::GradientFill::baselineValueChanged;
```




<hr>




### signal directionChanged {#signal-directionchanged}

_Emitted when the direction property changes._ 
```C++
void QAccelPlot::GradientFill::directionChanged;
```




<hr>




### signal gradientChanged {#signal-gradientchanged}

_Emitted when the gradient property changes._ 
```C++
void QAccelPlot::GradientFill::gradientChanged;
```




<hr>




### signal gradientValueMaxChanged {#signal-gradientvaluemaxchanged}

_Emitted when the gradientValueMax property changes._ 
```C++
void QAccelPlot::GradientFill::gradientValueMaxChanged;
```




<hr>




### signal gradientValueMaxSourceChanged {#signal-gradientvaluemaxsourcechanged}

_Emitted when the gradientValueMaxSource property changes._ 
```C++
void QAccelPlot::GradientFill::gradientValueMaxSourceChanged;
```




<hr>




### signal gradientValueMinChanged {#signal-gradientvalueminchanged}

_Emitted when the gradientValueMin property changes._ 
```C++
void QAccelPlot::GradientFill::gradientValueMinChanged;
```




<hr>




### signal gradientValueMinSourceChanged {#signal-gradientvalueminsourcechanged}

_Emitted when the gradientValueMinSource property changes._ 
```C++
void QAccelPlot::GradientFill::gradientValueMinSourceChanged;
```




<hr>




### signal opacityChanged {#signal-opacitychanged}

_Emitted when the opacity property changes._ 
```C++
void QAccelPlot::GradientFill::opacityChanged;
```




<hr>
## Public Functions Documentation





### function GradientFill {#function-gradientfill}

_Constructs a_ [_**GradientFill**_](classQAccelPlot_1_1GradientFill.md) _with the given__parent_ _._
```C++
explicit QAccelPlot::GradientFill::GradientFill (
    QObject * parent=nullptr
) 
```




<hr>




### function baseline {#function-baseline-22}

_Returns the baseline mode._ 
```C++
GradientFillBaseline QAccelPlot::GradientFill::baseline () const
```




<hr>




### function baselineValue {#function-baselinevalue-22}

_Returns the fixed baseline data value._ 
```C++
qreal QAccelPlot::GradientFill::baselineValue () const
```




<hr>




### function direction {#function-direction-22}

_Returns the gradient direction._ 
```C++
GradientDirection QAccelPlot::GradientFill::direction () const
```




<hr>




### function gradient {#function-gradient-22}

_Returns the Qt Gradient object supplying color stops._ 
```C++
QObject * QAccelPlot::GradientFill::gradient () const
```




<hr>




### function gradientValueMax {#function-gradientvaluemax-22}

_Returns the maximum data value for gradient normalization._ 
```C++
qreal QAccelPlot::GradientFill::gradientValueMax () const
```




<hr>




### function gradientValueMaxSource {#function-gradientvaluemaxsource-22}

_Returns how the gradient normalization maximum is determined._ 
```C++
GradientValueSource QAccelPlot::GradientFill::gradientValueMaxSource () const
```




<hr>




### function gradientValueMin {#function-gradientvaluemin-22}

_Returns the minimum data value for gradient normalization._ 
```C++
qreal QAccelPlot::GradientFill::gradientValueMin () const
```




<hr>




### function gradientValueMinSource {#function-gradientvalueminsource-22}

_Returns how the gradient normalization minimum is determined._ 
```C++
GradientValueSource QAccelPlot::GradientFill::gradientValueMinSource () const
```




<hr>




### function opacity {#function-opacity-22}

_Returns the fill opacity._ 
```C++
qreal QAccelPlot::GradientFill::opacity () const
```




<hr>




### function payload {#function-payload}

_Returns a render-thread-safe snapshot of all fill parameters._ 
```C++
GradientFillPayload QAccelPlot::GradientFill::payload () const
```




<hr>




### function setBaseline {#function-setbaseline}

_Sets the baseline mode to_ _baseline_ _._
```C++
void QAccelPlot::GradientFill::setBaseline (
    GradientFillBaseline baseline
) 
```




<hr>




### function setBaselineValue {#function-setbaselinevalue}

_Sets the fixed baseline data value to_ _value_ _._
```C++
void QAccelPlot::GradientFill::setBaselineValue (
    qreal value
) 
```




<hr>




### function setDirection {#function-setdirection}

_Sets the gradient direction to_ _direction_ _._
```C++
void QAccelPlot::GradientFill::setDirection (
    GradientDirection direction
) 
```




<hr>




### function setGradient {#function-setgradient}

_Sets the Qt Gradient object to_ _gradient_ _._
```C++
void QAccelPlot::GradientFill::setGradient (
    QObject * gradient
) 
```




<hr>




### function setGradientValueMax {#function-setgradientvaluemax}

_Sets the maximum data value for gradient normalization to_ _value_ _._
```C++
void QAccelPlot::GradientFill::setGradientValueMax (
    qreal value
) 
```




<hr>




### function setGradientValueMaxSource {#function-setgradientvaluemaxsource}

_Sets the gradient normalization maximum source to_ _source_ _._
```C++
void QAccelPlot::GradientFill::setGradientValueMaxSource (
    GradientValueSource source
) 
```




<hr>




### function setGradientValueMin {#function-setgradientvaluemin}

_Sets the minimum data value for gradient normalization to_ _value_ _._
```C++
void QAccelPlot::GradientFill::setGradientValueMin (
    qreal value
) 
```




<hr>




### function setGradientValueMinSource {#function-setgradientvalueminsource}

_Sets the gradient normalization minimum source to_ _source_ _._
```C++
void QAccelPlot::GradientFill::setGradientValueMinSource (
    GradientValueSource source
) 
```




<hr>




### function setOpacity {#function-setopacity}

_Sets the fill opacity to_ _value_ _._
```C++
void QAccelPlot::GradientFill::setOpacity (
    qreal value
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/effects/GradientFill.hpp`

