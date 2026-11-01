








# Namespace QAccelPlot::StepModeNS



[**Namespace List**](namespaces.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**StepModeNS**](namespaceQAccelPlot_1_1StepModeNS.md)



_Namespace exposing the_ `StepMode` _enum to QML._




















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**Mode**](#enum-mode)  <br>_How a line joins consecutive data points: straight, or as a horizontal step and a vertical riser._  |
















































## Public Types Documentation





### enum Mode {#enum-mode}

_How a line joins consecutive data points: straight, or as a horizontal step and a vertical riser._ 
```C++
enum QAccelPlot::StepModeNS::Mode {
    None,
    Before,
    After,
    Center
};
```



A step mode decides where each data point's value holds. The line starts at the first data point and ends at the last one; a step never reaches past them. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/StepMode.hpp`

