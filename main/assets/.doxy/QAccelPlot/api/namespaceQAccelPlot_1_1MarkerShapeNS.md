








# Namespace QAccelPlot::MarkerShapeNS



[**Namespace List**](namespaces.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**MarkerShapeNS**](namespaceQAccelPlot_1_1MarkerShapeNS.md)



_Namespace exposing the_ `MarkerShape` _enum to QML._




















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**Shape**](#enum-shape)  <br>_Marker shapes drawn by series with a_ `marker` _grouped property, such as_`LineCurve` _and_`PointCloud` _._ |
















































## Public Types Documentation





### enum Shape {#enum-shape}

_Marker shapes drawn by series with a_ `marker` _grouped property, such as_`LineCurve` _and_`PointCloud` _._
```C++
enum QAccelPlot::MarkerShapeNS::Shape {
    None,
    Circle,
    Square,
    Diamond,
    TriangleUp,
    TriangleDown,
    TriangleLeft,
    TriangleRight,
    Cross,
    XCross,
    HLine,
    VLine,
    Star,
    Asterisk,
    Pixel,
    Hexagon,
    Pentagon
};
```



Every shape except `Pixel` fits within a square whose half-width is the marker size. The shaders select a shape by this value minus one, so append new shapes at the end and never reorder or insert. Series that always draw markers, such as `PointCloud`, do not accept `None`. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/MarkerShape.hpp`

