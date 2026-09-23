








# Class QAccelPlot::PointSpatialIndex



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**PointSpatialIndex**](classQAccelPlot_1_1PointSpatialIndex.md)



_Uniform-grid spatial index for nearest-point queries over large point sets._ [More...](#detailed-description)

* `#include <PointSpatialIndex.hpp>`

















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**Mapping**](structQAccelPlot_1_1PointSpatialIndex_1_1Mapping.md) <br>_Coordinate mapping applied before indexing._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**build**](#function-build-12) (const float \* data, int pointCount, int stride, [**Mapping**](structQAccelPlot_1_1PointSpatialIndex_1_1Mapping.md) mapping) <br>_Rebuilds the index from_ _pointCount_ _points in__data_ _using__stride_ _floats per point._ |
|  void | [**build**](#function-build-22) (const float \* data, int pointCount, int stride=2) <br>_Rebuilds the index with the identity mapping, for linear axes._  |
|  void | [**clear**](#function-clear) () <br>_Removes all indexed points._  |
|  bool | [**isEmpty**](#function-isempty) () const<br>_Returns_ `true` _when no valid point is indexed._ |
|  [**Mapping**](structQAccelPlot_1_1PointSpatialIndex_1_1Mapping.md) | [**mapping**](#function-mapping) () const<br>_Returns the mapping used by the last build._  |
|  int | [**nearest**](#function-nearest) (double x, double y, double radiusX, double radiusY) const<br>_Returns the index of the point nearest to (_ _x_ _,__y_ _) inside the ellipse with radii__radiusX_ _and__radiusY_ _, or -1 when none. All arguments are in mapped space._ |
|  int | [**validPointCount**](#function-validpointcount) () const<br>_Returns the number of indexed (valid) points._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  bool | [**mapCoordinate**](#function-mapcoordinate) (double value, bool logarithmic, double & mapped) <br>_Maps_ _value_ _for indexing. Returns_`false` _when the value cannot be indexed._ |


























## Detailed Description


Points are read from a flat float array with a configurable stride whose first two components are `x` and `y`. Coordinates are indexed in _mapped_ space: log10 of the value on logarithmic axes, the value itself otherwise, so a pixel radius converts to a constant mapped-space radius at any zoom level. Non-finite points, and non-positive points on logarithmic axes, are excluded.


Cells are stored in a compressed layout (`cellStart_` offsets into one index array), which keeps a rebuild over a million points to a handful of allocations.




**See also:** [**PointCloud**](classQAccelPlot_1_1PointCloud.md) 



    
## Public Functions Documentation





### function build {#function-build-12}

_Rebuilds the index from_ _pointCount_ _points in__data_ _using__stride_ _floats per point._
```C++
void QAccelPlot::PointSpatialIndex::build (
    const float * data,
    int pointCount,
    int stride,
    Mapping mapping
) 
```




<hr>




### function build {#function-build-22}

_Rebuilds the index with the identity mapping, for linear axes._ 
```C++
void QAccelPlot::PointSpatialIndex::build (
    const float * data,
    int pointCount,
    int stride=2
) 
```



A separate overload rather than a defaulted `Mapping` argument: a nested class's default member initializers are not usable in a default argument until the enclosing class is complete, which MSVC accepts but Clang and GCC reject. 


        

<hr>




### function clear {#function-clear}

_Removes all indexed points._ 
```C++
void QAccelPlot::PointSpatialIndex::clear () 
```




<hr>




### function isEmpty {#function-isempty}

_Returns_ `true` _when no valid point is indexed._
```C++
bool QAccelPlot::PointSpatialIndex::isEmpty () const
```




<hr>




### function mapping {#function-mapping}

_Returns the mapping used by the last build._ 
```C++
Mapping QAccelPlot::PointSpatialIndex::mapping () const
```




<hr>




### function nearest {#function-nearest}

_Returns the index of the point nearest to (_ _x_ _,__y_ _) inside the ellipse with radii__radiusX_ _and__radiusY_ _, or -1 when none. All arguments are in mapped space._
```C++
int QAccelPlot::PointSpatialIndex::nearest (
    double x,
    double y,
    double radiusX,
    double radiusY
) const
```



Distance is normalized by the radii, so an anisotropic pixel radius behaves like a circle on screen. Ties resolve to the highest index, the point drawn last. 


        

<hr>




### function validPointCount {#function-validpointcount}

_Returns the number of indexed (valid) points._ 
```C++
int QAccelPlot::PointSpatialIndex::validPointCount () const
```




<hr>
## Public Static Functions Documentation





### function mapCoordinate {#function-mapcoordinate}

_Maps_ _value_ _for indexing. Returns_`false` _when the value cannot be indexed._
```C++
static bool QAccelPlot::PointSpatialIndex::mapCoordinate (
    double value,
    bool logarithmic,
    double & mapped
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/PointSpatialIndex.hpp`

