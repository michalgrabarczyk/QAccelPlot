








# Namespace QAccelPlot::LineStroke



[**Namespace List**](namespaces.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**LineStroke**](namespaceQAccelPlot_1_1LineStroke.md)



_Building blocks for the line ribbon drawn by the line shaders, shared by the line renderers._ 


















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**Uniforms**](structQAccelPlot_1_1LineStroke_1_1Uniforms.md) <br>[_**Uniforms**_](structQAccelPlot_1_1LineStroke_1_1Uniforms.md) _of a line material that do not depend on its shader variant._ |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**applyUniforms**](#function-applyuniforms) ([**LineMaterial**](classQAccelPlot_1_1LineMaterial.md) & material, const [**Uniforms**](structQAccelPlot_1_1LineStroke_1_1Uniforms.md) & uniforms) <br>_Copies_ _uniforms_ _into__material_ _._ |
|  std::vector&lt; float &gt; | [**arcLengths**](#function-arclengths) (int pointCount, const std::function&lt; std::optional&lt; QPointF &gt;(int)&gt; & pixelAt) <br>_Returns the cumulative pixel length at each of_ _pointCount_ _data points, for dash patterns._ |
|  QSGGeometryNode \* | [**createNode**](#function-createnode) (int vertexCount, [**LineMaterial**](classQAccelPlot_1_1LineMaterial.md) \* material) <br>_Returns a triangle-strip node with_ _vertexCount_ _line vertices that owns__material_ _._ |
|  bool | [**setMaterial**](#function-setmaterial) (QSGGeometryNode \* node, [**LineMaterial**](classQAccelPlot_1_1LineMaterial.md) \* material) <br>_Gives_ _node_ _, a node from_[_**createNode()**_](namespaceQAccelPlot_1_1LineStroke.md#function-createnode) _, the vertex layout of__material_ _, which it takes ownership of._ |
|  const QSGGeometry::AttributeSet & | [**vertexAttributes**](#function-vertexattributes) (bool dashed) <br>_Returns the vertex attribute set matching_ `DashedLineVertex` _when__dashed_ _, or_`LineVertex` _otherwise._ |
|  int | [**verticesPerPathPoint**](#function-verticesperpathpoint) (bool splitJoins) noexcept<br>_Returns the ribbon vertices of a path point: four with_ _splitJoins_ _, or two otherwise._ |
|  void | [**writeDashedVertices**](#function-writedashedvertices) ([**DashedLineVertex**](structQAccelPlot_1_1DashedLineVertex.md) \* vertices, int pointCount, int cornerCount, bool splitJoins, const std::vector&lt; float &gt; & arcLengths) <br>_Writes the ribbon vertices of_ _pointCount_ _data points of__cornerCount_ _path points each into__vertices_ _._ |
|  void | [**writeVertices**](#function-writevertices) ([**LineVertex**](structQAccelPlot_1_1LineVertex.md) \* vertices, int pointCount, int cornerCount, bool splitJoins, int firstPoint=0) <br>_Writes the ribbon vertices of data points [_ _firstPoint_ _,__pointCount_ _) of__cornerCount_ _path points each into__vertices_ _._ |




























## Public Functions Documentation





### function applyUniforms {#function-applyuniforms}

_Copies_ _uniforms_ _into__material_ _._
```C++
void QAccelPlot::LineStroke::applyUniforms (
    LineMaterial & material,
    const Uniforms & uniforms
) 
```




<hr>




### function arcLengths {#function-arclengths}

_Returns the cumulative pixel length at each of_ _pointCount_ _data points, for dash patterns._
```C++
std::vector< float > QAccelPlot::LineStroke::arcLengths (
    int pointCount,
    const std::function< std::optional< QPointF >(int)> & pixelAt
) 
```



_pixelAt_ returns the pixel position of a data point, or `std::nullopt` for an invalid data point. Segments touching an invalid data point add no length, so the dash phase continues across a gap. 


        

<hr>




### function createNode {#function-createnode}

_Returns a triangle-strip node with_ _vertexCount_ _line vertices that owns__material_ _._
```C++
QSGGeometryNode * QAccelPlot::LineStroke::createNode (
    int vertexCount,
    LineMaterial * material
) 
```



The vertices have the dashed layout when _material_ is dashed. 


        

<hr>




### function setMaterial {#function-setmaterial}

_Gives_ _node_ _, a node from_[_**createNode()**_](namespaceQAccelPlot_1_1LineStroke.md#function-createnode) _, the vertex layout of__material_ _, which it takes ownership of._
```C++
bool QAccelPlot::LineStroke::setMaterial (
    QSGGeometryNode * node,
    LineMaterial * material
) 
```



Replaces the geometry of _node_ with one of the same vertex count, whose vertices are undefined, when the layout differs from the current one, and returns `true` then. 


        

<hr>




### function vertexAttributes {#function-vertexattributes}

_Returns the vertex attribute set matching_ `DashedLineVertex` _when__dashed_ _, or_`LineVertex` _otherwise._
```C++
const QSGGeometry::AttributeSet & QAccelPlot::LineStroke::vertexAttributes (
    bool dashed
) 
```




<hr>




### function verticesPerPathPoint {#function-verticesperpathpoint}

_Returns the ribbon vertices of a path point: four with_ _splitJoins_ _, or two otherwise._
```C++
int QAccelPlot::LineStroke::verticesPerPathPoint (
    bool splitJoins
) noexcept
```



Split joins give a path point one vertex pair that ends the segment before it and one that starts the segment after it, so a join too sharp for a miter draws as a bevel. Without them, both segments share a mitered pair, which suits paths that only turn by right angles. 


        

<hr>




### function writeDashedVertices {#function-writedashedvertices}

_Writes the ribbon vertices of_ _pointCount_ _data points of__cornerCount_ _path points each into__vertices_ _._
```C++
void QAccelPlot::LineStroke::writeDashedVertices (
    DashedLineVertex * vertices,
    int pointCount,
    int cornerCount,
    bool splitJoins,
    const std::vector< float > & arcLengths
) 
```



Each path point gets verticesPerPathPoint(_splitJoins_) vertices. _arcLengths_ holds cumulative lengths of the leading path points. Path points past its end, such as vertices reserved for appended data, repeat its last length, or 0 when it is empty. 


        

<hr>




### function writeVertices {#function-writevertices}

_Writes the ribbon vertices of data points [_ _firstPoint_ _,__pointCount_ _) of__cornerCount_ _path points each into__vertices_ _._
```C++
void QAccelPlot::LineStroke::writeVertices (
    LineVertex * vertices,
    int pointCount,
    int cornerCount,
    bool splitJoins,
    int firstPoint=0
) 
```



Each path point gets verticesPerPathPoint(_splitJoins_) vertices. _vertices_ holds the vertices of all _pointCount_ data points; those before _firstPoint_ are left unchanged. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/LineStroke.hpp`

