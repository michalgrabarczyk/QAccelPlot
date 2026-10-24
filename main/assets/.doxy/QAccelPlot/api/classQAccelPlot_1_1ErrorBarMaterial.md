








# Class QAccelPlot::ErrorBarMaterial



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**ErrorBarMaterial**](classQAccelPlot_1_1ErrorBarMaterial.md)



_Material that draws error bars from (x, y, low, high) data points in a data texture._ [More...](#detailed-description)

* `#include <ErrorBarMaterial.hpp>`



Inherits the following classes: [QAccelPlot::DataTextureMaterial](classQAccelPlot_1_1DataTextureMaterial.md)




## Inheritance diagram

```mermaid
flowchart TB
  classQAccelPlot_1_1ErrorBarMaterial["QAccelPlot::ErrorBarMaterial"]

  classQAccelPlot_1_1DataTextureMaterial["QAccelPlot::DataTextureMaterial"]
  classQAccelPlot_1_1DataTextureMaterial --> classQAccelPlot_1_1ErrorBarMaterial
  click classQAccelPlot_1_1DataTextureMaterial "../classQAccelPlot_1_1DataTextureMaterial/" "Open QAccelPlot::DataTextureMaterial"

  external_base_classQAccelPlot_1_1DataTextureMaterial_1["QSGMaterial"]
  external_base_classQAccelPlot_1_1DataTextureMaterial_1 --> classQAccelPlot_1_1DataTextureMaterial

```




















## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**capLength**](#variable-caplength)   = `{8.0f}`<br>_Length of the caps in pixels; 0 draws none._  |
|  float | [**errorBarCount**](#variable-errorbarcount)   = `{0.0f}`<br>_Number of error bars drawn; vertices beyond them are culled._  |
|  float | [**horizontal**](#variable-horizontal)   = `{0.0f}`<br>_1.0 when the bounds lie on the X axis (float for std140 UBO compatibility)._  |
|  QColor | [**hoverColor**](#variable-hovercolor)   = `{Qt::transparent}`<br>_Color of the highlighted error bar._  |
|  float | [**hoveredIndex**](#variable-hoveredindex)   = `{-1.0f}`<br>_Index of the highlighted error bar, or -1 for none._  |
|  float | [**lineWidth**](#variable-linewidth)   = `{1.5f}`<br>_Width of the lines and caps in pixels._  |
|  float | [**offset**](#variable-offset)   = `{0.0f}`<br>_Shift of every error bar across its bounds, in data units._  |


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


## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  constexpr int | [**kVerticesPerErrorBar**](#variable-kverticespererrorbar)   = `{18}`<br>_Vertices per error bar: a quad of two triangles for the line and for each cap._  |




























## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**ErrorBarMaterial**](#function-errorbarmaterial) () <br>_Constructs an_ [_**ErrorBarMaterial**_](classQAccelPlot_1_1ErrorBarMaterial.md) _with default uniform values._ |
|  QSGMaterialShader \* | [**createShader**](#function-createshader) (QSGRendererInterface::RenderMode) override const<br>_Creates and returns the error bar shader program._  |
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
|  const QSGGeometry::AttributeSet & | [**attributeSet**](#function-attributeset) () <br>_Returns the vertex attribute set: a 4-byte placeholder the shader ignores._  |


## Public Static Functions inherited from QAccelPlot::DataTextureMaterial

See [QAccelPlot::DataTextureMaterial](classQAccelPlot_1_1DataTextureMaterial.md)

| Type | Name |
| ---: | :--- |
|  const QSGGeometry::AttributeSet & | [**attributeSet**](classQAccelPlot_1_1DataTextureMaterial.md#function-attributeset) () <br>_Returns the shared vertex attribute set:_ `{float` _id, float param, uchar4 color} (12 bytes)._ |
|  void | [**commitTexture**](classQAccelPlot_1_1DataTextureMaterial.md#function-committexture) (QSGMaterialShader::RenderState & state, int binding, QSGTexture \*\* texture, const [**DataTexture**](classQAccelPlot_1_1DataTexture.md) \* data) <br>_Commits the texture of_ _data_ _to sampler__binding_ _(call from_`updateSampledImage` _). Does nothing before an upload._ |
|  const QSGGeometry::AttributeSet & | [**vertexIndexAttributeSet**](classQAccelPlot_1_1DataTextureMaterial.md#function-vertexindexattributeset) () <br>_Returns the attribute set of marker quads: one ignored_ `uchar4` _placeholder (4 bytes)._ |
|  bool | [**writeCommonUniforms**](classQAccelPlot_1_1DataTextureMaterial.md#function-writecommonuniforms) (char \* buf, int bufSize, const QMatrix4x4 & matrix, const [**DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md) \* mat) <br>_Writes the shared UBO prefix (transform + color + domain + flags) into_ _buf_ _._ |










































## Protected Functions

| Type | Name |
| ---: | :--- |
| virtual int | [**compareExtra**](#function-compareextra) (const QSGMaterial \* other) override const<br>_Compares the error-bar-specific uniforms after the base-class comparison succeeds._  |


## Protected Functions inherited from QAccelPlot::DataTextureMaterial

See [QAccelPlot::DataTextureMaterial](classQAccelPlot_1_1DataTextureMaterial.md)

| Type | Name |
| ---: | :--- |
| virtual int | [**compareExtra**](classQAccelPlot_1_1DataTextureMaterial.md#function-compareextra) (const QSGMaterial \* other) const<br>_Subclass hook for_ `compare()` _— called after the shared fields compare equal._ |






## Detailed Description


Each error bar takes `kVerticesPerErrorBar` vertices: two triangles each for the line and the two caps. The shader derives the error bar, part, and corner from the vertex index. 


    
## Public Attributes Documentation





### variable capLength {#variable-caplength}

_Length of the caps in pixels; 0 draws none._ 
```C++
float QAccelPlot::ErrorBarMaterial::capLength;
```




<hr>




### variable errorBarCount {#variable-errorbarcount}

_Number of error bars drawn; vertices beyond them are culled._ 
```C++
float QAccelPlot::ErrorBarMaterial::errorBarCount;
```




<hr>




### variable horizontal {#variable-horizontal}

_1.0 when the bounds lie on the X axis (float for std140 UBO compatibility)._ 
```C++
float QAccelPlot::ErrorBarMaterial::horizontal;
```




<hr>




### variable hoverColor {#variable-hovercolor}

_Color of the highlighted error bar._ 
```C++
QColor QAccelPlot::ErrorBarMaterial::hoverColor;
```




<hr>




### variable hoveredIndex {#variable-hoveredindex}

_Index of the highlighted error bar, or -1 for none._ 
```C++
float QAccelPlot::ErrorBarMaterial::hoveredIndex;
```




<hr>




### variable lineWidth {#variable-linewidth}

_Width of the lines and caps in pixels._ 
```C++
float QAccelPlot::ErrorBarMaterial::lineWidth;
```




<hr>




### variable offset {#variable-offset}

_Shift of every error bar across its bounds, in data units._ 
```C++
float QAccelPlot::ErrorBarMaterial::offset;
```




<hr>
## Public Static Attributes Documentation





### variable kVerticesPerErrorBar {#variable-kverticespererrorbar}

_Vertices per error bar: a quad of two triangles for the line and for each cap._ 
```C++
constexpr int QAccelPlot::ErrorBarMaterial::kVerticesPerErrorBar;
```




<hr>
## Public Functions Documentation





### function ErrorBarMaterial {#function-errorbarmaterial}

_Constructs an_ [_**ErrorBarMaterial**_](classQAccelPlot_1_1ErrorBarMaterial.md) _with default uniform values._
```C++
QAccelPlot::ErrorBarMaterial::ErrorBarMaterial () 
```




<hr>




### function createShader {#function-createshader}

_Creates and returns the error bar shader program._ 
```C++
QSGMaterialShader * QAccelPlot::ErrorBarMaterial::createShader (
    QSGRendererInterface::RenderMode
) override const
```




<hr>




### function type {#function-type}

_Returns the unique material type identifier for this class._ 
```C++
QSGMaterialType * QAccelPlot::ErrorBarMaterial::type () override const
```




<hr>
## Public Static Functions Documentation





### function attributeSet {#function-attributeset}

_Returns the vertex attribute set: a 4-byte placeholder the shader ignores._ 
```C++
static const QSGGeometry::AttributeSet & QAccelPlot::ErrorBarMaterial::attributeSet () 
```




<hr>
## Protected Functions Documentation





### function compareExtra {#function-compareextra}

_Compares the error-bar-specific uniforms after the base-class comparison succeeds._ 
```C++
virtual int QAccelPlot::ErrorBarMaterial::compareExtra (
    const QSGMaterial * other
) override const
```



Implements [*QAccelPlot::DataTextureMaterial::compareExtra*](classQAccelPlot_1_1DataTextureMaterial.md#function-compareextra)


<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/materials/ErrorBarMaterial.hpp`

