








# Struct QAccelPlot::Internal::RecordFrame



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md) **>** [**RecordFrame**](structQAccelPlot_1_1Internal_1_1RecordFrame.md)



_What a series that draws one record per data point draws in a frame._ 

* `#include <RecordNode.hpp>`























## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**drawable**](#variable-drawable)   = `{false}`<br>`false` _removes the node: no records are drawn, or an axis, the plot rectangle, or the window is missing._ |
|  int | [**drawnCount**](#variable-drawncount)   = `{0}`<br>_Records drawn._  |
|  int | [**floatsPerRecord**](#variable-floatsperrecord)   = `{0}`<br>_Floats of one record in the data texture._  |
|  const char \* | [**typeName**](#variable-typename)   = `{""}`<br>_Series class the software-backend warning names._  |
|  int | [**verticesPerRecord**](#variable-verticesperrecord)   = `{0}`<br>_Vertices the shader draws one record with._  |
|  QQuickWindow \* | [**window**](#variable-window)   = `{nullptr}`<br>_Window the series renders in._  |












































## Public Attributes Documentation





### variable drawable {#variable-drawable}

`false` _removes the node: no records are drawn, or an axis, the plot rectangle, or the window is missing._
```C++
bool QAccelPlot::Internal::RecordFrame::drawable;
```




<hr>




### variable drawnCount {#variable-drawncount}

_Records drawn._ 
```C++
int QAccelPlot::Internal::RecordFrame::drawnCount;
```




<hr>




### variable floatsPerRecord {#variable-floatsperrecord}

_Floats of one record in the data texture._ 
```C++
int QAccelPlot::Internal::RecordFrame::floatsPerRecord;
```




<hr>




### variable typeName {#variable-typename}

_Series class the software-backend warning names._ 
```C++
const char* QAccelPlot::Internal::RecordFrame::typeName;
```




<hr>




### variable verticesPerRecord {#variable-verticesperrecord}

_Vertices the shader draws one record with._ 
```C++
int QAccelPlot::Internal::RecordFrame::verticesPerRecord;
```




<hr>




### variable window {#variable-window}

_Window the series renders in._ 
```C++
QQuickWindow* QAccelPlot::Internal::RecordFrame::window;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/internal/RecordNode.hpp`

