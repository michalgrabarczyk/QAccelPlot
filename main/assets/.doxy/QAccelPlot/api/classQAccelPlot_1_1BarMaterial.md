








# Class QAccelPlot::BarMaterial



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**BarMaterial**](classQAccelPlot_1_1BarMaterial.md)



_QSGMaterial for bar series rendering, extending_ [_**RectMaterial**_](classQAccelPlot_1_1RectMaterial.md) _with the bar geometry uniforms._[More...](#detailed-description)

* `#include <BarMaterial.hpp>`



Inherits the following classes: [QAccelPlot::RectMaterial](classQAccelPlot_1_1RectMaterial.md)




## Inheritance diagram

```mermaid
flowchart TB
  classQAccelPlot_1_1BarMaterial["QAccelPlot::BarMaterial"]

  classQAccelPlot_1_1RectMaterial["QAccelPlot::RectMaterial"]
  classQAccelPlot_1_1RectMaterial --> classQAccelPlot_1_1BarMaterial
  click classQAccelPlot_1_1RectMaterial "../classQAccelPlot_1_1RectMaterial/" "Open QAccelPlot::RectMaterial"

  classQAccelPlot_1_1DataTextureMaterial["QAccelPlot::DataTextureMaterial"]
  classQAccelPlot_1_1DataTextureMaterial --> classQAccelPlot_1_1RectMaterial
  click classQAccelPlot_1_1DataTextureMaterial "../classQAccelPlot_1_1DataTextureMaterial/" "Open QAccelPlot::DataTextureMaterial"

  external_base_classQAccelPlot_1_1DataTextureMaterial_1["QSGMaterial"]
  external_base_classQAccelPlot_1_1DataTextureMaterial_1 --> classQAccelPlot_1_1DataTextureMaterial

```
























## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**barOffset**](#variable-baroffset)   = `{0.0f}`<br>_Shift of every bar along the position axis in data units._  |
|  float | [**barWidth**](#variable-barwidth)   = `{0.8f}`<br>_Bar width in position-axis data units._  |
|  float | [**baseline**](#variable-baseline)   = `{0.0f}`<br>_Value the bars start from, relative to the value-axis render origin._  |
|  float | [**horizontal**](#variable-horizontal)   = `{0.0f}`<br>_1.0 when positions lie on the Y axis (float for std140 UBO compatibility)._  |


## Public Attributes inherited from QAccelPlot::RectMaterial

See [QAccelPlot::RectMaterial](classQAccelPlot_1_1RectMaterial.md)

| Type | Name |
| ---: | :--- |
|  QColor | [**borderColor**](classQAccelPlot_1_1RectMaterial.md#variable-bordercolor)   = `{Qt::black}`<br>_Outline color._  |
|  float | [**borderWidth**](classQAccelPlot_1_1RectMaterial.md#variable-borderwidth)   = `{0.0f}`<br>_Outline width in pixels, drawn inside each rectangle._  |
|  QColor | [**hoverColor**](classQAccelPlot_1_1RectMaterial.md#variable-hovercolor)   = `{Qt::transparent}`<br>_Fill color of the highlighted rectangle._  |
|  float | [**hoveredIndex**](classQAccelPlot_1_1RectMaterial.md#variable-hoveredindex)   = `{-1.0f}`<br>_Index of the highlighted rectangle, or -1 for none._  |
|  QVector2D | [**minimumSize**](classQAccelPlot_1_1RectMaterial.md#variable-minimumsize)   = `{0.0f, 0.0f}`<br>_Minimum drawn rectangle width and height in pixels._  |
|  float | [**rectCount**](classQAccelPlot_1_1RectMaterial.md#variable-rectcount)   = `{0.0f}`<br>_Number of rectangles in the data texture (used to index the sampler)._  |


## Public Attributes inherited from QAccelPlot::DataTextureMaterial

See [QAccelPlot::DataTextureMaterial](classQAccelPlot_1_1DataTextureMaterial.md)

| Type | Name |
| ---: | :--- |
|  QColor | [**color**](classQAccelPlot_1_1DataTextureMaterial.md#variable-color)   = `{Qt::blue}`<br>_Line/fill color uniform._  |
|  std::unique\_ptr&lt; QSGTexture &gt; | [**dataTexture**](classQAccelPlot_1_1DataTextureMaterial.md#variable-datatexture)  <br>_Owned data texture bound to the shader sampler._  |
|  QVector2D | [**domainMax**](classQAccelPlot_1_1DataTextureMaterial.md#variable-domainmax)   = `{1.0f, 1.0f}`<br>_Maximum data-space coordinate._  |
|  QVector2D | [**domainMin**](classQAccelPlot_1_1DataTextureMaterial.md#variable-domainmin)   = `{0.0f, 0.0f}`<br>_Minimum data-space coordinate._  |
|  float | [**logScaleX**](classQAccelPlot_1_1DataTextureMaterial.md#variable-logscalex)   = `{0.0f}`<br>_1.0 when the X axis uses log scale (float for std140 UBO compatibility)._  |
|  float | [**logScaleY**](classQAccelPlot_1_1DataTextureMaterial.md#variable-logscaley)   = `{0.0f}`<br>_1.0 when the Y axis uses log scale (float for std140 UBO compatibility)._  |
|  float | [**useVertexColor**](classQAccelPlot_1_1DataTextureMaterial.md#variable-usevertexcolor)   = `{0.0f}`<br>_1.0 when per-vertex color overrides_ `color` _(float for std140 UBO compatibility)._ |
|  QVector2D | [**viewportSize**](classQAccelPlot_1_1DataTextureMaterial.md#variable-viewportsize)   = `{800.0f, 600.0f}`<br>_Viewport size in pixels._  |












































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**BarMaterial**](#function-barmaterial) () <br>_Constructs a_ [_**BarMaterial**_](classQAccelPlot_1_1BarMaterial.md) _with default uniform values._ |
|  QSGMaterialShader \* | [**createShader**](#function-createshader) (QSGRendererInterface::RenderMode) override const<br>_Creates and returns the bar shader program._  |
|  QSGMaterialType \* | [**type**](#function-type) () override const<br>_Returns the unique material type identifier for this class._  |


## Public Functions inherited from QAccelPlot::RectMaterial

See [QAccelPlot::RectMaterial](classQAccelPlot_1_1RectMaterial.md)

| Type | Name |
| ---: | :--- |
|   | [**RectMaterial**](classQAccelPlot_1_1RectMaterial.md#function-rectmaterial) () <br>_Constructs a_ [_**RectMaterial**_](classQAccelPlot_1_1RectMaterial.md) _with default uniform values._ |
|  QSGMaterialShader \* | [**createShader**](classQAccelPlot_1_1RectMaterial.md#function-createshader) (QSGRendererInterface::RenderMode) override const<br>_Creates and returns the rectangle shader program._  |
|  QSGMaterialType \* | [**type**](classQAccelPlot_1_1RectMaterial.md#function-type) () override const<br>_Returns the unique material type identifier for this class._  |


## Public Functions inherited from QAccelPlot::DataTextureMaterial

See [QAccelPlot::DataTextureMaterial](classQAccelPlot_1_1DataTextureMaterial.md)

| Type | Name |
| ---: | :--- |
|   | [**DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md#function-datatexturematerial) () <br>_Constructs an empty_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _._ |
|  int | [**compare**](classQAccelPlot_1_1DataTextureMaterial.md#function-compare) (const QSGMaterial \* other) override const<br>_Compares shared uniform fields; delegates type-specific fields to_ `compareExtra()` _._ |
|  void | [**uploadTexture**](classQAccelPlot_1_1DataTextureMaterial.md#function-uploadtexture) (std::unique\_ptr&lt; QSGTexture &gt; & texture, QQuickWindow \* window, const float \* data, int floatCount) <br>_Uploads_ _floatCount_ _raw floats as an RGBA8888 data texture, reusing existing GPU/CPU buffers where possible._ |
|   | [**~DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md#function-datatexturematerial) () override<br> |






## Public Static Functions inherited from QAccelPlot::DataTextureMaterial

See [QAccelPlot::DataTextureMaterial](classQAccelPlot_1_1DataTextureMaterial.md)

| Type | Name |
| ---: | :--- |
|  const QSGGeometry::AttributeSet & | [**attributeSet**](classQAccelPlot_1_1DataTextureMaterial.md#function-attributeset) () <br>_Returns the shared vertex attribute set:_ `{float` _id, float param, uchar4 color} (12 bytes)._ |
|  void | [**commitTexture**](classQAccelPlot_1_1DataTextureMaterial.md#function-committexture) (QSGMaterialShader::RenderState & state, int binding, QSGTexture \*\* texture, QSGTexture \* dataTexture) <br>_Commits_ _dataTexture_ _to sampler__binding_ _(call from_`updateSampledImage` _)._ |
|  bool | [**writeCommonUniforms**](classQAccelPlot_1_1DataTextureMaterial.md#function-writecommonuniforms) (char \* buf, int bufSize, const QMatrix4x4 & matrix, const [**DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md) \* mat) <br>_Writes the shared UBO prefix (transform + color + domain + flags) into_ _buf_ _._ |






























































## Protected Functions

| Type | Name |
| ---: | :--- |
| virtual int | [**compareExtra**](#function-compareextra) (const QSGMaterial \* other) override const<br>_Compares the bar-specific uniforms after the rectangle uniforms compare equal._  |


## Protected Functions inherited from QAccelPlot::RectMaterial

See [QAccelPlot::RectMaterial](classQAccelPlot_1_1RectMaterial.md)

| Type | Name |
| ---: | :--- |
| virtual int | [**compareExtra**](classQAccelPlot_1_1RectMaterial.md#function-compareextra) (const QSGMaterial \* other) override const<br>_Compares the rectangle-specific uniforms after the base-class comparison succeeds._  |


## Protected Functions inherited from QAccelPlot::DataTextureMaterial

See [QAccelPlot::DataTextureMaterial](classQAccelPlot_1_1DataTextureMaterial.md)

| Type | Name |
| ---: | :--- |
| virtual int | [**compareExtra**](classQAccelPlot_1_1DataTextureMaterial.md#function-compareextra) (const QSGMaterial \* other) const<br>_Subclass hook for_ `compare()` _— called after the shared fields compare equal._ |








## Detailed Description


The data texture holds (position, value) pairs; the shader turns each into a rectangle. 


    
## Public Attributes Documentation





### variable barOffset {#variable-baroffset}

_Shift of every bar along the position axis in data units._ 
```C++
float QAccelPlot::BarMaterial::barOffset;
```




<hr>




### variable barWidth {#variable-barwidth}

_Bar width in position-axis data units._ 
```C++
float QAccelPlot::BarMaterial::barWidth;
```




<hr>




### variable baseline {#variable-baseline}

_Value the bars start from, relative to the value-axis render origin._ 
```C++
float QAccelPlot::BarMaterial::baseline;
```




<hr>




### variable horizontal {#variable-horizontal}

_1.0 when positions lie on the Y axis (float for std140 UBO compatibility)._ 
```C++
float QAccelPlot::BarMaterial::horizontal;
```




<hr>
## Public Functions Documentation





### function BarMaterial {#function-barmaterial}

_Constructs a_ [_**BarMaterial**_](classQAccelPlot_1_1BarMaterial.md) _with default uniform values._
```C++
QAccelPlot::BarMaterial::BarMaterial () 
```




<hr>




### function createShader {#function-createshader}

_Creates and returns the bar shader program._ 
```C++
QSGMaterialShader * QAccelPlot::BarMaterial::createShader (
    QSGRendererInterface::RenderMode
) override const
```




<hr>




### function type {#function-type}

_Returns the unique material type identifier for this class._ 
```C++
QSGMaterialType * QAccelPlot::BarMaterial::type () override const
```




<hr>
## Protected Functions Documentation





### function compareExtra {#function-compareextra}

_Compares the bar-specific uniforms after the rectangle uniforms compare equal._ 
```C++
virtual int QAccelPlot::BarMaterial::compareExtra (
    const QSGMaterial * other
) override const
```



Implements [*QAccelPlot::DataTextureMaterial::compareExtra*](classQAccelPlot_1_1DataTextureMaterial.md#function-compareextra)


<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/materials/BarMaterial.hpp`

