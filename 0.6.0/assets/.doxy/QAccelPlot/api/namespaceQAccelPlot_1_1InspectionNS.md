








# Namespace QAccelPlot::InspectionNS



[**Namespace List**](namespaces.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**InspectionNS**](namespaceQAccelPlot_1_1InspectionNS.md)



_Namespace exposing the inspection_ `Status` _enum to QML as_`Inspection` _._




















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**Status**](#enum-status)  <br>_Outcome of an inspection query, or readiness of a series for queries._  |
















































## Public Types Documentation





### enum Status {#enum-status}

_Outcome of an inspection query, or readiness of a series for queries._ 
```C++
enum QAccelPlot::InspectionNS::Status {
    Ready,
    NoMatch,
    Idle,
    Preparing,
    Unsupported,
    Unavailable,
    InvalidArgument,
    Stale
};
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/inspection/InspectionResult.hpp`

