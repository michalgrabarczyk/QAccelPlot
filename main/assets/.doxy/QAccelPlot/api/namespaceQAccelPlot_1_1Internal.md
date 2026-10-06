








# Namespace QAccelPlot::Internal



[**Namespace List**](namespaces.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md)






















## Classes

| Type | Name |
| ---: | :--- |
| class | [**HoverIndexBudget**](classQAccelPlot_1_1Internal_1_1HoverIndexBudget.md) <br>_Decides when queried data is worth a hover index._  |
| struct | [**RectUbo**](structQAccelPlot_1_1Internal_1_1RectUbo.md) <br>_Mirrors the std140 uniform block of rect.vert._  |






## Public Attributes

| Type | Name |
| ---: | :--- |
|  constexpr int | [**kDataTextureWidth**](#variable-kdatatexturewidth)   = `{8192}`<br>_Number of floats stored per data texture row, one float per RGBA8 texel._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  constexpr int | [**dataTextureHeight**](#function-datatextureheight) (const int floatCount) <br>_Returns the number of data texture rows needed to store_ _floatCount_ _floats._ |
|  constexpr qint64 | [**dataTextureItemCapacity**](#function-datatextureitemcapacity) (const int maxHeight, const int floatsPerItem) <br>_Returns how many items of_ _floatsPerItem_ _floats fit in a data texture at most__maxHeight_ _rows tall._ |
|  qreal | [**edgePixel**](#function-edgepixel) (double value, const [**Axis**](classQAccelPlot_1_1Axis.md) & axis, qreal length) <br>_Maps rectangle edge_ _value_ _on__axis_ _to item pixels along an item side__length_ _pixels long._ |
|  bool | [**hoverEnabled**](#function-hoverenabled) () <br>_Returns whether series accept hover events:_ `false` _only when_`QACCELPLOT_HOVER_ENABLED` _is 0._ |
|  int | [**maxTextureSize**](#function-maxtexturesize) (QQuickWindow \* window) <br>_Returns the largest texture width and height, in pixels, that_ _window's_ _GPU supports._ |
|  bool | [**supportsCustomShaderRendering**](#function-supportscustomshaderrendering) (const QQuickWindow \* window) <br>_Returns_ `true` _when__window_ _renders through a hardware scene graph backend that runs custom shaders._ |
|  void | [**uploadDataTexture**](#function-uploaddatatexture) (std::unique\_ptr&lt; QSGTexture &gt; & texture, QQuickWindow \* window, const QImage & image) <br>_Uploads an image to the live-data texture using the configured Qt API path._  |
|  std::pair&lt; qreal, qreal &gt; | [**widenedSpan**](#function-widenedspan) (qreal a, qreal b, qreal minimumSize) <br>_Returns the pixel span between edges_ _a_ _and__b_ _, widened around its center to at least__minimumSize_ _._ |
|  void | [**writeRectUniforms**](#function-writerectuniforms) ([**RectUbo**](structQAccelPlot_1_1Internal_1_1RectUbo.md) & ubo, const QSGMaterialShader::RenderState & state, const [**RectMaterial**](classQAccelPlot_1_1RectMaterial.md) & material) <br>_Fills_ _ubo_ _from__material_ _and the matrix and opacity of the render__state_ _._ |




























## Public Attributes Documentation





### variable kDataTextureWidth {#variable-kdatatexturewidth}

_Number of floats stored per data texture row, one float per RGBA8 texel._ 
```C++
constexpr int QAccelPlot::Internal::kDataTextureWidth;
```



data\_texture.glsl reads the width from the bound texture, so this is the only definition. 


        

<hr>
## Public Functions Documentation





### function dataTextureHeight {#function-datatextureheight}

_Returns the number of data texture rows needed to store_ _floatCount_ _floats._
```C++
constexpr int QAccelPlot::Internal::dataTextureHeight (
    const int floatCount
) 
```




<hr>




### function dataTextureItemCapacity {#function-datatextureitemcapacity}

_Returns how many items of_ _floatsPerItem_ _floats fit in a data texture at most__maxHeight_ _rows tall._
```C++
constexpr qint64 QAccelPlot::Internal::dataTextureItemCapacity (
    const int maxHeight,
    const int floatsPerItem
) 
```




<hr>




### function edgePixel {#function-edgepixel}

_Maps rectangle edge_ _value_ _on__axis_ _to item pixels along an item side__length_ _pixels long._
```C++
qreal QAccelPlot::Internal::edgePixel (
    double value,
    const Axis & axis,
    qreal length
) 
```



Infinite edges, and non-positive edges on a log axis, map to infinity on the matching side, as rect\_geometry.glsl extends them past the plot edge. 


        

<hr>




### function hoverEnabled {#function-hoverenabled}

_Returns whether series accept hover events:_ `false` _only when_`QACCELPLOT_HOVER_ENABLED` _is 0._
```C++
bool QAccelPlot::Internal::hoverEnabled () 
```




<hr>




### function maxTextureSize {#function-maxtexturesize}

_Returns the largest texture width and height, in pixels, that_ _window's_ _GPU supports._
```C++
int QAccelPlot::Internal::maxTextureSize (
    QQuickWindow * window
) 
```



Falls back to 8192, supported by every target GPU, when the RHI cannot be queried (Qt older than 6.6, or a build without the private Qt API). `QACCELPLOT_MAX_TEXTURE_SIZE` lowers the result, for example to exercise capacity limits in tests. 


        

<hr>




### function supportsCustomShaderRendering {#function-supportscustomshaderrendering}

_Returns_ `true` _when__window_ _renders through a hardware scene graph backend that runs custom shaders._
```C++
bool QAccelPlot::Internal::supportsCustomShaderRendering (
    const QQuickWindow * window
) 
```




<hr>




### function uploadDataTexture {#function-uploaddatatexture}

_Uploads an image to the live-data texture using the configured Qt API path._ 
```C++
void QAccelPlot::Internal::uploadDataTexture (
    std::unique_ptr< QSGTexture > & texture,
    QQuickWindow * window,
    const QImage & image
) 
```




<hr>




### function widenedSpan {#function-widenedspan}

_Returns the pixel span between edges_ _a_ _and__b_ _, widened around its center to at least__minimumSize_ _._
```C++
std::pair< qreal, qreal > QAccelPlot::Internal::widenedSpan (
    qreal a,
    qreal b,
    qreal minimumSize
) 
```



Matches `widenedSpan()` in rect\_geometry.glsl, so hit tests use the drawn size. 


        

<hr>




### function writeRectUniforms {#function-writerectuniforms}

_Fills_ _ubo_ _from__material_ _and the matrix and opacity of the render__state_ _._
```C++
void QAccelPlot::Internal::writeRectUniforms (
    RectUbo & ubo,
    const QSGMaterialShader::RenderState & state,
    const RectMaterial & material
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/materials/internal/DataTextureLayout.hpp`

