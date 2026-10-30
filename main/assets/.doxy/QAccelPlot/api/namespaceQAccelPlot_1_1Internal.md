








# Namespace QAccelPlot::Internal



[**Namespace List**](namespaces.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md)






















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**DataTextureUploadTotals**](structQAccelPlot_1_1Internal_1_1DataTextureUploadTotals.md) <br>_How many data textures were uploaded since the last reset, and how many bytes they held._  |
| struct | [**GrabSize**](structQAccelPlot_1_1Internal_1_1GrabSize.md) <br>_Sizes of one item grab._  |
| struct | [**HeatmapGridDimension**](structQAccelPlot_1_1Internal_1_1HeatmapGridDimension.md) <br>_One dimension of a heatmap grid:_ `count` _cells between the edges_`from` _and_`to` _._ |
| struct | [**HeatmapShaderMapping**](structQAccelPlot_1_1Internal_1_1HeatmapShaderMapping.md) <br>_How heatmap.frag maps an item-local pixel position_ _p_ _to a fractional cell coordinate along one dimension._ |
| class | [**HoverIndexBudget**](classQAccelPlot_1_1Internal_1_1HoverIndexBudget.md) <br>_Decides when queried data is worth a hover index._  |
| class | [**PointerTracker**](classQAccelPlot_1_1Internal_1_1PointerTracker.md) <br>_Tracks the pointer over a plot from the plot's mouse and hover events._  |
| struct | [**RectUbo**](structQAccelPlot_1_1Internal_1_1RectUbo.md) <br>_Mirrors the std140 uniform block of rect.vert._  |
| class | [**ScopedPaintedItemTextures**](classQAccelPlot_1_1Internal_1_1ScopedPaintedItemTextures.md) <br>_Rasterizes the painted items under a root item at a capture scale until destroyed._  |
| struct | [**ValueRange**](structQAccelPlot_1_1Internal_1_1ValueRange.md) <br>_Bounds of an axis viewport or a colormap value range._  |






## Public Attributes

| Type | Name |
| ---: | :--- |
|  constexpr int | [**kDataTextureWidth**](#variable-kdatatexturewidth)   = `{8192}`<br>_Number of floats stored per data texture row, one float per RGBA8 texel._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  constexpr int | [**dataTextureHeight**](#function-datatextureheight) (const int floatCount) <br>_Returns the number of data texture rows needed to store_ _floatCount_ _floats._ |
|  constexpr qint64 | [**dataTextureItemCapacity**](#function-datatextureitemcapacity) (const int maxHeight, const int floatsPerItem) <br>_Returns how many items of_ _floatsPerItem_ _floats fit in a data texture at most__maxHeight_ _rows tall._ |
|  [**DataTextureUploadTotals**](structQAccelPlot_1_1Internal_1_1DataTextureUploadTotals.md) | [**dataTextureUploadTotals**](#function-datatextureuploadtotals) () <br>_Returns the totals since the last reset. Thread-safe._  |
|  qreal | [**edgePixel**](#function-edgepixel) (double value, const [**Axis**](classQAccelPlot_1_1Axis.md) & axis, qreal length) <br>_Maps rectangle edge_ _value_ _on__axis_ _to item pixels along an item side__length_ _pixels long._ |
|  [**GrabSize**](structQAccelPlot_1_1Internal_1_1GrabSize.md) | [**grabSizeFor**](#function-grabsizefor) (const QSize & pixels, qreal devicePixelRatio) <br>_Returns the grab whose image is closest to_ _pixels_ _in a window at__devicePixelRatio_ _._ |
|  bool | [**hoverEnabled**](#function-hoverenabled) () <br>_Returns whether series accept hover events:_ `false` _only when_`QACCELPLOT_HOVER_ENABLED` _is 0._ |
|  QByteArray | [**imageFormatForPath**](#function-imageformatforpath) (const QString & path) <br>_Returns the lowercase_ `QImageWriter` _format for__path's_ _suffix, or an empty array when none can write it._ |
|  std::optional&lt; QPointF &gt; | [**linePathPoint**](#function-linepathpoint) (const [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) & data, int pointCount, int index, int corner, [**StepMode**](namespaceQAccelPlot_1_1StepModeNS.md#enum-mode) mode, bool logScaleX, bool logScaleY) <br>_Returns path point_ _corner_ _of data point__index_ _on the drawn line of__pointCount_ _data points._ |
|  QString | [**localFilePath**](#function-localfilepath) (const QString & pathOrUrl) <br>_Returns_ _pathOrUrl_ _as a local file path; a_`file:` _URL, as a QML file dialog returns, is converted._ |
|  int | [**maxTextureSize**](#function-maxtexturesize) (QQuickWindow \* window) <br>_Returns the largest texture width and height, in pixels, that_ _window's_ _GPU supports._ |
|  [**ValueRange**](structQAccelPlot_1_1Internal_1_1ValueRange.md) | [**pannedRange**](#function-pannedrange) (const [**ValueRange**](structQAccelPlot_1_1Internal_1_1ValueRange.md) & range, qreal fraction, bool logarithmic) <br>_Returns_ _range_ _shifted by__fraction_ _of its extent; a positive__fraction_ _raises both bounds._ |
|  int | [**recordCapacity**](#function-recordcapacity) (QQuickWindow \* window, int floatsPerRecord, int vertexBytesPerRecord) <br>_Returns how many records a series draws that reads them from the data texture by a float vertex id._  |
|  void | [**recordDataTextureUpload**](#function-recorddatatextureupload) (std::int64\_t bytes) <br>_Adds one upload of_ _bytes_ _to the totals. Thread-safe._ |
|  void | [**resetDataTextureUploadTotals**](#function-resetdatatextureuploadtotals) () <br>_Sets the totals back to zero. Thread-safe._  |
|  void | [**setImageResolution**](#function-setimageresolution) (QImage & image, qreal dpi) <br>_Sets the resolution stored with_ _image_ _to__dpi_ _dots per inch._ |
|  std::optional&lt; QPointF &gt; | [**stepCorner**](#function-stepcorner) (const [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) & data, int from, int to, int corner, [**StepMode**](namespaceQAccelPlot_1_1StepModeNS.md#enum-mode) mode, bool logScaleX, bool logScaleY) <br>_Returns corner_ _corner_ _of the step from data point__from_ _to data point__to_ _, in data space._ |
|  int | [**stepCornerCount**](#function-stepcornercount) ([**StepMode**](namespaceQAccelPlot_1_1StepModeNS.md#enum-mode) mode) noexcept<br>_Returns the number of line path points per data point for_ _mode:_ _the data point and the step corners after it._ |
|  bool | [**supportsCustomShaderRendering**](#function-supportscustomshaderrendering) (const QQuickWindow \* window) <br>_Returns_ `true` _when__window_ _renders through a hardware scene graph backend that runs custom shaders._ |
|  void | [**uploadDataTexture**](#function-uploaddatatexture) (std::unique\_ptr&lt; QSGTexture &gt; & texture, QQuickWindow \* window, const QImage & image) <br>_Uploads an image to the live-data texture using the configured Qt API path._  |
|  qreal | [**wheelZoomFactor**](#function-wheelzoomfactor) (qreal zoomScaleFactor, bool zoomingIn, qreal steps=1.0) <br>_Returns the extent multiplier for mouse-wheel zoom with a_ _zoomScaleFactor_ _in (0, 1)._ |
|  std::pair&lt; qreal, qreal &gt; | [**widenedSpan**](#function-widenedspan) (qreal a, qreal b, qreal minimumSize) <br>_Returns the pixel span between edges_ _a_ _and__b_ _, widened around its center to at least__minimumSize_ _._ |
|  QString | [**writeImageFile**](#function-writeimagefile) (const QImage & image, const QString & path, const QByteArray & format, int quality) <br>_Writes_ _image_ _to__path_ _in__format_ _and returns an error message, or an empty string on success._ |
|  void | [**writeRectUniforms**](#function-writerectuniforms) ([**RectUbo**](structQAccelPlot_1_1Internal_1_1RectUbo.md) & ubo, const QSGMaterialShader::RenderState & state, const [**RectMaterial**](classQAccelPlot_1_1RectMaterial.md) & material) <br>_Fills_ _ubo_ _from__material_ _and the matrix and opacity of the render__state_ _._ |
|  [**ValueRange**](structQAccelPlot_1_1Internal_1_1ValueRange.md) | [**zoomedRange**](#function-zoomedrange) (const [**ValueRange**](structQAccelPlot_1_1Internal_1_1ValueRange.md) & range, qreal factor, qreal anchorRatio, bool logarithmic) <br>_Returns_ _range_ _with its extent multiplied by__factor_ _, keeping the value at__anchorRatio_ _in place._ |




























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




### function dataTextureUploadTotals {#function-datatextureuploadtotals}

_Returns the totals since the last reset. Thread-safe._ 
```C++
DataTextureUploadTotals QAccelPlot::Internal::dataTextureUploadTotals () 
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




### function grabSizeFor {#function-grabsizefor}

_Returns the grab whose image is closest to_ _pixels_ _in a window at__devicePixelRatio_ _._
```C++
GrabSize QAccelPlot::Internal::grabSizeFor (
    const QSize & pixels,
    qreal devicePixelRatio
) 
```



Qt 6.2 renders the target size in pixels. Later versions multiply it by the device pixel ratio and round, so some pixel counts cannot be rendered and the image can differ from _pixels_ by a pixel; a tie picks the smaller image. 


        

<hr>




### function hoverEnabled {#function-hoverenabled}

_Returns whether series accept hover events:_ `false` _only when_`QACCELPLOT_HOVER_ENABLED` _is 0._
```C++
bool QAccelPlot::Internal::hoverEnabled () 
```




<hr>




### function imageFormatForPath {#function-imageformatforpath}

_Returns the lowercase_ `QImageWriter` _format for__path's_ _suffix, or an empty array when none can write it._
```C++
QByteArray QAccelPlot::Internal::imageFormatForPath (
    const QString & path
) 
```




<hr>




### function linePathPoint {#function-linepathpoint}

_Returns path point_ _corner_ _of data point__index_ _on the drawn line of__pointCount_ _data points._
```C++
std::optional< QPointF > QAccelPlot::Internal::linePathPoint (
    const CurveDataView & data,
    int pointCount,
    int index,
    int corner,
    StepMode mode,
    bool logScaleX,
    bool logScaleY
) 
```



The last data point has no step after it, so its corners repeat it. Matches the vertices of the line shader. 


        

<hr>




### function localFilePath {#function-localfilepath}

_Returns_ _pathOrUrl_ _as a local file path; a_`file:` _URL, as a QML file dialog returns, is converted._
```C++
QString QAccelPlot::Internal::localFilePath (
    const QString & pathOrUrl
) 
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




### function pannedRange {#function-pannedrange}

_Returns_ _range_ _shifted by__fraction_ _of its extent; a positive__fraction_ _raises both bounds._
```C++
ValueRange QAccelPlot::Internal::pannedRange (
    const ValueRange & range,
    qreal fraction,
    bool logarithmic
) 
```



With _logarithmic_ set, the shift is applied in `log10` space, so both bounds are scaled by the same ratio. A range that is not positive is shifted linearly instead. Returns _range_ unchanged when a bound would leave the range of a double. 


        

<hr>




### function recordCapacity {#function-recordcapacity}

_Returns how many records a series draws that reads them from the data texture by a float vertex id._ 
```C++
int QAccelPlot::Internal::recordCapacity (
    QQuickWindow * window,
    int floatsPerRecord,
    int vertexBytesPerRecord
) 
```



A record takes _floatsPerRecord_ floats of the data texture and _vertexBytesPerRecord_ bytes of vertices. The count is limited by the texture size of _window's_ GPU, by the 2^24 ids a float holds exactly, and by `QSGGeometry`, which sizes its vertex buffer in `int` bytes. 


        

<hr>




### function recordDataTextureUpload {#function-recorddatatextureupload}

_Adds one upload of_ _bytes_ _to the totals. Thread-safe._
```C++
void QAccelPlot::Internal::recordDataTextureUpload (
    std::int64_t bytes
) 
```




<hr>




### function resetDataTextureUploadTotals {#function-resetdatatextureuploadtotals}

_Sets the totals back to zero. Thread-safe._ 
```C++
void QAccelPlot::Internal::resetDataTextureUploadTotals () 
```




<hr>




### function setImageResolution {#function-setimageresolution}

_Sets the resolution stored with_ _image_ _to__dpi_ _dots per inch._
```C++
void QAccelPlot::Internal::setImageResolution (
    QImage & image,
    qreal dpi
) 
```




<hr>




### function stepCorner {#function-stepcorner}

_Returns corner_ _corner_ _of the step from data point__from_ _to data point__to_ _, in data space._
```C++
std::optional< QPointF > QAccelPlot::Internal::stepCorner (
    const CurveDataView & data,
    int from,
    int to,
    int corner,
    StepMode mode,
    bool logScaleX,
    bool logScaleY
) 
```



Corner 0 is data point _from_. A step corner holds the Y of the data point whose flat step it ends or starts, and is drawn when that data point is valid and the X values placing the corner are usable. Returns `std::nullopt` for a corner that is not drawn. Matches the line shader. 


        

<hr>




### function stepCornerCount {#function-stepcornercount}

_Returns the number of line path points per data point for_ _mode:_ _the data point and the step corners after it._
```C++
int QAccelPlot::Internal::stepCornerCount (
    StepMode mode
) noexcept
```




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




### function wheelZoomFactor {#function-wheelzoomfactor}

_Returns the extent multiplier for mouse-wheel zoom with a_ _zoomScaleFactor_ _in (0, 1)._
```C++
qreal QAccelPlot::Internal::wheelZoomFactor (
    qreal zoomScaleFactor,
    bool zoomingIn,
    qreal steps=1.0
) 
```



Zooming in multiplies the extent by the factor and zooming out by its reciprocal. A factor of 0 or below counts as 0.01 and a factor of 1 or above as 0.99, so a step always zooms and never collapses the range. Fractional step counts scale partial wheel notches proportionally. 


        

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




### function writeImageFile {#function-writeimagefile}

_Writes_ _image_ _to__path_ _in__format_ _and returns an error message, or an empty string on success._
```C++
QString QAccelPlot::Internal::writeImageFile (
    const QImage & image,
    const QString & path,
    const QByteArray & format,
    int quality
) 
```



_quality_ is passed to `QImageWriter::setQuality()`. Formats without an alpha channel get the image composited over white. The file is replaced atomically, so a failed write leaves an existing file intact. Safe to call from a worker thread. 


        

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




### function zoomedRange {#function-zoomedrange}

_Returns_ _range_ _with its extent multiplied by__factor_ _, keeping the value at__anchorRatio_ _in place._
```C++
ValueRange QAccelPlot::Internal::zoomedRange (
    const ValueRange & range,
    qreal factor,
    qreal anchorRatio,
    bool logarithmic
) 
```



_anchorRatio_ is a position inside the range: 0 at `min` and 1 at `max`. With _logarithmic_ set, the extent is measured in `log10` space. A range that is not positive is zoomed linearly instead. Returns _range_ unchanged when a bound would leave the range of a double: it would not be finite, or in `log10` space it would reach zero. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/axis/internal/RangeGesture.hpp`

