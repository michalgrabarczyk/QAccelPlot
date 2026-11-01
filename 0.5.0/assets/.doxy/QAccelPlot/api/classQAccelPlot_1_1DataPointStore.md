








# Class QAccelPlot::DataPointStore



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**DataPointStore**](classQAccelPlot_1_1DataPointStore.md)



_Holds the data points of one series and derives everything that depends only on them._ [More...](#detailed-description)

* `#include <DataPointStore.hpp>`

















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**AttributeView**](structQAccelPlot_1_1DataPointStore_1_1AttributeView.md) <br>_Read-only view over the float attributes of the stored data points._  |
| struct | [**DimensionState**](structQAccelPlot_1_1DataPointStore_1_1DimensionState.md) <br>_Scale and viewport of one dimension, as its axis currently shows it._  |
| struct | [**Extent**](structQAccelPlot_1_1DataPointStore_1_1Extent.md) <br>_Smallest and largest valid coordinate in one dimension._  |
| struct | [**Extents**](structQAccelPlot_1_1DataPointStore_1_1Extents.md) <br>[_**Extents**_](structQAccelPlot_1_1DataPointStore_1_1Extents.md) _in both dimensions; a dimension without a valid coordinate has none._ |
| struct | [**Layout**](structQAccelPlot_1_1DataPointStore_1_1Layout.md) <br>_Shape of a data point._  |
| struct | [**View**](structQAccelPlot_1_1DataPointStore_1_1View.md) <br>_Read-only view over interleaved data points in float or double precision._  |
| struct | [**Viewport**](structQAccelPlot_1_1DataPointStore_1_1Viewport.md) <br>_Visible range of one dimension._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**Dimension**](#enum-dimension)  <br>_Plot dimension a coordinate column belongs to._  |
| enum  | [**RangeRule**](#enum-rangerule)  <br>_Which coordinates count toward the extents._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**DataPointStore**](#function-datapointstore) ([**Layout**](structQAccelPlot_1_1DataPointStore_1_1Layout.md) layout) <br>_Creates an empty store whose data points have_ _layout_ _._ |
|  void | [**append**](#function-append) (const double \* coordinates) <br>_Appends one double data point of_ `columnCount()` _coordinates; promotes float data first._ |
|  void | [**assignFloat**](#function-assignfloat) (const float \* coordinates, int count) <br>_Replaces the data points with_ _count_ _float data points copied from__coordinates_ _, reusing the allocation._ |
|  void | [**assignPrecise**](#function-assignprecise) (const double \* coordinates, int count) <br>_Replaces the data points with_ _count_ _double data points copied from__coordinates_ _._ |
|  float | [**attribute**](#function-attribute) (int index, int which=0) noexcept const<br>_Returns attribute_ _which_ _of data point__index_ _; NaN without attributes._ |
|  [**AttributeView**](structQAccelPlot_1_1DataPointStore_1_1AttributeView.md) | [**attributes**](#function-attributes) () noexcept const<br>_Returns the stored float attributes._  |
|  const std::vector&lt; int &gt; & | [**categories**](#function-categories) () noexcept const<br>_Returns the stored categories: empty, or one per data point._  |
|  void | [**clear**](#function-clear) () <br>_Removes all data points, attributes, and categories._  |
|  void | [**clearDrawn**](#function-cleardrawn) () <br>_Draws the stored data points again._  |
|  int | [**columnCount**](#function-columncount) () noexcept const<br>_Returns the number of coordinate columns per data point._  |
|  int | [**count**](#function-count) () noexcept const<br>_Returns the number of data points._  |
|  const [**DimensionState**](structQAccelPlot_1_1DataPointStore_1_1DimensionState.md) & | [**dimensionState**](#function-dimensionstate) ([**Dimension**](classQAccelPlot_1_1DataPointStore.md#enum-dimension) dimension) noexcept const<br>_Returns the scale and viewport last given for_ _dimension_ _._ |
|  [**View**](structQAccelPlot_1_1DataPointStore_1_1View.md) | [**drawnView**](#function-drawnview) () const<br>_Returns the data points that are drawn: the drawn ones, or the stored ones._  |
|  [**Extents**](structQAccelPlot_1_1DataPointStore_1_1Extents.md) | [**extents**](#function-extents) (bool logScaleX, bool logScaleY, int first=0, int last=-1) const<br>_Returns the extents of data points [_ _first_ _,__last_ _) under the layout's range rule._ |
|  bool | [**hasAttributes**](#function-hasattributes) () noexcept const<br>_Returns whether the data points carry float attributes._  |
|  bool | [**hasCategories**](#function-hascategories) () noexcept const<br>_Returns whether the data points carry categories._  |
|  bool | [**hasDrawn**](#function-hasdrawn) () noexcept const<br>_Returns whether drawn data points replace the stored ones._  |
|  bool | [**isPrecise**](#function-isprecise) () noexcept const<br>_Returns whether the data points are stored as doubles._  |
|  const [**Layout**](structQAccelPlot_1_1DataPointStore_1_1Layout.md) & | [**layout**](#function-layout) () noexcept const<br>_Returns the data point layout._  |
|  void | [**promoteToPrecise**](#function-promotetoprecise) () <br>_Converts float data points to double, so later updates keep their precision. Does nothing for double data._  |
|  void | [**remapColumns**](#function-remapcolumns) (std::vector&lt; [**Dimension**](classQAccelPlot_1_1DataPointStore.md#enum-dimension) &gt; columns) <br>_Ties the coordinate columns to_ _columns_ _, one dimension per column, keeping the data points._ |
|  int | [**renderCount**](#function-rendercount) () const<br>_Returns the number of data points in the render data._  |
|  const std::vector&lt; float &gt; & | [**renderData**](#function-renderdata) () const<br>_Returns the float render data, rebuilding it first when stale._  |
|  double | [**renderOrigin**](#function-renderorigin) ([**Dimension**](classQAccelPlot_1_1DataPointStore.md#enum-dimension) dimension) const<br>_Returns the render origin of_ _dimension_ _, rebuilding the render data first when stale._ |
|  std::uint64\_t | [**renderRevision**](#function-renderrevision) () noexcept const<br>_Returns a number that changes whenever the render data changes._  |
|  int | [**renderStride**](#function-renderstride) () noexcept const<br>_Returns the floats per data point in the render data._  |
|  [**View**](structQAccelPlot_1_1DataPointStore_1_1View.md) | [**renderedView**](#function-renderedview) () const<br>_Returns the data points the render data is built from:_ `drawnView()` _, without the invalid data points when_`skipInvalid()` _is set._ |
|  void | [**setAttributes**](#function-setattributes) (std::vector&lt; float &gt; && attributes) <br>_Replaces the float attributes: empty, or_ `attributeCount` _floats per data point._ |
|  void | [**setCategories**](#function-setcategories) (std::vector&lt; int &gt; && categories) <br>_Replaces the categories: empty, or one per data point._  |
|  void | [**setDimensionState**](#function-setdimensionstate) ([**Dimension**](classQAccelPlot_1_1DataPointStore.md#enum-dimension) dimension, const [**DimensionState**](structQAccelPlot_1_1DataPointStore_1_1DimensionState.md) & state) <br>_Updates the scale and viewport of_ _dimension_ _._ |
|  std::vector&lt; double &gt; | [**setDrawn**](#function-setdrawn) (std::vector&lt; double &gt; && dataPoints, int count) <br>_Draws_ _count_ _data points moved from__dataPoints_ _instead of the stored ones._ |
|  void | [**setFloat**](#function-setfloat) (std::vector&lt; float &gt; && coordinates, int count, std::vector&lt; float &gt; && attributes={}) <br>_Replaces the data points with_ _count_ _float data points moved from__coordinates_ _._ |
|  void | [**setLayout**](#function-setlayout) ([**Layout**](structQAccelPlot_1_1DataPointStore_1_1Layout.md) layout) <br>_Clears the store and switches to_ _layout_ _._ |
|  void | [**setPrecise**](#function-setprecise) (std::vector&lt; double &gt; && coordinates, int count, std::vector&lt; float &gt; && attributes={}) <br>_Replaces the data points with_ _count_ _double data points moved from__coordinates_ _._ |
|  void | [**setSkipInvalid**](#function-setskipinvalid) (bool skip) <br>_Leaves invalid data points out of the render data and_ `renderedView()` _._ |
|  bool | [**skipInvalid**](#function-skipinvalid) () noexcept const<br>_Returns whether invalid data points are left out of the render data._  |
|  double | [**value**](#function-value) (int index, int column) noexcept const<br>_Returns coordinate_ _column_ _of stored data point__index_ _._ |
|  [**View**](structQAccelPlot_1_1DataPointStore_1_1View.md) | [**view**](#function-view) () noexcept const<br>_Returns the stored data points._  |




























## Detailed Description


A data point is a fixed number of coordinate columns, each tied to the X or Y dimension, plus optional float attributes and an optional integer category. The store keeps the coordinates at the precision they were given in and derives:



* the extents of the valid coordinates under one of three range rules;
* the float render data uploaded to the GPU. Float input is its own render data. Double input is uploaded relative to a render origin per dimension, which moves to the viewport center when the viewport gets too far from it. Logarithmic dimensions are never shifted.




Render data is rebuilt lazily, the first time it is read after something it depends on changed, so even its const readers may rebuild it. The store is not thread-safe: use it from the series' thread, or from `updatePaintNode()` while that thread is blocked. 


    
## Public Types Documentation





### enum Dimension {#enum-dimension}

_Plot dimension a coordinate column belongs to._ 
```C++
enum QAccelPlot::DataPointStore::Dimension {
    X,
    Y
};
```




<hr>




### enum RangeRule {#enum-rangerule}

_Which coordinates count toward the extents._ 
```C++
enum QAccelPlot::DataPointStore::RangeRule {
    EachCoordinate,
    WholeDataPoint,
    SkipOnNaN
};
```



A coordinate is valid when it is finite and, on a logarithmic dimension, positive. 


        

<hr>
## Public Functions Documentation





### function DataPointStore {#function-datapointstore}

_Creates an empty store whose data points have_ _layout_ _._
```C++
explicit QAccelPlot::DataPointStore::DataPointStore (
    Layout layout
) 
```




<hr>




### function append {#function-append}

_Appends one double data point of_ `columnCount()` _coordinates; promotes float data first._
```C++
void QAccelPlot::DataPointStore::append (
    const double * coordinates
) 
```



The render data grows in place when nothing else it depends on has changed. 


        

<hr>




### function assignFloat {#function-assignfloat}

_Replaces the data points with_ _count_ _float data points copied from__coordinates_ _, reusing the allocation._
```C++
void QAccelPlot::DataPointStore::assignFloat (
    const float * coordinates,
    int count
) 
```




<hr>




### function assignPrecise {#function-assignprecise}

_Replaces the data points with_ _count_ _double data points copied from__coordinates_ _._
```C++
void QAccelPlot::DataPointStore::assignPrecise (
    const double * coordinates,
    int count
) 
```




<hr>




### function attribute {#function-attribute}

_Returns attribute_ _which_ _of data point__index_ _; NaN without attributes._
```C++
float QAccelPlot::DataPointStore::attribute (
    int index,
    int which=0
) noexcept const
```




<hr>




### function attributes {#function-attributes}

_Returns the stored float attributes._ 
```C++
AttributeView QAccelPlot::DataPointStore::attributes () noexcept const
```




<hr>




### function categories {#function-categories}

_Returns the stored categories: empty, or one per data point._ 
```C++
const std::vector< int > & QAccelPlot::DataPointStore::categories () noexcept const
```




<hr>




### function clear {#function-clear}

_Removes all data points, attributes, and categories._ 
```C++
void QAccelPlot::DataPointStore::clear () 
```




<hr>




### function clearDrawn {#function-cleardrawn}

_Draws the stored data points again._ 
```C++
void QAccelPlot::DataPointStore::clearDrawn () 
```




<hr>




### function columnCount {#function-columncount}

_Returns the number of coordinate columns per data point._ 
```C++
int QAccelPlot::DataPointStore::columnCount () noexcept const
```




<hr>




### function count {#function-count}

_Returns the number of data points._ 
```C++
int QAccelPlot::DataPointStore::count () noexcept const
```




<hr>




### function dimensionState {#function-dimensionstate}

_Returns the scale and viewport last given for_ _dimension_ _._
```C++
const DimensionState & QAccelPlot::DataPointStore::dimensionState (
    Dimension dimension
) noexcept const
```




<hr>




### function drawnView {#function-drawnview}

_Returns the data points that are drawn: the drawn ones, or the stored ones._ 
```C++
View QAccelPlot::DataPointStore::drawnView () const
```




<hr>




### function extents {#function-extents}

_Returns the extents of data points [_ _first_ _,__last_ _) under the layout's range rule._
```C++
Extents QAccelPlot::DataPointStore::extents (
    bool logScaleX,
    bool logScaleY,
    int first=0,
    int last=-1
) const
```



_last_ of -1 means the end. _logScaleX_ and _logScaleY_ decide which coordinates are valid. 


        

<hr>




### function hasAttributes {#function-hasattributes}

_Returns whether the data points carry float attributes._ 
```C++
bool QAccelPlot::DataPointStore::hasAttributes () noexcept const
```




<hr>




### function hasCategories {#function-hascategories}

_Returns whether the data points carry categories._ 
```C++
bool QAccelPlot::DataPointStore::hasCategories () noexcept const
```




<hr>




### function hasDrawn {#function-hasdrawn}

_Returns whether drawn data points replace the stored ones._ 
```C++
bool QAccelPlot::DataPointStore::hasDrawn () noexcept const
```




<hr>




### function isPrecise {#function-isprecise}

_Returns whether the data points are stored as doubles._ 
```C++
bool QAccelPlot::DataPointStore::isPrecise () noexcept const
```




<hr>




### function layout {#function-layout}

_Returns the data point layout._ 
```C++
const Layout & QAccelPlot::DataPointStore::layout () noexcept const
```




<hr>




### function promoteToPrecise {#function-promotetoprecise}

_Converts float data points to double, so later updates keep their precision. Does nothing for double data._ 
```C++
void QAccelPlot::DataPointStore::promoteToPrecise () 
```




<hr>




### function remapColumns {#function-remapcolumns}

_Ties the coordinate columns to_ _columns_ _, one dimension per column, keeping the data points._
```C++
void QAccelPlot::DataPointStore::remapColumns (
    std::vector< Dimension > columns
) 
```




<hr>




### function renderCount {#function-rendercount}

_Returns the number of data points in the render data._ 
```C++
int QAccelPlot::DataPointStore::renderCount () const
```




<hr>




### function renderData {#function-renderdata}

_Returns the float render data, rebuilding it first when stale._ 
```C++
const std::vector< float > & QAccelPlot::DataPointStore::renderData () const
```



Each data point is its coordinates relative to `renderOrigin()`, then its attributes. 


        

<hr>




### function renderOrigin {#function-renderorigin}

_Returns the render origin of_ _dimension_ _, rebuilding the render data first when stale._
```C++
double QAccelPlot::DataPointStore::renderOrigin (
    Dimension dimension
) const
```



It is 0 for float data and logarithmic dimensions. 


        

<hr>




### function renderRevision {#function-renderrevision}

_Returns a number that changes whenever the render data changes._ 
```C++
std::uint64_t QAccelPlot::DataPointStore::renderRevision () noexcept const
```




<hr>




### function renderStride {#function-renderstride}

_Returns the floats per data point in the render data._ 
```C++
int QAccelPlot::DataPointStore::renderStride () noexcept const
```




<hr>




### function renderedView {#function-renderedview}

_Returns the data points the render data is built from:_ `drawnView()` _, without the invalid data points when_`skipInvalid()` _is set._
```C++
View QAccelPlot::DataPointStore::renderedView () const
```




<hr>




### function setAttributes {#function-setattributes}

_Replaces the float attributes: empty, or_ `attributeCount` _floats per data point._
```C++
void QAccelPlot::DataPointStore::setAttributes (
    std::vector< float > && attributes
) 
```




<hr>




### function setCategories {#function-setcategories}

_Replaces the categories: empty, or one per data point._ 
```C++
void QAccelPlot::DataPointStore::setCategories (
    std::vector< int > && categories
) 
```



Replacing the data points drops the categories; appending gives the new data point -1. 


        

<hr>




### function setDimensionState {#function-setdimensionstate}

_Updates the scale and viewport of_ _dimension_ _._
```C++
void QAccelPlot::DataPointStore::setDimensionState (
    Dimension dimension,
    const DimensionState & state
) 
```



Marks the render data stale when the scale changes, or when the viewport moves too far from the render origin to keep float precision. 


        

<hr>




### function setDrawn {#function-setdrawn}

_Draws_ _count_ _data points moved from__dataPoints_ _instead of the stored ones._
```C++
std::vector< double > QAccelPlot::DataPointStore::setDrawn (
    std::vector< double > && dataPoints,
    int count
) 
```



Used by a running transition for the frame it shows. _dataPoints_ holds `columnCount()` coordinates per data point, followed by `attributeCount` attributes when the stored data points have attributes. The stored data points, their extents, and `view()` are unchanged. Returns the previous drawn buffer, so a caller drawing frame after frame reuses its allocation. Replacing the stored data points with ones of another stride draws them again. 


        

<hr>




### function setFloat {#function-setfloat}

_Replaces the data points with_ _count_ _float data points moved from__coordinates_ _._
```C++
void QAccelPlot::DataPointStore::setFloat (
    std::vector< float > && coordinates,
    int count,
    std::vector< float > && attributes={}
) 
```



Without attributes the moved buffer becomes the render data as is. With attributes, it is interleaved with them once. 


        

<hr>




### function setLayout {#function-setlayout}

_Clears the store and switches to_ _layout_ _._
```C++
void QAccelPlot::DataPointStore::setLayout (
    Layout layout
) 
```




<hr>




### function setPrecise {#function-setprecise}

_Replaces the data points with_ _count_ _double data points moved from__coordinates_ _._
```C++
void QAccelPlot::DataPointStore::setPrecise (
    std::vector< double > && coordinates,
    int count,
    std::vector< float > && attributes={}
) 
```



_attributes_ is empty, or holds `attributeCount` floats per data point. The sizes must match. 


        

<hr>




### function setSkipInvalid {#function-setskipinvalid}

_Leaves invalid data points out of the render data and_ `renderedView()` _._
```C++
void QAccelPlot::DataPointStore::setSkipInvalid (
    bool skip
) 
```



A data point is invalid when any coordinate is invalid on the current scale. 


        

<hr>




### function skipInvalid {#function-skipinvalid}

_Returns whether invalid data points are left out of the render data._ 
```C++
bool QAccelPlot::DataPointStore::skipInvalid () noexcept const
```




<hr>




### function value {#function-value}

_Returns coordinate_ _column_ _of stored data point__index_ _._
```C++
double QAccelPlot::DataPointStore::value (
    int index,
    int column
) noexcept const
```




<hr>




### function view {#function-view}

_Returns the stored data points._ 
```C++
View QAccelPlot::DataPointStore::view () noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/DataPointStore.hpp`

