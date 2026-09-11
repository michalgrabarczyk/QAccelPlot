








# Namespace QAccelPlot



[**Namespace List**](namespaces.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md)




















## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**GradientDirectionNS**](namespaceQAccelPlot_1_1GradientDirectionNS.md) <br>_Namespace exposing the_ `GradientDirection` _enum to QML._ |
| namespace | [**GradientFillBaselineNS**](namespaceQAccelPlot_1_1GradientFillBaselineNS.md) <br>_Namespace exposing the_ `GradientFillBaseline` _enum to QML._ |
| namespace | [**GradientValueSourceNS**](namespaceQAccelPlot_1_1GradientValueSourceNS.md) <br>_Namespace exposing the_ `GradientValueSource` _enum to QML._ |
| namespace | [**Internal**](namespaceQAccelPlot_1_1Internal.md) <br> |


## Classes

| Type | Name |
| ---: | :--- |
| class | [**Axis**](classQAccelPlot_1_1Axis.md) <br>_A visual axis item that maps a data-space range to pixel coordinates and renders tick marks and labels._  |
| class | [**AxisTickPainter**](classQAccelPlot_1_1AxisTickPainter.md) <br>_Internal helper that paints tick marks and labels onto a QPainter for a single_ [_**Axis**_](classQAccelPlot_1_1Axis.md) _._ |
| class | [**AxisTicker**](classQAccelPlot_1_1AxisTicker.md) <br>_Controls the visual appearance of ticks, sub-ticks, and tick labels on an_ `Axis` _._ |
| struct | [**CurveChunk**](structQAccelPlot_1_1CurveChunk.md) <br>_Axis-aligned bounding box (AABB) for a contiguous block of curve points, used for hit-test culling._  |
| struct | [**CurveHitTestParams**](structQAccelPlot_1_1CurveHitTestParams.md) <br>_All inputs required for a_ `contains()` _hit-test, bundled to reduce parameter count._ |
| class | [**DashLine**](classQAccelPlot_1_1DashLine.md) <br>_A line style that renders the curve as a customisable dashed line._  |
| struct | [**DashParameters**](structQAccelPlot_1_1DashParameters.md) <br>_Plain-data snapshot of dash rendering parameters._  |
| class | [**DataAnchor**](classQAccelPlot_1_1DataAnchor.md) <br>_A QQuickItem that tracks a data-coordinate rectangle in pixel space._  |
| class | [**DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md) <br>_Base QSGMaterial that uploads curve data as a floating-point texture and exposes shared shader uniforms._  |
| class | [**DataTransition**](classQAccelPlot_1_1DataTransition.md) <br>_Abstract base class for animated data transitions on plot elements._  |
| class | [**DateTimeTickLabelFormatter**](classQAccelPlot_1_1DateTimeTickLabelFormatter.md) <br>_A tick label formatter that displays tick values as formatted date/time strings._  |
| class | [**DrawTransition**](classQAccelPlot_1_1DrawTransition.md) <br>_An animation transition that reveals the target curve by drawing it point-by-point from start to end._  |
| struct | [**GradientColorPayload**](structQAccelPlot_1_1GradientColorPayload.md) <br>_Render-thread snapshot of gradient stroke (line-color) parameters._  |
| class | [**GradientFill**](classQAccelPlot_1_1GradientFill.md) <br>_A_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _effect that fills the area under the curve with a color gradient._ |
| class | [**GradientFillMaterial**](classQAccelPlot_1_1GradientFillMaterial.md) <br>_Scene-graph material that evaluates fill gradients per fragment._  |
| struct | [**GradientFillPayload**](structQAccelPlot_1_1GradientFillPayload.md) <br>_Render-thread snapshot of gradient fill (area-under-curve) parameters._  |
| class | [**GradientLineMaterial**](classQAccelPlot_1_1GradientLineMaterial.md) <br>_Line material variant that samples a one-dimensional gradient texture._  |
| struct | [**GradientStopData**](structQAccelPlot_1_1GradientStopData.md) <br>_A single color stop within a gradient definition._  |
| class | [**GradientStroke**](classQAccelPlot_1_1GradientStroke.md) <br>_A_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _effect that replaces the solid line color with a color gradient._ |
| class | [**GradientTexture**](classQAccelPlot_1_1GradientTexture.md) <br>_Cached one-dimensional texture used by gradient materials._  |
| class | [**Grid**](classQAccelPlot_1_1Grid.md) <br>_Configuration object that controls the appearance of the plot grid._  |
| class | [**GridNode**](classQAccelPlot_1_1GridNode.md) <br>_Internal QSGNode responsible for rendering the plot grid into the scene graph._  |
| class | [**LineCurve**](classQAccelPlot_1_1LineCurve.md) <br>_A hardware-accelerated QML item that renders a 2D line curve with optional markers, dashing, and gradient effects._  |
| class | [**LineCurveEffect**](classQAccelPlot_1_1LineCurveEffect.md) <br>_Abstract base class for visual effects applied to a_ `LineCurve` _._ |
| class | [**LineCurveLineRenderer**](classQAccelPlot_1_1LineCurveLineRenderer.md) <br>_Internal renderer responsible for building and updating QSGNode line geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._ |
| class | [**LineCurvePointRenderer**](classQAccelPlot_1_1LineCurvePointRenderer.md) <br>_Internal renderer responsible for building and updating QSGNode marker geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._ |
| struct | [**LineCurveRenderParams**](structQAccelPlot_1_1LineCurveRenderParams.md) <br>_Input parameters for_ [_**LineCurveLineRenderer::paint()**_](classQAccelPlot_1_1LineCurveLineRenderer.md#function-paint) _, assembled on the main thread._ |
| class | [**LineCurveVertexCache**](classQAccelPlot_1_1LineCurveVertexCache.md) <br>_Owns a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _'s pre-built vertex bytes and the metadata required to use them safely._ |
| class | [**LineMaterial**](classQAccelPlot_1_1LineMaterial.md) <br>_QSGMaterial for line rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with line-specific uniforms._ |
| class | [**LineStyle**](classQAccelPlot_1_1LineStyle.md) <br>_Abstract base class for all line styles._  |
| struct | [**LineVertex**](structQAccelPlot_1_1LineVertex.md) <br>_Vertex layout for line geometry, shared with the main thread for pre-built vertex caches._  |
| class | [**LogTickLabelFormatter**](classQAccelPlot_1_1LogTickLabelFormatter.md) <br>_A tick label formatter that produces power-of-ten labels for logarithmic axes._  |
| class | [**MorphTransition**](classQAccelPlot_1_1MorphTransition.md) <br>_An animation transition that smoothly interpolates point positions between two datasets._  |
| class | [**NoLine**](classQAccelPlot_1_1NoLine.md) <br>_A line style that suppresses line rendering entirely, leaving only markers visible._  |
| class | [**NumericTickLabelFormatter**](classQAccelPlot_1_1NumericTickLabelFormatter.md) <br>_The default tick label formatter — produces numeric labels with automatic decimal precision._  |
| class | [**PlotBorder**](classQAccelPlot_1_1PlotBorder.md) <br>_Decorative frame configuration exposed by_ `PlotView::border` _._ |
| class | [**PlotMouseEvent**](classQAccelPlot_1_1PlotMouseEvent.md) <br>_Carries mouse event data for the mouse signals._  |
| class | [**PlotSeries**](classQAccelPlot_1_1PlotSeries.md) <br>_Common QML item contract for data series hosted by_ `PlotView` _._ |
| struct | [**PointCurveRenderParams**](structQAccelPlot_1_1PointCurveRenderParams.md) <br>_Input parameters for_ [_**LineCurvePointRenderer::paint()**_](classQAccelPlot_1_1LineCurvePointRenderer.md#function-paint) _, assembled on the main thread._ |
| class | [**PointMaterial**](classQAccelPlot_1_1PointMaterial.md) <br>_QSGMaterial for marker (point) rendering._  |
| struct | [**PointVertex**](structQAccelPlot_1_1PointVertex.md) <br>_Vertex layout for point (marker) geometry, shared with the main thread for vertex caches._  |
| class | [**QAccelPlot**](classQAccelPlot_1_1QAccelPlot.md) <br>_The main plot canvas QML item — hosts axes, curves, and a grid._  |
| class | [**RectMaterial**](classQAccelPlot_1_1RectMaterial.md) <br>_QSGMaterial for rectangle list rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with a rect-count uniform._ |
| class | [**RectangleList**](classQAccelPlot_1_1RectangleList.md) <br>_A hardware-accelerated QML item that renders a large list of axis-aligned rectangles._  |
| class | [**SolidLine**](classQAccelPlot_1_1SolidLine.md) <br>_The default line style — renders a continuous solid line with no gaps._  |
| class | [**SpatialGrid**](classQAccelPlot_1_1SpatialGrid.md) <br>_Uniform-grid spatial index for O(1) point-in-rectangle hit-test queries._  |
| class | [**TextTickLabelFormatter**](classQAccelPlot_1_1TextTickLabelFormatter.md) <br>_A tick label formatter that maps integer tick indices to a user-supplied list of strings._  |
| class | [**TickLabelFormatter**](classQAccelPlot_1_1TickLabelFormatter.md) <br>_Abstract base class for tick label formatters._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef [**GradientDirectionNS::Direction**](namespaceQAccelPlot_1_1GradientDirectionNS.md#enum-direction) | [**GradientDirection**](#typedef-gradientdirection)  <br> |
| typedef [**GradientFillBaselineNS::Mode**](namespaceQAccelPlot_1_1GradientFillBaselineNS.md#enum-mode) | [**GradientFillBaseline**](#typedef-gradientfillbaseline)  <br> |
| typedef [**GradientValueSourceNS::Source**](namespaceQAccelPlot_1_1GradientValueSourceNS.md#enum-source) | [**GradientValueSource**](#typedef-gradientvaluesource)  <br> |




## Public Attributes

| Type | Name |
| ---: | :--- |
|  constexpr double | [**kNearlyEqualEpsilon**](#variable-knearlyequalepsilon)   = `1e-12`<br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**appendStopFromObject**](#function-appendstopfromobject) (std::vector&lt; [**GradientStopData**](structQAccelPlot_1_1GradientStopData.md) &gt; & outStops, QObject \* stopObject) <br>_Reads a QML gradient stop object and appends it to_ _outStops_ _._ |
|  bool | [**nearly\_equal**](#function-nearly_equal) (double a, double b, double eps\_rel=kNearlyEqualEpsilon, double eps\_abs=std::numeric\_limits&lt; double &gt;::min()) noexcept<br> |
|  float | [**unboundedGradientCoordinate**](#function-unboundedgradientcoordinate) (const [**GradientDirection**](namespaceQAccelPlot_1_1GradientDirectionNS.md#enum-direction) direction, const float value, const float minimum, const float maximum) <br>_Returns an unbounded palette coordinate for a data-space_ _value_ _._ |




























## Public Types Documentation





### typedef GradientDirection {#typedef-gradientdirection}

```C++
using QAccelPlot::GradientDirection = typedef GradientDirectionNS::Direction;
```




<hr>




### typedef GradientFillBaseline {#typedef-gradientfillbaseline}

```C++
using QAccelPlot::GradientFillBaseline = typedef GradientFillBaselineNS::Mode;
```




<hr>




### typedef GradientValueSource {#typedef-gradientvaluesource}

```C++
using QAccelPlot::GradientValueSource = typedef GradientValueSourceNS::Source;
```




<hr>
## Public Attributes Documentation





### variable kNearlyEqualEpsilon {#variable-knearlyequalepsilon}

```C++
constexpr double QAccelPlot::kNearlyEqualEpsilon;
```




<hr>
## Public Functions Documentation





### function appendStopFromObject {#function-appendstopfromobject}

_Reads a QML gradient stop object and appends it to_ _outStops_ _._
```C++
inline void QAccelPlot::appendStopFromObject (
    std::vector< GradientStopData > & outStops,
    QObject * stopObject
) 
```



The stop object is expected to expose `position` (real) and `color` (color) properties via the QML meta-object system. 


        

<hr>




### function nearly\_equal {#function-nearly_equal}

```C++
inline bool QAccelPlot::nearly_equal (
    double a,
    double b,
    double eps_rel=kNearlyEqualEpsilon,
    double eps_abs=std::numeric_limits< double >::min()
) noexcept
```




<hr>




### function unboundedGradientCoordinate {#function-unboundedgradientcoordinate}

_Returns an unbounded palette coordinate for a data-space_ _value_ _._
```C++
inline float QAccelPlot::unboundedGradientCoordinate (
    const GradientDirection direction,
    const float value,
    const float minimum,
    const float maximum
) 
```



Coordinates outside [0, 1] are intentionally preserved for interpolation. Renderers must clamp only after interpolation, immediately before sampling the gradient, so values beyond the configured range retain endpoint colors. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/annotations/DataAnchor.hpp`

