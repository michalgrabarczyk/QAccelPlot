








# Class QAccelPlot::GridNode



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**GridNode**](classQAccelPlot_1_1GridNode.md)



_Internal QSGNode responsible for rendering the plot grid into the scene graph._ [More...](#detailed-description)

* `#include <GridNode.hpp>`



Inherits the following classes: QSGNode




## Inheritance diagram

```mermaid
flowchart TB
  classQAccelPlot_1_1GridNode["QAccelPlot::GridNode"]

  external_base_classQAccelPlot_1_1GridNode_1["QSGNode"]
  external_base_classQAccelPlot_1_1GridNode_1 --> classQAccelPlot_1_1GridNode

```
































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**GridNode**](#function-gridnode) () <br>_Constructs a_ [_**GridNode**_](classQAccelPlot_1_1GridNode.md) _with empty sub-grid and main grid children._ |
|  void | [**update**](#function-update) (const [**Grid**](classQAccelPlot_1_1Grid.md) \* grid, const [**Axis**](classQAccelPlot_1_1Axis.md) \* xAxis, const [**Axis**](classQAccelPlot_1_1Axis.md) \* yAxis, const QRectF & plotRect) <br>_Recomputes and updates all grid geometry from the supplied configuration objects._  |




























## Detailed Description


Draws grid lines at the tick values from `AxisTickPainter::computeTickValues()`, the same values the axis places its tick marks at, so both stay aligned under all zoom levels, including log scale.


Keeps two child geometry nodes, sub-grid first and main grid second, for its whole lifetime and updates them in place. A hidden layer keeps its node with no vertices. Adding or removing nodes would make the Qt Quick renderer rebuild all batches and re-upload every series' vertex data. 


    
## Public Functions Documentation





### function GridNode {#function-gridnode}

_Constructs a_ [_**GridNode**_](classQAccelPlot_1_1GridNode.md) _with empty sub-grid and main grid children._
```C++
explicit QAccelPlot::GridNode::GridNode () 
```




<hr>




### function update {#function-update}

_Recomputes and updates all grid geometry from the supplied configuration objects._ 
```C++
void QAccelPlot::GridNode::update (
    const Grid * grid,
    const Axis * xAxis,
    const Axis * yAxis,
    const QRectF & plotRect
) 
```





**Parameters:**


* `grid` [**Grid**](classQAccelPlot_1_1Grid.md) configuration (line widths, visibility flags, colors). 
* `xAxis` Horizontal axis used to determine column positions. 
* `yAxis` Vertical axis used to determine row positions. 
* `plotRect` Plot area rectangle in item-local pixel coordinates. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/grid/GridNode.hpp`

