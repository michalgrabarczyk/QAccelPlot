








# Class QAccelPlot::DateTimeTickLabelFormatter



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**DateTimeTickLabelFormatter**](classQAccelPlot_1_1DateTimeTickLabelFormatter.md)



_A tick label formatter that displays tick values as formatted date/time strings._ [More...](#detailed-description)

* `#include <DateTimeTickLabelFormatter.hpp>`



Inherits the following classes: [QAccelPlot::TickLabelFormatter](classQAccelPlot_1_1TickLabelFormatter.md)




## Inheritance diagram

```mermaid
flowchart TB
  classQAccelPlot_1_1DateTimeTickLabelFormatter["QAccelPlot::DateTimeTickLabelFormatter"]

  classQAccelPlot_1_1TickLabelFormatter["QAccelPlot::TickLabelFormatter"]
  classQAccelPlot_1_1TickLabelFormatter --> classQAccelPlot_1_1DateTimeTickLabelFormatter
  click classQAccelPlot_1_1TickLabelFormatter "../classQAccelPlot_1_1TickLabelFormatter/" "Open QAccelPlot::TickLabelFormatter"

  external_base_classQAccelPlot_1_1TickLabelFormatter_1["QObject"]
  external_base_classQAccelPlot_1_1TickLabelFormatter_1 --> classQAccelPlot_1_1TickLabelFormatter

```




































## Public Properties

| Type | Name |
| ---: | :--- |
| property QString | [**dateTimeFormat**](classQAccelPlot_1_1DateTimeTickLabelFormatter.md#property-datetimeformat-12)  <br>_The QDateTime format string used to render each tick label. Default:_ `"yyyy-MM-dd HH:mm:ss"` _._ |


## Public Properties inherited from QAccelPlot::TickLabelFormatter

See [QAccelPlot::TickLabelFormatter](classQAccelPlot_1_1TickLabelFormatter.md)

| Type | Name |
| ---: | :--- |
| property QML\_ANONYMOUSQJSValue | [**tickLabel**](classQAccelPlot_1_1TickLabelFormatter.md#property-ticklabel-12)  <br>_Optional JavaScript callback_ `function(value, tickStep)` _that overrides_`doFormat()` _._ |






## Public Signals

| Type | Name |
| ---: | :--- |
| signal void | [**dateTimeFormatChanged**](classQAccelPlot_1_1DateTimeTickLabelFormatter.md#signal-datetimeformatchanged)  <br>_Emitted when the dateTimeFormat property changes._  |


## Public Signals inherited from QAccelPlot::TickLabelFormatter

See [QAccelPlot::TickLabelFormatter](classQAccelPlot_1_1TickLabelFormatter.md)

| Type | Name |
| ---: | :--- |
| signal void | [**formatChanged**](classQAccelPlot_1_1TickLabelFormatter.md#signal-formatchanged)  <br>_Emitted whenever a property that affects formatted output changes._  |
| signal void | [**tickLabelChanged**](classQAccelPlot_1_1TickLabelFormatter.md#signal-ticklabelchanged)  <br>_Emitted when the tickLabel property changes._  |






## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**DateTimeTickLabelFormatter**](#function-datetimeticklabelformatter) (QObject \* parent=nullptr) <br>_Constructs a_ [_**DateTimeTickLabelFormatter**_](classQAccelPlot_1_1DateTimeTickLabelFormatter.md) _with the given__parent_ _._ |
|  QString | [**dateTimeFormat**](#function-datetimeformat-22) () const<br>_Returns the current date/time format string._  |
|  void | [**setDateTimeFormat**](#function-setdatetimeformat) (const QString & format) <br>_Sets the date/time format string to_ _format_ _._ |


## Public Functions inherited from QAccelPlot::TickLabelFormatter

See [QAccelPlot::TickLabelFormatter](classQAccelPlot_1_1TickLabelFormatter.md)

| Type | Name |
| ---: | :--- |
|   | [**TickLabelFormatter**](classQAccelPlot_1_1TickLabelFormatter.md#function-ticklabelformatter) (QObject \* parent=nullptr) <br>_Constructs an_ [_**TickLabelFormatter**_](classQAccelPlot_1_1TickLabelFormatter.md) _with the given__parent_ _._ |
|  QString | [**format**](classQAccelPlot_1_1TickLabelFormatter.md#function-format) (qreal value, qreal tickStep) const<br>_Returns the display string for_ _value_ _at the given__tickStep_ _._ |
|  void | [**setTickLabel**](classQAccelPlot_1_1TickLabelFormatter.md#function-setticklabel) (const QJSValue & tickLabel) <br>_Sets the JavaScript override callback to_ _tickLabel_ _._ |
|  QJSValue | [**tickLabel**](classQAccelPlot_1_1TickLabelFormatter.md#function-ticklabel-22) () const<br>_Returns the optional JavaScript override callback._  |














































## Protected Functions

| Type | Name |
| ---: | :--- |
| virtual QString | [**doFormat**](#function-doformat) (qreal value, qreal tickStep) override const<br>_Returns a date/time string for_ _value_ _(milliseconds since epoch)._ |


## Protected Functions inherited from QAccelPlot::TickLabelFormatter

See [QAccelPlot::TickLabelFormatter](classQAccelPlot_1_1TickLabelFormatter.md)

| Type | Name |
| ---: | :--- |
| virtual QString | [**doFormat**](classQAccelPlot_1_1TickLabelFormatter.md#function-doformat) (qreal value, qreal tickStep) const = 0<br>_Subclass entry point — returns the formatted label for_ _value_ _._ |






## Detailed Description


Tick values are interpreted as milliseconds since the Unix epoch (1970-01-01T00:00:00 UTC).




**See also:** [**NumericTickLabelFormatter**](classQAccelPlot_1_1NumericTickLabelFormatter.md), [**TickLabelFormatter**](classQAccelPlot_1_1TickLabelFormatter.md) 



    
## Public Properties Documentation





### property dateTimeFormat {#property-datetimeformat-12}

_The QDateTime format string used to render each tick label. Default:_ `"yyyy-MM-dd HH:mm:ss"` _._
```C++
QString QAccelPlot::DateTimeTickLabelFormatter::dateTimeFormat;
```




<hr>
## Public Signals Documentation





### signal dateTimeFormatChanged {#signal-datetimeformatchanged}

_Emitted when the dateTimeFormat property changes._ 
```C++
void QAccelPlot::DateTimeTickLabelFormatter::dateTimeFormatChanged;
```




<hr>
## Public Functions Documentation





### function DateTimeTickLabelFormatter {#function-datetimeticklabelformatter}

_Constructs a_ [_**DateTimeTickLabelFormatter**_](classQAccelPlot_1_1DateTimeTickLabelFormatter.md) _with the given__parent_ _._
```C++
explicit QAccelPlot::DateTimeTickLabelFormatter::DateTimeTickLabelFormatter (
    QObject * parent=nullptr
) 
```




<hr>




### function dateTimeFormat {#function-datetimeformat-22}

_Returns the current date/time format string._ 
```C++
QString QAccelPlot::DateTimeTickLabelFormatter::dateTimeFormat () const
```




<hr>




### function setDateTimeFormat {#function-setdatetimeformat}

_Sets the date/time format string to_ _format_ _._
```C++
void QAccelPlot::DateTimeTickLabelFormatter::setDateTimeFormat (
    const QString & format
) 
```




<hr>
## Protected Functions Documentation





### function doFormat {#function-doformat}

_Returns a date/time string for_ _value_ _(milliseconds since epoch)._
```C++
virtual QString QAccelPlot::DateTimeTickLabelFormatter::doFormat (
    qreal value,
    qreal tickStep
) override const
```



Implements [*QAccelPlot::TickLabelFormatter::doFormat*](classQAccelPlot_1_1TickLabelFormatter.md#function-doformat)


<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/formatters/DateTimeTickLabelFormatter.hpp`

