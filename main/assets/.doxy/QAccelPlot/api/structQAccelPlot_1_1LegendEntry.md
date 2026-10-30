








# Struct QAccelPlot::LegendEntry



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**LegendEntry**](structQAccelPlot_1_1LegendEntry.md)



_How a legend draws one series: its symbol, color, line, and marker._ [More...](#detailed-description)

* `#include <LegendEntry.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**LineStyle**](classQAccelPlot_1_1LineStyle.md) \* | [**lineStyle**](#variable-linestyle-22)   = `{nullptr}`<br>_Line style, or null for solid._  |
|  [**MarkerShape**](namespaceQAccelPlot_1_1MarkerShapeNS.md#enum-shape) | [**markerShape**](#variable-markershape-22)   = `{MarkerShape::None}`<br>_Marker shape._  |
|  [**PlotSeries::LegendSymbol**](classQAccelPlot_1_1PlotSeries.md#enum-legendsymbol) | [**symbol**](#variable-symbol-22)   = `{PlotSeries::LegendSymbol::Line}`<br>_Symbol style._  |








## Public Properties

| Type | Name |
| ---: | :--- |
| property bool | [**antialiasing**](structQAccelPlot_1_1LegendEntry.md#property-antialiasing)  <br>_Whether the line and marker are antialiased._  |
| property qreal | [**antialiasingFeather**](structQAccelPlot_1_1LegendEntry.md#property-antialiasingfeather)  <br>_Antialiasing feather width in pixels._  |
| property QColor | [**color**](structQAccelPlot_1_1LegendEntry.md#property-color)  <br>_Color of the line, marker, or fill._  |
| property [**QAccelPlot::LineStyle**](classQAccelPlot_1_1LineStyle.md) \* | [**lineStyle**](structQAccelPlot_1_1LegendEntry.md#property-linestyle-12)  <br>_Line style, or null for a solid line. Owned by the series._  |
| property qreal | [**lineWidth**](structQAccelPlot_1_1LegendEntry.md#property-linewidth)  <br>_Line width in pixels._  |
| property bool | [**markerFilled**](structQAccelPlot_1_1LegendEntry.md#property-markerfilled)  <br>_Whether the marker is filled._  |
| property [**QAccelPlot::MarkerShape**](namespaceQAccelPlot_1_1MarkerShapeNS.md#enum-shape) | [**markerShape**](structQAccelPlot_1_1LegendEntry.md#property-markershape-12)  <br>_Marker shape;_ `None` _draws no marker._ |
| property qreal | [**markerSize**](structQAccelPlot_1_1LegendEntry.md#property-markersize)  <br>_Marker size in pixels._  |
| property qreal | [**markerStrokeWidth**](structQAccelPlot_1_1LegendEntry.md#property-markerstrokewidth)  <br>_Outline width in pixels of a hollow marker._  |
| property QML\_ANONYMOUSQAccelPlot::PlotSeries::LegendSymbol | [**symbol**](structQAccelPlot_1_1LegendEntry.md#property-symbol-12)  <br>_Symbol style._  |




































## Detailed Description


Returned by `PlotSeries::legendEntry()` and exposed by `LegendModel` as the `entry` role. The line fields apply to the `Line` symbol and the marker fields to `Line` and `Marker`.




**See also:** [**LegendModel**](classQAccelPlot_1_1LegendModel.md), [**PlotSeries::legendEntry()**](classQAccelPlot_1_1PlotSeries.md#function-legendentry) 



    
## Public Attributes Documentation





### variable lineStyle {#variable-linestyle-22}

_Line style, or null for solid._ 
```C++
LineStyle* QAccelPlot::LegendEntry::lineStyle;
```




<hr>




### variable markerShape {#variable-markershape-22}

_Marker shape._ 
```C++
MarkerShape QAccelPlot::LegendEntry::markerShape;
```




<hr>




### variable symbol {#variable-symbol-22}

_Symbol style._ 
```C++
PlotSeries::LegendSymbol QAccelPlot::LegendEntry::symbol;
```




<hr>
## Public Properties Documentation





### property antialiasing {#property-antialiasing}

_Whether the line and marker are antialiased._ 
```C++
bool QAccelPlot::LegendEntry::antialiasing;
```



Whether antialiasing is on. 


        

<hr>




### property antialiasingFeather {#property-antialiasingfeather}

_Antialiasing feather width in pixels._ 
```C++
qreal QAccelPlot::LegendEntry::antialiasingFeather;
```



Antialiasing feather width. 


        

<hr>




### property color {#property-color}

_Color of the line, marker, or fill._ 
```C++
QColor QAccelPlot::LegendEntry::color;
```



Line, marker, or fill color. 


        

<hr>




### property lineStyle {#property-linestyle-12}

_Line style, or null for a solid line. Owned by the series._ 
```C++
QAccelPlot::LineStyle* QAccelPlot::LegendEntry::lineStyle;
```




<hr>




### property lineWidth {#property-linewidth}

_Line width in pixels._ 
```C++
qreal QAccelPlot::LegendEntry::lineWidth;
```




<hr>




### property markerFilled {#property-markerfilled}

_Whether the marker is filled._ 
```C++
bool QAccelPlot::LegendEntry::markerFilled;
```




<hr>




### property markerShape {#property-markershape-12}

_Marker shape;_ `None` _draws no marker._
```C++
QAccelPlot::MarkerShape QAccelPlot::LegendEntry::markerShape;
```




<hr>




### property markerSize {#property-markersize}

_Marker size in pixels._ 
```C++
qreal QAccelPlot::LegendEntry::markerSize;
```




<hr>




### property markerStrokeWidth {#property-markerstrokewidth}

_Outline width in pixels of a hollow marker._ 
```C++
qreal QAccelPlot::LegendEntry::markerStrokeWidth;
```



Hollow marker outline width. 


        

<hr>




### property symbol {#property-symbol-12}

_Symbol style._ 
```C++
QML_ANONYMOUSQAccelPlot::PlotSeries::LegendSymbol QAccelPlot::LegendEntry::symbol;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/legend/LegendEntry.hpp`

