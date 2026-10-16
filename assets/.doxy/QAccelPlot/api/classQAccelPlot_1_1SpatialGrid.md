








# Class QAccelPlot::SpatialGrid



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**SpatialGrid**](classQAccelPlot_1_1SpatialGrid.md)



_Uniform-grid spatial index with bounded per-rectangle storage._ [More...](#detailed-description)

* `#include <SpatialGrid.hpp>`







































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**build**](#function-build) (const double \* data, int itemCount, int valuesPerItem=4) <br>_Rebuilds the spatial index from_ _data_ _containing__itemCount_ _axis-aligned rectangles._ |
|  void | [**buildF**](#function-buildf) (const float \* data, int itemCount, int valuesPerItem=4) <br>_Rebuilds the spatial index from single-precision_ _data_ _, like_`build()` _._ |
|  int | [**query**](#function-query) (double x, double y) const<br>_Returns the index of the topmost rectangle that contains point (_ _x_ _,__y_ _), or -1 if none._ |
|  int | [**queryTopmost**](#function-querytopmost) (double minX, double minY, double maxX, double maxY, const std::function&lt; bool(int)&gt; & accept) const<br>_Returns the highest index among rectangles overlapping the box for which_ _accept_ _returns true, or -1._ |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int | [**scanTopmost**](#function-scantopmost) (const double \* data, int itemCount, double minX, double minY, double maxX, double maxY, const std::function&lt; bool(int)&gt; & accept, int valuesPerItem=4) <br>_Returns what_ `queryTopmost()` _returns on a grid built from the same rectangles, without building one._ |
|  int | [**scanTopmostF**](#function-scantopmostf) (const float \* data, int itemCount, double minX, double minY, double maxX, double maxY, const std::function&lt; bool(int)&gt; & accept, int valuesPerItem=4) <br>_Scans single-precision_ _data_ _, like_`scanTopmost()` _._ |


























## Detailed Description


Each item is stored as four consecutive doubles (x1, y1, x2, y2) in a flat array with a configurable _valuesPerItem_ stride. Intended for use by [**RectangleSeries**](classQAccelPlot_1_1RectangleSeries.md) and similar shape types. Rectangles spanning more than 64 cells are stored once and checked separately during queries. An infinite edge leaves a rectangle unbounded in that direction; a rectangle with a NaN edge is never hit. 


    
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




### function buildF {#function-buildf}

_Rebuilds the spatial index from single-precision_ _data_ _, like_`build()` _._
```C++
void QAccelPlot::SpatialGrid::buildF (
    const float * data,
    int itemCount,
    int valuesPerItem=4
) 
```




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




### function queryTopmost {#function-querytopmost}

_Returns the highest index among rectangles overlapping the box for which_ _accept_ _returns true, or -1._
```C++
int QAccelPlot::SpatialGrid::queryTopmost (
    double minX,
    double minY,
    double maxX,
    double maxY,
    const std::function< bool(int)> & accept
) const
```



Lets callers apply a precise hit test, e.g. in pixel space, to the few candidates near a point. 


        

<hr>
## Public Static Functions Documentation





### function scanTopmost {#function-scantopmost}

_Returns what_ `queryTopmost()` _returns on a grid built from the same rectangles, without building one._
```C++
static int QAccelPlot::SpatialGrid::scanTopmost (
    const double * data,
    int itemCount,
    double minX,
    double minY,
    double maxX,
    double maxY,
    const std::function< bool(int)> & accept,
    int valuesPerItem=4
) 
```



Tests rectangles from the last to the first, so a call costs up to one pass over _data_. That is cheaper than a rebuild while the data is replaced after only a few queries. 


        

<hr>




### function scanTopmostF {#function-scantopmostf}

_Scans single-precision_ _data_ _, like_`scanTopmost()` _._
```C++
static int QAccelPlot::SpatialGrid::scanTopmostF (
    const float * data,
    int itemCount,
    double minX,
    double minY,
    double maxX,
    double maxY,
    const std::function< bool(int)> & accept,
    int valuesPerItem=4
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/SpatialGrid.hpp`

