








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
|  std::vector&lt; float &gt; | [**arcLengths**](#function-arclengths) (const int pointCount, const PixelAt & pixelAt) <br>_Returns the cumulative pixel length at each of_ _pointCount_ _samples, for dash patterns._ |
|  QSGGeometryNode \* | [**createNode**](#function-createnode) (int vertexCount, QSGMaterial \* material) <br>_Returns a triangle-strip node with_ _vertexCount_ _line vertices that owns__material_ _._ |
|  const QSGGeometry::AttributeSet & | [**vertexAttributes**](#function-vertexattributes) () <br>_Returns the vertex attribute set matching_ `LineVertex` _._ |
|  void | [**writeVertices**](#function-writevertices) ([**LineVertex**](structQAccelPlot_1_1LineVertex.md) \* vertices, int pointCount, const QColor & color, const std::vector&lt; float &gt; & arcLengths) <br>_Writes two ribbon vertices per sample for_ _pointCount_ _samples into__vertices_ _._ |




























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

_Returns the cumulative pixel length at each of_ _pointCount_ _samples, for dash patterns._
```C++
template<typename PixelAt>
std::vector< float > QAccelPlot::LineStroke::arcLengths (
    const int pointCount,
    const PixelAt & pixelAt
) 
```



_pixelAt_ returns the pixel position of a sample, or `std::nullopt` for an invalid sample. Segments touching an invalid sample add no length, so the dash phase continues across a gap. 


        

<hr>




### function createNode {#function-createnode}

_Returns a triangle-strip node with_ _vertexCount_ _line vertices that owns__material_ _._
```C++
QSGGeometryNode * QAccelPlot::LineStroke::createNode (
    int vertexCount,
    QSGMaterial * material
) 
```




<hr>




### function vertexAttributes {#function-vertexattributes}

_Returns the vertex attribute set matching_ `LineVertex` _._
```C++
const QSGGeometry::AttributeSet & QAccelPlot::LineStroke::vertexAttributes () 
```




<hr>




### function writeVertices {#function-writevertices}

_Writes two ribbon vertices per sample for_ _pointCount_ _samples into__vertices_ _._
```C++
void QAccelPlot::LineStroke::writeVertices (
    LineVertex * vertices,
    int pointCount,
    const QColor & color,
    const std::vector< float > & arcLengths
) 
```



_arcLengths_ holds cumulative lengths of the leading samples, or is empty for solid lines. Samples past its end, such as vertices reserved for appended data, repeat its last length. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/renderers/LineStroke.hpp`

