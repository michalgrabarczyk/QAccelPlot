








# Class QAccelPlot::BandMaterial



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**BandMaterial**](classQAccelPlot_1_1BandMaterial.md)



_QSGMaterial that fills a band between the low and high values of_ `(x, low, high)` _samples._[More...](#detailed-description)

* `#include <BandMaterial.hpp>`



Inherits the following classes: [QAccelPlot::DataTextureMaterial](classQAccelPlot_1_1DataTextureMaterial.md)




## Inheritance diagram

```mermaid
flowchart TB
  classQAccelPlot_1_1BandMaterial["QAccelPlot::BandMaterial"]

  classQAccelPlot_1_1DataTextureMaterial["QAccelPlot::DataTextureMaterial"]
  classQAccelPlot_1_1DataTextureMaterial --> classQAccelPlot_1_1BandMaterial
  click classQAccelPlot_1_1DataTextureMaterial "../classQAccelPlot_1_1DataTextureMaterial/" "Open QAccelPlot::DataTextureMaterial"

  external_base_classQAccelPlot_1_1DataTextureMaterial_1["QSGMaterial"]
  external_base_classQAccelPlot_1_1DataTextureMaterial_1 --> classQAccelPlot_1_1DataTextureMaterial

```










## Classes

| Type | Name |
| ---: | :--- |
| struct | [**Vertex**](structQAccelPlot_1_1BandMaterial_1_1Vertex.md) <br>[_**Vertex**_](structQAccelPlot_1_1BandMaterial_1_1Vertex.md) _layout of the band triangle strip: two vertices per sample._ |










## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**sampleCount**](#variable-samplecount)   = `{0.0f}`<br>_Number of samples in the data texture (used to clamp neighbor lookups)._  |


## Public Attributes inherited from QAccelPlot::DataTextureMaterial

See [QAccelPlot::DataTextureMaterial](classQAccelPlot_1_1DataTextureMaterial.md)

| Type | Name |
| ---: | :--- |
|  QColor | [**color**](classQAccelPlot_1_1DataTextureMaterial.md#variable-color)   = `{Qt::blue}`<br>_Line/fill color uniform._  |
|  std::shared\_ptr&lt; [**DataTexture**](classQAccelPlot_1_1DataTexture.md) &gt; | [**dataTexture**](classQAccelPlot_1_1DataTextureMaterial.md#variable-datatexture)  <br>_Data texture sampled by the shader. Materials drawing the same samples may share one._  |
|  QVector2D | [**domainMax**](classQAccelPlot_1_1DataTextureMaterial.md#variable-domainmax)   = `{1.0f, 1.0f}`<br>_Maximum data-space coordinate._  |
|  QVector2D | [**domainMin**](classQAccelPlot_1_1DataTextureMaterial.md#variable-domainmin)   = `{0.0f, 0.0f}`<br>_Minimum data-space coordinate._  |
|  float | [**logScaleX**](classQAccelPlot_1_1DataTextureMaterial.md#variable-logscalex)   = `{0.0f}`<br>_1.0 when the X axis uses log scale (float for std140 UBO compatibility)._  |
|  float | [**logScaleY**](classQAccelPlot_1_1DataTextureMaterial.md#variable-logscaley)   = `{0.0f}`<br>_1.0 when the Y axis uses log scale (float for std140 UBO compatibility)._  |
|  float | [**useVertexColor**](classQAccelPlot_1_1DataTextureMaterial.md#variable-usevertexcolor)   = `{0.0f}`<br>_1.0 when per-vertex color overrides_ `color` _(float for std140 UBO compatibility)._ |
|  QVector2D | [**viewportSize**](classQAccelPlot_1_1DataTextureMaterial.md#variable-viewportsize)   = `{800.0f, 600.0f}`<br>_Viewport size in pixels._  |






























## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**BandMaterial**](#function-bandmaterial) () <br>_Constructs a_ [_**BandMaterial**_](classQAccelPlot_1_1BandMaterial.md) _with default uniform values._ |
|  QSGMaterialShader \* | [**createShader**](#function-createshader) (QSGRendererInterface::RenderMode) override const<br>_Creates and returns the band shader program._  |
|  QSGMaterialType \* | [**type**](#function-type) () override const<br>_Returns the unique material type identifier for this class._  |


## Public Functions inherited from QAccelPlot::DataTextureMaterial

See [QAccelPlot::DataTextureMaterial](classQAccelPlot_1_1DataTextureMaterial.md)

| Type | Name |
| ---: | :--- |
|   | [**DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md#function-datatexturematerial) () <br>_Constructs an empty_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _._ |
|  int | [**compare**](classQAccelPlot_1_1DataTextureMaterial.md#function-compare) (const QSGMaterial \* other) override const<br>_Compares shared uniform fields; delegates type-specific fields to_ `compareExtra()` _._ |
|  QSGTexture \* | [**sampledTexture**](classQAccelPlot_1_1DataTextureMaterial.md#function-sampledtexture) () const<br>_Returns the texture the shader samples, or_ `nullptr` _before_`dataTexture` _holds an upload._ |
|  void | [**uploadTexture**](classQAccelPlot_1_1DataTextureMaterial.md#function-uploadtexture) (QQuickWindow \* window, const float \* data, int floatCount) <br>_Uploads_ _floatCount_ _raw floats from__data_ _into_`dataTexture` _, creating it when it is_`nullptr` _._ |
|   | [**~DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md#function-datatexturematerial) () override<br> |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  const QSGGeometry::AttributeSet & | [**vertexAttributes**](#function-vertexattributes) () <br>_Returns the vertex attribute set matching_ `Vertex :` __`{float` _id, float edge} (8 bytes)._ |


## Public Static Functions inherited from QAccelPlot::DataTextureMaterial

See [QAccelPlot::DataTextureMaterial](classQAccelPlot_1_1DataTextureMaterial.md)

| Type | Name |
| ---: | :--- |
|  const QSGGeometry::AttributeSet & | [**attributeSet**](classQAccelPlot_1_1DataTextureMaterial.md#function-attributeset) () <br>_Returns the shared vertex attribute set:_ `{float` _id, float param, uchar4 color} (12 bytes)._ |
|  void | [**commitTexture**](classQAccelPlot_1_1DataTextureMaterial.md#function-committexture) (QSGMaterialShader::RenderState & state, int binding, QSGTexture \*\* texture, const [**DataTexture**](classQAccelPlot_1_1DataTexture.md) \* data) <br>_Commits the texture of_ _data_ _to sampler__binding_ _(call from_`updateSampledImage` _). Does nothing before an upload._ |
|  bool | [**writeCommonUniforms**](classQAccelPlot_1_1DataTextureMaterial.md#function-writecommonuniforms) (char \* buf, int bufSize, const QMatrix4x4 & matrix, const [**DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md) \* mat) <br>_Writes the shared UBO prefix (transform + color + domain + flags) into_ _buf_ _._ |










































## Protected Functions

| Type | Name |
| ---: | :--- |
| virtual int | [**compareExtra**](#function-compareextra) (const QSGMaterial \* other) override const<br>_Compares the band-specific uniforms after the base-class comparison succeeds._  |


## Protected Functions inherited from QAccelPlot::DataTextureMaterial

See [QAccelPlot::DataTextureMaterial](classQAccelPlot_1_1DataTextureMaterial.md)

| Type | Name |
| ---: | :--- |
| virtual int | [**compareExtra**](classQAccelPlot_1_1DataTextureMaterial.md#function-compareextra) (const QSGMaterial \* other) const<br>_Subclass hook for_ `compare()` _— called after the shared fields compare equal._ |






## Detailed Description


The data texture holds three floats per sample. Vertices carry only a sample index and an edge selector, so panning and zooming change uniforms only. 


    
## Public Attributes Documentation





### variable sampleCount {#variable-samplecount}

_Number of samples in the data texture (used to clamp neighbor lookups)._ 
```C++
float QAccelPlot::BandMaterial::sampleCount;
```




<hr>
## Public Functions Documentation





### function BandMaterial {#function-bandmaterial}

_Constructs a_ [_**BandMaterial**_](classQAccelPlot_1_1BandMaterial.md) _with default uniform values._
```C++
QAccelPlot::BandMaterial::BandMaterial () 
```




<hr>




### function createShader {#function-createshader}

_Creates and returns the band shader program._ 
```C++
QSGMaterialShader * QAccelPlot::BandMaterial::createShader (
    QSGRendererInterface::RenderMode
) override const
```




<hr>




### function type {#function-type}

_Returns the unique material type identifier for this class._ 
```C++
QSGMaterialType * QAccelPlot::BandMaterial::type () override const
```




<hr>
## Public Static Functions Documentation





### function vertexAttributes {#function-vertexattributes}

_Returns the vertex attribute set matching_ `Vertex :` __`{float` _id, float edge} (8 bytes)._
```C++
static const QSGGeometry::AttributeSet & QAccelPlot::BandMaterial::vertexAttributes () 
```




<hr>
## Protected Functions Documentation





### function compareExtra {#function-compareextra}

_Compares the band-specific uniforms after the base-class comparison succeeds._ 
```C++
virtual int QAccelPlot::BandMaterial::compareExtra (
    const QSGMaterial * other
) override const
```



Implements [*QAccelPlot::DataTextureMaterial::compareExtra*](classQAccelPlot_1_1DataTextureMaterial.md#function-compareextra)


<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/materials/BandMaterial.hpp`

