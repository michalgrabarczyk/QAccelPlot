








# Class QAccelPlot::TextTickLabelFormatter



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**TextTickLabelFormatter**](classQAccelPlot_1_1TextTickLabelFormatter.md)



_A tick label formatter that maps integer tick indices to a user-supplied list of strings._ [More...](#detailed-description)

* `#include <TextTickLabelFormatter.hpp>`



Inherits the following classes: [QAccelPlot::TickLabelFormatter](classQAccelPlot_1_1TickLabelFormatter.md)




## Inheritance diagram

```mermaid
flowchart TB
  classQAccelPlot_1_1TextTickLabelFormatter["QAccelPlot::TextTickLabelFormatter"]

  classQAccelPlot_1_1TickLabelFormatter["QAccelPlot::TickLabelFormatter"]
  classQAccelPlot_1_1TickLabelFormatter --> classQAccelPlot_1_1TextTickLabelFormatter
  click classQAccelPlot_1_1TickLabelFormatter "../classQAccelPlot_1_1TickLabelFormatter/" "Open QAccelPlot::TickLabelFormatter"

  external_base_classQAccelPlot_1_1TickLabelFormatter_1["QObject"]
  external_base_classQAccelPlot_1_1TickLabelFormatter_1 --> classQAccelPlot_1_1TickLabelFormatter

```




































## Public Properties

| Type | Name |
| ---: | :--- |
| property QStringList | [**labels**](classQAccelPlot_1_1TextTickLabelFormatter.md#property-labels-12)  <br>_The ordered list of label strings, indexed by the integer tick value._  |


## Public Properties inherited from QAccelPlot::TickLabelFormatter

See [QAccelPlot::TickLabelFormatter](classQAccelPlot_1_1TickLabelFormatter.md)

| Type | Name |
| ---: | :--- |
| property QML\_ANONYMOUSQJSValue | [**tickLabel**](classQAccelPlot_1_1TickLabelFormatter.md#property-ticklabel-12)  <br>_Optional JavaScript callback_ `function(value, tickStep)` _that overrides_`doFormat()` _._ |






## Public Signals

| Type | Name |
| ---: | :--- |
| signal void | [**labelsChanged**](classQAccelPlot_1_1TextTickLabelFormatter.md#signal-labelschanged)  <br>_Emitted when the labels property changes._  |


## Public Signals inherited from QAccelPlot::TickLabelFormatter

See [QAccelPlot::TickLabelFormatter](classQAccelPlot_1_1TickLabelFormatter.md)

| Type | Name |
| ---: | :--- |
| signal void | [**formatChanged**](classQAccelPlot_1_1TickLabelFormatter.md#signal-formatchanged)  <br>_Emitted whenever a property that affects formatted output changes._  |
| signal void | [**tickLabelChanged**](classQAccelPlot_1_1TickLabelFormatter.md#signal-ticklabelchanged)  <br>_Emitted when the tickLabel property changes._  |






## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**TextTickLabelFormatter**](#function-textticklabelformatter) (QObject \* parent=nullptr) <br>_Constructs a_ [_**TextTickLabelFormatter**_](classQAccelPlot_1_1TextTickLabelFormatter.md) _with the given__parent_ _._ |
|  QStringList | [**labels**](#function-labels-22) () const<br>_Returns the current label list._  |
| virtual qreal | [**minimumTickStep**](#function-minimumtickstep) () override const<br>_Returns_ `1` _, so a zoomed linear axis keeps one tick per label instead of ticks between labels._ |
|  void | [**setLabels**](#function-setlabels) (const QStringList & labels) <br>_Sets the label list to_ _labels_ _._ |


## Public Functions inherited from QAccelPlot::TickLabelFormatter

See [QAccelPlot::TickLabelFormatter](classQAccelPlot_1_1TickLabelFormatter.md)

| Type | Name |
| ---: | :--- |
|   | [**TickLabelFormatter**](classQAccelPlot_1_1TickLabelFormatter.md#function-ticklabelformatter) (QObject \* parent=nullptr) <br>_Constructs an_ [_**TickLabelFormatter**_](classQAccelPlot_1_1TickLabelFormatter.md) _with the given__parent_ _._ |
|  QString | [**format**](classQAccelPlot_1_1TickLabelFormatter.md#function-format) (qreal value, qreal tickStep) const<br>_Returns the display string for_ _value_ _at the given__tickStep_ _._ |
|  QString | [**formatLogTick**](classQAccelPlot_1_1TickLabelFormatter.md#function-formatlogtick) (qreal value) const<br>_Returns the display string for a major tick at_ _value_ _on a logarithmic axis._ |
| virtual qreal | [**minimumTickStep**](classQAccelPlot_1_1TickLabelFormatter.md#function-minimumtickstep) () const<br>_Returns the smallest major tick step a linear axis may use with this formatter._  |
|  void | [**setTickLabel**](classQAccelPlot_1_1TickLabelFormatter.md#function-setticklabel) (const QJSValue & tickLabel) <br>_Sets the JavaScript override callback to_ _tickLabel_ _._ |
|  QJSValue | [**tickLabel**](classQAccelPlot_1_1TickLabelFormatter.md#function-ticklabel-22) () const<br>_Returns the optional JavaScript override callback._  |














































## Protected Functions

| Type | Name |
| ---: | :--- |
| virtual QString | [**doFormat**](#function-doformat) (qreal value, qreal tickStep) override const<br>_Returns the label at the integer_ _value_ _, or an empty string if__value_ _is not an integer or is out of range._ |


## Protected Functions inherited from QAccelPlot::TickLabelFormatter

See [QAccelPlot::TickLabelFormatter](classQAccelPlot_1_1TickLabelFormatter.md)

| Type | Name |
| ---: | :--- |
| virtual QString | [**doFormat**](classQAccelPlot_1_1TickLabelFormatter.md#function-doformat) (qreal value, qreal tickStep) const = 0<br>_Subclass entry point — returns the formatted label for_ _value_ _._ |
| virtual QString | [**doFormatLogTick**](classQAccelPlot_1_1TickLabelFormatter.md#function-doformatlogtick) (qreal value) const<br>_Subclass entry point for log-scale major ticks — returns the label for_ _value_ _._ |






## Detailed Description


A tick on an integer value is labeled with the string at that index in `labels`. Other values and indices outside the list range are rendered as empty strings. On a linear axis, `minimumTickStep()` keeps ticks on whole numbers when the axis is zoomed in.




**See also:** [**NumericTickLabelFormatter**](classQAccelPlot_1_1NumericTickLabelFormatter.md), [**TickLabelFormatter**](classQAccelPlot_1_1TickLabelFormatter.md) 



    
## Public Properties Documentation





### property labels {#property-labels-12}

_The ordered list of label strings, indexed by the integer tick value._ 
```C++
QStringList QAccelPlot::TextTickLabelFormatter::labels;
```




<hr>
## Public Signals Documentation





### signal labelsChanged {#signal-labelschanged}

_Emitted when the labels property changes._ 
```C++
void QAccelPlot::TextTickLabelFormatter::labelsChanged;
```




<hr>
## Public Functions Documentation





### function TextTickLabelFormatter {#function-textticklabelformatter}

_Constructs a_ [_**TextTickLabelFormatter**_](classQAccelPlot_1_1TextTickLabelFormatter.md) _with the given__parent_ _._
```C++
explicit QAccelPlot::TextTickLabelFormatter::TextTickLabelFormatter (
    QObject * parent=nullptr
) 
```




<hr>




### function labels {#function-labels-22}

_Returns the current label list._ 
```C++
QStringList QAccelPlot::TextTickLabelFormatter::labels () const
```




<hr>




### function minimumTickStep {#function-minimumtickstep}

_Returns_ `1` _, so a zoomed linear axis keeps one tick per label instead of ticks between labels._
```C++
virtual qreal QAccelPlot::TextTickLabelFormatter::minimumTickStep () override const
```



Implements [*QAccelPlot::TickLabelFormatter::minimumTickStep*](classQAccelPlot_1_1TickLabelFormatter.md#function-minimumtickstep)


<hr>




### function setLabels {#function-setlabels}

_Sets the label list to_ _labels_ _._
```C++
void QAccelPlot::TextTickLabelFormatter::setLabels (
    const QStringList & labels
) 
```




<hr>
## Protected Functions Documentation





### function doFormat {#function-doformat}

_Returns the label at the integer_ _value_ _, or an empty string if__value_ _is not an integer or is out of range._
```C++
virtual QString QAccelPlot::TextTickLabelFormatter::doFormat (
    qreal value,
    qreal tickStep
) override const
```



Implements [*QAccelPlot::TickLabelFormatter::doFormat*](classQAccelPlot_1_1TickLabelFormatter.md#function-doformat)


<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/formatters/TextTickLabelFormatter.hpp`

