








# Class QAccelPlot::SpatialGrid



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**SpatialGrid**](classQAccelPlot_1_1SpatialGrid.md)



_Uniform-grid spatial index with bounded per-rectangle storage._ [More...](#detailed-description)

* `#include <SpatialGrid.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**build**](#function-build) (const double \* data, int itemCount, int valuesPerItem=4) <br>_Rebuilds the spatial index from_ _data_ _containing__itemCount_ _axis-aligned rectangles._ |
|  int | [**query**](#function-query) (double x, double y) const<br>_Returns the index of the topmost rectangle that contains point (_ _x_ _,__y_ _), or -1 if none._ |




























## Detailed Description


Each item is stored as four consecutive doubles (x1, y1, x2, y2) in a flat array with a configurable _valuesPerItem_ stride. Intended for use by [**RectangleList**](classQAccelPlot_1_1RectangleList.md) and similar shape types. Rectangles spanning more than 64 cells are stored once and checked separately during queries. 


    
## Public Functions Documentation





### function build {#function-build}

_Rebuilds the spatial index from_ _data_ _containing__itemCount_ _axis-aligned rectangles._
```C++
void QAccelPlot::SpatialGrid::build (
    const double * data,
    int itemCount,
    int valuesPerItem=4
) 
```





**Parameters:**


* `data` Pointer to the flat double array (x1, y1, x2, y2 per item, with _valuesPerItem_ stride). 
* `itemCount` Number of rectangles in _data_. 
* `valuesPerItem` Number of doubles per rectangle entry (default 4). 




        

<hr>




### function query {#function-query}

_Returns the index of the topmost rectangle that contains point (_ _x_ _,__y_ _), or -1 if none._
```C++
int QAccelPlot::SpatialGrid::query (
    double x,
    double y
) const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/SpatialGrid.hpp`

