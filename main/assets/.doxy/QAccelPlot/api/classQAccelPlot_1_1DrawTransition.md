








# Class QAccelPlot::DrawTransition



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**DrawTransition**](classQAccelPlot_1_1DrawTransition.md)



_A transition that reveals items one after another, in index order._ [More...](#detailed-description)

* `#include <DrawTransition.hpp>`



Inherits the following classes: [QAccelPlot::EnterExitTransition](classQAccelPlot_1_1EnterExitTransition.md)




## Inheritance diagram

```mermaid
flowchart TB
  classQAccelPlot_1_1DrawTransition["QAccelPlot::DrawTransition"]

  classQAccelPlot_1_1EnterExitTransition["QAccelPlot::EnterExitTransition"]
  classQAccelPlot_1_1EnterExitTransition --> classQAccelPlot_1_1DrawTransition
  click classQAccelPlot_1_1EnterExitTransition "../classQAccelPlot_1_1EnterExitTransition/" "Open QAccelPlot::EnterExitTransition"

  classQAccelPlot_1_1DataTransition["QAccelPlot::DataTransition"]
  classQAccelPlot_1_1DataTransition --> classQAccelPlot_1_1EnterExitTransition
  click classQAccelPlot_1_1DataTransition "../classQAccelPlot_1_1DataTransition/" "Open QAccelPlot::DataTransition"

  external_base_classQAccelPlot_1_1DataTransition_1["QObject"]
  external_base_classQAccelPlot_1_1DataTransition_1 --> classQAccelPlot_1_1DataTransition
  external_base_classQAccelPlot_1_1DataTransition_2["QQmlParserStatus"]
  external_base_classQAccelPlot_1_1DataTransition_2 --> classQAccelPlot_1_1DataTransition

```




















































## Public Properties inherited from QAccelPlot::DataTransition

See [QAccelPlot::DataTransition](classQAccelPlot_1_1DataTransition.md)

| Type | Name |
| ---: | :--- |
| property QQmlListProperty&lt; QObject &gt; | [**animations**](classQAccelPlot_1_1DataTransition.md#property-animations-12)  <br>_Qt Quick animations of the opacity of each item, such as_ `NumberAnimation` _{ property: "opacity" }._ |
| property QML\_ANONYMOUSint | [**duration**](classQAccelPlot_1_1DataTransition.md#property-duration-12)  <br>_Duration of the transition in milliseconds. Default: 300._  |
| property QEasingCurve | [**easing**](classQAccelPlot_1_1DataTransition.md#property-easing-12)  <br>_Easing curve applied to the progress of each item. Default:_ `QEasingCurve::Linear` _._ |
| property bool | [**enabled**](classQAccelPlot_1_1DataTransition.md#property-enabled-12)  <br>_Whether the transition is active. When_ `false` _, the items it animates change instantly._ |
| property bool | [**running**](classQAccelPlot_1_1DataTransition.md#property-running-12)  <br>_Read-only:_ `true` _while the transition is playing on at least one series._ |
| property qreal | [**stagger**](classQAccelPlot_1_1DataTransition.md#property-stagger-12)  <br>_Share of_ `duration` _over which the start times of the items are spread, in [0, 1]._ |












## Public Signals inherited from QAccelPlot::DataTransition

See [QAccelPlot::DataTransition](classQAccelPlot_1_1DataTransition.md)

| Type | Name |
| ---: | :--- |
| signal void | [**durationChanged**](classQAccelPlot_1_1DataTransition.md#signal-durationchanged)  <br>_Emitted when the duration property changes._  |
| signal void | [**easingChanged**](classQAccelPlot_1_1DataTransition.md#signal-easingchanged)  <br>_Emitted when the easing property changes._  |
| signal void | [**enabledChanged**](classQAccelPlot_1_1DataTransition.md#signal-enabledchanged)  <br>_Emitted when the enabled property changes._  |
| signal void | [**runningChanged**](classQAccelPlot_1_1DataTransition.md#signal-runningchanged)  <br>_Emitted when the running property changes._  |
| signal void | [**staggerChanged**](classQAccelPlot_1_1DataTransition.md#signal-staggerchanged)  <br>_Emitted when the stagger property changes._  |








## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**DrawTransition**](#function-drawtransition) (QObject \* parent=nullptr) <br>_Constructs a_ [_**DrawTransition**_](classQAccelPlot_1_1DrawTransition.md) _with the given__parent_ _._ |




## Public Functions inherited from QAccelPlot::DataTransition

See [QAccelPlot::DataTransition](classQAccelPlot_1_1DataTransition.md)

| Type | Name |
| ---: | :--- |
|  QQmlListProperty&lt; QObject &gt; | [**animations**](classQAccelPlot_1_1DataTransition.md#function-animations-22) () <br>_Returns the QML list property of the opacity animations._  |
|  void | [**cancel**](classQAccelPlot_1_1DataTransition.md#function-cancel) () <br>_Ends every active run immediately._  |
|  void | [**classBegin**](classQAccelPlot_1_1DataTransition.md#function-classbegin) () override<br>_Holds back the warnings about ignored animations while the component loads._  |
|  void | [**componentComplete**](classQAccelPlot_1_1DataTransition.md#function-componentcomplete) () override<br>_Reports ignored animations once the component is complete._  |
|  int | [**duration**](classQAccelPlot_1_1DataTransition.md#function-duration-22) () const<br>_Returns the animation duration in milliseconds._  |
|  QEasingCurve | [**easing**](classQAccelPlot_1_1DataTransition.md#function-easing-22) () const<br>_Returns the easing curve._  |
|  bool | [**enabled**](classQAccelPlot_1_1DataTransition.md#function-enabled-22) () const<br>_Returns_ `true` _if the transition is enabled._ |
|  double | [**progress**](classQAccelPlot_1_1DataTransition.md#function-progress) (const [**Run**](classQAccelPlot_1_1DataTransition_1_1Run.md) & run) const<br>_Returns the progress of_ _run_ _in [0, 1], before easing._ |
|  bool | [**running**](classQAccelPlot_1_1DataTransition.md#function-running-22) () const<br>_Returns_ `true` _while at least one run is active._ |
|  void | [**setDuration**](classQAccelPlot_1_1DataTransition.md#function-setduration) (int duration) <br>_Sets the animation duration to_ _duration_ _milliseconds._ |
|  void | [**setEasing**](classQAccelPlot_1_1DataTransition.md#function-seteasing) (const QEasingCurve & easing) <br>_Sets the easing curve to_ _easing_ _._ |
|  void | [**setEnabled**](classQAccelPlot_1_1DataTransition.md#function-setenabled) (bool enabled) <br>_Sets the enabled state to_ _enabled_ _._ |
|  void | [**setStagger**](classQAccelPlot_1_1DataTransition.md#function-setstagger) (qreal stagger) <br>_Sets the stagger to_ _stagger_ _, clamped to [0, 1]._ |
|  qreal | [**stagger**](classQAccelPlot_1_1DataTransition.md#function-stagger-22) () const<br>_Returns the share of the duration over which item start times are spread._  |
|  void | [**start**](classQAccelPlot_1_1DataTransition.md#function-start) ([**Run**](classQAccelPlot_1_1DataTransition_1_1Run.md) & run) <br>_Starts_ _run_ _, restarting it if it is already active._ |
|   | [**~DataTransition**](classQAccelPlot_1_1DataTransition.md#function-datatransition) () override<br>_Destroys the transition, ending its runs._  |
















































































## Detailed Description


As an `enterTransition` it draws the added items in; as an `exitTransition` it hides the removed items from the last to the first. A line draws its last revealed segment partway.


`stagger` defaults to 1: each item appears at once, and `easing` shapes how the reveal moves along the items. An opacity animation fades each item in from the moment the reveal reaches it:



```C++
QAccelPlot.DrawTransition {
    NumberAnimation { property: "opacity"; from: 0; duration: 250 }
}
```





**See also:** [**GrowTransition**](classQAccelPlot_1_1GrowTransition.md), [**MorphTransition**](classQAccelPlot_1_1MorphTransition.md), [**EnterExitTransition**](classQAccelPlot_1_1EnterExitTransition.md) 



    
## Public Functions Documentation





### function DrawTransition {#function-drawtransition}

_Constructs a_ [_**DrawTransition**_](classQAccelPlot_1_1DrawTransition.md) _with the given__parent_ _._
```C++
explicit QAccelPlot::DrawTransition::DrawTransition (
    QObject * parent=nullptr
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/transitions/DrawTransition.hpp`

