








# Namespace QAccelPlot



[**Namespace List**](namespaces.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md)




















## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**GradientDirectionNS**](namespaceQAccelPlot_1_1GradientDirectionNS.md) <br>_Namespace exposing the_ `GradientDirection` _enum to QML._ |
| namespace | [**GradientFillBaselineNS**](namespaceQAccelPlot_1_1GradientFillBaselineNS.md) <br>_Namespace exposing the_ `GradientFillBaseline` _enum to QML._ |
| namespace | [**GradientValueSourceNS**](namespaceQAccelPlot_1_1GradientValueSourceNS.md) <br>_Namespace exposing the_ `GradientValueSource` _enum to QML._ |
| namespace | [**InspectionNS**](namespaceQAccelPlot_1_1InspectionNS.md) <br>_Namespace exposing the inspection_ `Status` _enum to QML as_`Inspection` _._ |
| namespace | [**InspectionScan**](namespaceQAccelPlot_1_1InspectionScan.md) <br>_Inspection queries that scan every record; used for small series that are not ordered along the queried axis._  |
| namespace | [**Internal**](namespaceQAccelPlot_1_1Internal.md) <br> |
| namespace | [**LineCurveGapFilter**](namespaceQAccelPlot_1_1LineCurveGapFilter.md) <br>_Stateless helpers implementing the invalid-sample contract shared by_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _subsystems._ |
| namespace | [**LineStroke**](namespaceQAccelPlot_1_1LineStroke.md) <br>_Building blocks for the line ribbon drawn by the line shaders, shared by the line renderers._  |
| namespace | [**NanGapModeNS**](namespaceQAccelPlot_1_1NanGapModeNS.md) <br>_Namespace exposing the_ `NanGapMode` _enum to QML._ |


## Classes

| Type | Name |
| ---: | :--- |
| class | [**Axis**](classQAccelPlot_1_1Axis.md) <br>_A visual axis item that maps a data-space range to pixel coordinates and renders tick marks and labels._  |
| struct | [**AxisMapping**](structQAccelPlot_1_1AxisMapping.md) <br>_Snapshot of an axis viewport that maps between data values and pixel positions._  |
| struct | [**AxisTick**](structQAccelPlot_1_1AxisTick.md) <br>_A single major tick: its data-space value and its formatted label._  |
| class | [**AxisTickPainter**](classQAccelPlot_1_1AxisTickPainter.md) <br>_Internal helper that computes and paints tick marks and labels for a single_ [_**Axis**_](classQAccelPlot_1_1Axis.md) _._ |
| class | [**AxisTicker**](classQAccelPlot_1_1AxisTicker.md) <br>_Controls the visual appearance of ticks, sub-ticks, and tick labels on an_ `Axis` _._ |
| struct | [**AxisTicks**](structQAccelPlot_1_1AxisTicks.md) <br>_The visible tick and subtick values, with formatted labels, for one axis viewport._  |
| class | [**BandEdgeMaterial**](classQAccelPlot_1_1BandEdgeMaterial.md) <br>_Line material for the lower or upper edge line of a band, reading_ `(x, low, high)` _samples._ |
| struct | [**BandEdgeRenderParams**](structQAccelPlot_1_1BandEdgeRenderParams.md) <br>_Inputs for_ [_**BandEdgeRenderer::paint()**_](classQAccelPlot_1_1BandEdgeRenderer.md#function-paint) _, assembled while the GUI thread is blocked._ |
| class | [**BandEdgeRenderer**](classQAccelPlot_1_1BandEdgeRenderer.md) <br>_Internal renderer for the lower or upper edge line of a_ `BandSeries` _._ |
| class | [**BandEdges**](classQAccelPlot_1_1BandEdges.md) <br>_Controls the lines a_ `BandSeries` _draws along its lower and upper edges._ |
| class | [**BandMaterial**](classQAccelPlot_1_1BandMaterial.md) <br>_QSGMaterial that fills a band between the low and high values of_ `(x, low, high)` _samples._ |
| struct | [**BandSamples**](structQAccelPlot_1_1BandSamples.md) <br>_Read-only view over interleaved_ `(x, low, high)` _band samples in float or double precision._ |
| class | [**BandSeries**](classQAccelPlot_1_1BandSeries.md) <br>_A hardware-accelerated QML item that fills the area between a low and a high value at each X._  |
| class | [**BarMaterial**](classQAccelPlot_1_1BarMaterial.md) <br>_QSGMaterial for bar series rendering, extending_ [_**RectMaterial**_](classQAccelPlot_1_1RectMaterial.md) _with the bar geometry uniforms._ |
| class | [**BarSeries**](classQAccelPlot_1_1BarSeries.md) <br>_A hardware-accelerated QML item that renders a bar chart._  |
| class | [**ColorBar**](classQAccelPlot_1_1ColorBar.md) <br>_A continuous key that shows how a series'_ `Colormap` _maps values to colors._ |
| class | [**ColorPalette**](classQAccelPlot_1_1ColorPalette.md) <br>_A named set of theme colors shared by QML (via the_ `Colors` _singleton) and C++ defaults._ |
| class | [**Colormap**](classQAccelPlot_1_1Colormap.md) <br>_Maps data values to colors: a color ramp plus the rule that places a value on it._  |
| class | [**Colors**](classQAccelPlot_1_1Colors.md) <br>_QML singleton exposing_ [_**QAccelPlot**_](classQAccelPlot_1_1QAccelPlot.md) _'s built-in color palettes._ |
| struct | [**CurveChunk**](structQAccelPlot_1_1CurveChunk.md) <br>_Axis-aligned bounding box (AABB) for a contiguous block of curve points, used for hit-test culling._  |
| struct | [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) <br>_Read-only view over either interleaved float or double curve coordinates._  |
| struct | [**CurveHitTestParams**](structQAccelPlot_1_1CurveHitTestParams.md) <br>_All inputs required for a_ `contains()` _hit-test, bundled to reduce parameter count._ |
| class | [**DashLine**](classQAccelPlot_1_1DashLine.md) <br>_A line style that renders the curve as a customisable dashed line._  |
| struct | [**DashParameters**](structQAccelPlot_1_1DashParameters.md) <br>_Plain-data snapshot of dash rendering parameters._  |
| class | [**DataAnchor**](classQAccelPlot_1_1DataAnchor.md) <br>_A QQuickItem that tracks a data-coordinate rectangle in pixel space._  |
| class | [**DataTexture**](classQAccelPlot_1_1DataTexture.md) <br>_Series data uploaded to the GPU as an RGBA8888 texture, one float per texel._  |
| class | [**DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md) <br>_Base QSGMaterial that samples series data from a_ `DataTexture` _and exposes shared shader uniforms._ |
| class | [**DataTransition**](classQAccelPlot_1_1DataTransition.md) <br>_Abstract base class for animated data transitions on plot elements._  |
| class | [**DateTimeTickLabelFormatter**](classQAccelPlot_1_1DateTimeTickLabelFormatter.md) <br>_A tick label formatter that displays tick values as formatted date/time strings._  |
| class | [**DrawTransition**](classQAccelPlot_1_1DrawTransition.md) <br>_An animation transition that reveals the target curve by drawing it point-by-point from start to end._  |
| struct | [**FillSamples**](structQAccelPlot_1_1FillSamples.md) <br>_Samples of a gradient fill: one group per valid-sample run, broken at gaps._  |
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
| class | [**Histogram**](classQAccelPlot_1_1Histogram.md) <br>_Samples counted into bins, convertible to_ `BarSeries` _data._ |
| class | [**HistogramFactory**](classQAccelPlot_1_1HistogramFactory.md) <br>_QML singleton_ `Histogram` _that creates_[_**Histogram**_](classQAccelPlot_1_1Histogram.md) _values from JavaScript arrays._ |
| struct | [**InspectionBounds**](structQAccelPlot_1_1InspectionBounds.md) <br>_Inclusive data-space region; infinite limits leave a dimension unbounded._  |
| struct | [**InspectionBracket**](structQAccelPlot_1_1InspectionBracket.md) <br>_The valid samples on either side of a position on one axis._  |
| class | [**InspectionCache**](classQAccelPlot_1_1InspectionCache.md) <br>_Builds an_ `InspectionIndex` _on a worker thread from a snapshot of a series' records._ |
| struct | [**InspectionHit**](structQAccelPlot_1_1InspectionHit.md) <br>_Nearest-sample result: source index and pixel distance, or index -1._  |
| class | [**InspectionIndex**](classQAccelPlot_1_1InspectionIndex.md) <br>_Immutable k-d tree over the valid samples of a series, with their order along each axis._  |
| struct | [**InspectionMetric**](structQAccelPlot_1_1InspectionMetric.md) <br>_Pixel mapping of a series' plot area, used to measure on-screen distances._  |
| struct | [**InspectionNeighbors**](structQAccelPlot_1_1InspectionNeighbors.md) <br>_Source indices of the valid samples on either side of an X value, or -1._  |
| struct | [**InspectionPage**](structQAccelPlot_1_1InspectionPage.md) <br>_One page of source indices inside a region._  |
| struct | [**InspectionRecord**](structQAccelPlot_1_1InspectionRecord.md) <br>_A native record of a series that is not a plain XY series, such as a bar, rectangle, or band._  |
| struct | [**InspectionRow**](structQAccelPlot_1_1InspectionRow.md) <br>_Inspection result of one series, as shown by one row of an_ `InspectionRowModel` _._ |
| class | [**InspectionRowModel**](classQAccelPlot_1_1InspectionRowModel.md) <br>_List model with one stable row per inspected series._  |
| struct | [**InspectionSample**](structQAccelPlot_1_1InspectionSample.md) <br>_One XY source sample returned by an inspection query._  |
| struct | [**InspectionSource**](structQAccelPlot_1_1InspectionSource.md) <br>_Read-only view over the XY records a series exposes to inspection queries._  |
| struct | [**InspectionSummary**](structQAccelPlot_1_1InspectionSummary.md) <br>_Sample-weighted Y statistics over the valid samples inside a region._  |
| class | [**LineCurve**](classQAccelPlot_1_1LineCurve.md) <br>_A hardware-accelerated QML item that renders a 2D line curve with optional markers, dashing, and gradient effects._  |
| class | [**LineCurveEffect**](classQAccelPlot_1_1LineCurveEffect.md) <br>_Abstract base class for visual effects applied to a_ `LineCurve` _._ |
| class | [**LineCurveGaps**](classQAccelPlot_1_1LineCurveGaps.md) <br>_Controls how a_ `LineCurve` _renders gaps in its data._ |
| class | [**LineCurveLineRenderer**](classQAccelPlot_1_1LineCurveLineRenderer.md) <br>_Internal renderer responsible for building and updating QSGNode line geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._ |
| class | [**LineCurvePointRenderer**](classQAccelPlot_1_1LineCurvePointRenderer.md) <br>_Internal renderer responsible for building and updating QSGNode marker geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._ |
| struct | [**LineCurveRenderParams**](structQAccelPlot_1_1LineCurveRenderParams.md) <br>_Input parameters for_ [_**LineCurveLineRenderer::paint()**_](classQAccelPlot_1_1LineCurveLineRenderer.md#function-paint) _, assembled on the main thread._ |
| class | [**LineCurveVertexCache**](classQAccelPlot_1_1LineCurveVertexCache.md) <br>_Owns a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _'s pre-built vertex bytes and the metadata required to use them safely._ |
| class | [**LineMaterial**](classQAccelPlot_1_1LineMaterial.md) <br>_QSGMaterial for line rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with line-specific uniforms._ |
| class | [**LineStyle**](classQAccelPlot_1_1LineStyle.md) <br>_Abstract base class for all line styles._  |
| struct | [**LineVertex**](structQAccelPlot_1_1LineVertex.md) <br>_Vertex layout for line geometry, shared with the main thread for pre-built vertex caches._  |
| class | [**MorphTransition**](classQAccelPlot_1_1MorphTransition.md) <br>_An animation transition that smoothly interpolates point positions between two datasets._  |
| class | [**NoLine**](classQAccelPlot_1_1NoLine.md) <br>_A line style that suppresses line rendering entirely, leaving only markers visible._  |
| class | [**NumericTickLabelFormatter**](classQAccelPlot_1_1NumericTickLabelFormatter.md) <br>_The default tick label formatter — produces numeric labels with automatic decimal precision._  |
| class | [**OutlinedRectangle**](classQAccelPlot_1_1OutlinedRectangle.md) <br>_Item that fills its bounds and outlines them with a line of one logical pixel._  |
| class | [**OverlayChildren**](classQAccelPlot_1_1OverlayChildren.md) <br>_Objects declared inside an inspection tool in QML._  |
| class | [**PlotBorder**](classQAccelPlot_1_1PlotBorder.md) <br>_Decorative frame configuration exposed by_ `PlotView::border` _._ |
| class | [**PlotDragRect**](classQAccelPlot_1_1PlotDragRect.md) <br>_Rectangle dragged out inside the plot area, shared by rectangle zoom and data selection._  |
| class | [**PlotInspector**](classQAccelPlot_1_1PlotInspector.md) <br>_Inspects every visible XY series of a plot at a cursor and publishes one model row per series._  |
| class | [**PlotMouseEvent**](classQAccelPlot_1_1PlotMouseEvent.md) <br>_Carries mouse event data for the mouse signals._  |
| class | [**PlotRectangleZoom**](classQAccelPlot_1_1PlotRectangleZoom.md) <br>_Rectangle zoom configuration and selection state exposed by PlotView._  |
| class | [**PlotSeries**](classQAccelPlot_1_1PlotSeries.md) <br>_Common QML item contract for data series hosted by_ `PlotView` _._ |
| class | [**PointCloud**](classQAccelPlot_1_1PointCloud.md) <br>_A hardware-accelerated QML item that renders large sets of unconnected 2D points as markers._  |
| class | [**PointCloudMaterial**](classQAccelPlot_1_1PointCloudMaterial.md) <br>_QSGMaterial for_ `PointCloud` _rendering._ |
| struct | [**PointCurveRenderParams**](structQAccelPlot_1_1PointCurveRenderParams.md) <br>_Input parameters for_ [_**LineCurvePointRenderer::paint()**_](classQAccelPlot_1_1LineCurvePointRenderer.md#function-paint) _, assembled on the main thread._ |
| class | [**PointMaterial**](classQAccelPlot_1_1PointMaterial.md) <br>_QSGMaterial for marker (point) rendering._  |
| class | [**PointSpatialIndex**](classQAccelPlot_1_1PointSpatialIndex.md) <br>_Uniform-grid spatial index for nearest-point queries over large point sets._  |
| struct | [**PointVertex**](structQAccelPlot_1_1PointVertex.md) <br>_Vertex layout for point (marker) geometry, shared with the main thread for vertex caches._  |
| class | [**QAccelPlot**](classQAccelPlot_1_1QAccelPlot.md) <br>_The main plot canvas QML item — hosts axes, curves, and a grid._  |
| class | [**RectMaterial**](classQAccelPlot_1_1RectMaterial.md) <br>_QSGMaterial for rectangle series rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with a rect-count uniform._ |
| class | [**RectVertexCache**](classQAccelPlot_1_1RectVertexCache.md) <br>_Vertices of a series that draws each rectangle as a quad positioned from a data texture._  |
| class | [**RectangleBorder**](classQAccelPlot_1_1RectangleBorder.md) <br>_Controls the outline a_ `RectangleSeries` _draws inside each rectangle's edges._ |
| class | [**RectangleSeries**](classQAccelPlot_1_1RectangleSeries.md) <br>_A hardware-accelerated QML item that renders a large list of axis-aligned rectangles._  |
| class | [**RectangleZoomOverlay**](classQAccelPlot_1_1RectangleZoomOverlay.md) <br> |
| struct | [**SampleRun**](structQAccelPlot_1_1SampleRun.md) <br>_Contiguous range of valid curve samples, used to break fills and hit tests at gaps._  |
| class | [**SelectionRectangle**](classQAccelPlot_1_1SelectionRectangle.md) <br>_Draws the gesture or the selected region of a_ [_**SelectionTool**_](classQAccelPlot_1_1SelectionTool.md) _, clipped to the plot area._ |
| class | [**SelectionTool**](classQAccelPlot_1_1SelectionTool.md) <br>_Selects a data-space region by dragging, without changing the plot viewport._  |
| class | [**SeriesInspection**](classQAccelPlot_1_1SeriesInspection.md) <br>_Data queries for one series: nearest samples, brackets, region statistics, and index pages._  |
| class | [**SeriesMarker**](classQAccelPlot_1_1SeriesMarker.md) <br>_Controls the markers a series draws at its data points._  |
| class | [**SolidLine**](classQAccelPlot_1_1SolidLine.md) <br>_The default line style — renders a continuous solid line with no gaps._  |
| class | [**SourceInspection**](classQAccelPlot_1_1SourceInspection.md) <br>_Inspection queries that read a series' own buffer whose records are ordered along one axis._  |
| class | [**SpatialGrid**](classQAccelPlot_1_1SpatialGrid.md) <br>_Uniform-grid spatial index with bounded per-rectangle storage._  |
| struct | [**SummaryAccumulator**](structQAccelPlot_1_1SummaryAccumulator.md) <br>_Running sample-weighted Y statistics that can be merged across disjoint sample sets._  |
| class | [**TextTickLabelFormatter**](classQAccelPlot_1_1TextTickLabelFormatter.md) <br>_A tick label formatter that maps integer tick indices to a user-supplied list of strings._  |
| class | [**TickLabelFormatter**](classQAccelPlot_1_1TickLabelFormatter.md) <br>_Abstract base class for tick label formatters._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef [**GradientDirectionNS::Direction**](namespaceQAccelPlot_1_1GradientDirectionNS.md#enum-direction) | [**GradientDirection**](#typedef-gradientdirection)  <br> |
| typedef [**GradientFillBaselineNS::Mode**](namespaceQAccelPlot_1_1GradientFillBaselineNS.md#enum-mode) | [**GradientFillBaseline**](#typedef-gradientfillbaseline)  <br> |
| typedef [**GradientValueSourceNS::Source**](namespaceQAccelPlot_1_1GradientValueSourceNS.md#enum-source) | [**GradientValueSource**](#typedef-gradientvaluesource)  <br> |
| enum  | [**InspectionAxis**](#enum-inspectionaxis)  <br>_Data dimension a query measures along, or along which a series' records are ordered._  |
| typedef [**InspectionNS::Status**](namespaceQAccelPlot_1_1InspectionNS.md#enum-status) | [**InspectionStatus**](#typedef-inspectionstatus)  <br> |
| typedef [**NanGapModeNS::Mode**](namespaceQAccelPlot_1_1NanGapModeNS.md#enum-mode) | [**NanGapMode**](#typedef-nangapmode)  <br> |




## Public Attributes

| Type | Name |
| ---: | :--- |
|  constexpr int | [**kMaxDashPatternSize**](#variable-kmaxdashpatternsize)   = `8`<br>_Largest dash pattern the renderer can carry to the shader._  |
|  constexpr double | [**kNearlyEqualEpsilon**](#variable-knearlyequalepsilon)   = `2.0 \* std::numeric\_limits&lt;double&gt;::epsilon()`<br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  double | [**distanceToInterval**](#function-distancetointerval) (double position, double a, double b) noexcept<br>_Returns the signed distance from_ _position_ _to the closed interval between__a_ _and__b_ _._ |
|  bool | [**isEmptyChunk**](#function-isemptychunk) (const [**CurveChunk**](structQAccelPlot_1_1CurveChunk.md) & chunk) <br>_Returns_ `true` _when__chunk_ _contains no valid sample and can be skipped by hit tests._ |
|  bool | [**isValidSample**](#function-isvalidsample) (double value, bool logScale) noexcept<br> |
|  bool | [**nearly\_equal**](#function-nearly_equal) (double a, double b, double eps\_rel=kNearlyEqualEpsilon, double eps\_abs=0.0) noexcept<br> |
|  std::vector&lt; [**GradientStopData**](structQAccelPlot_1_1GradientStopData.md) &gt; | [**readEffectStops**](#function-readeffectstops) (const [**Colormap**](classQAccelPlot_1_1Colormap.md) \* colormap, QObject \* gradient) <br>_Returns the color stops of a gradient effect: the_ _colormap_ _ramp when set, otherwise the stops of__gradient_ _._ |
|  std::vector&lt; [**GradientStopData**](structQAccelPlot_1_1GradientStopData.md) &gt; | [**readGradientStopList**](#function-readgradientstoplist) (const QVariantList & stopObjects) <br>_Reads a list of stop objects, each exposing_ `position` _and_`color` _, into position order._ |
|  std::vector&lt; [**GradientStopData**](structQAccelPlot_1_1GradientStopData.md) &gt; | [**readGradientStops**](#function-readgradientstops) (QObject \* gradient) <br>_Reads the stops of a QML_ `Gradient` _into position order, covering the full [0, 1] range._ |
|  double | [**renderOriginForViewport**](#function-renderoriginforviewport) (double origin, double viewportMin, double viewportMax) noexcept<br> |
|  bool | [**renderOriginTooFar**](#function-renderorigintoofar) (double origin, double viewportMin, double viewportMax) noexcept<br> |
|  void | [**resolveGradientValueRange**](#function-resolvegradientvaluerange) (Payload & payload, const qreal dataMin, const qreal dataMax) <br>_Fills each unset gradient value bound of_ _payload_ _from [__dataMin_ _,__dataMax_ _]._ |
|  float | [**unboundedGradientCoordinate**](#function-unboundedgradientcoordinate) (const [**GradientDirection**](namespaceQAccelPlot_1_1GradientDirectionNS.md#enum-direction) direction, const qreal value, const qreal minimum, const qreal maximum) <br>_Returns an unbounded palette coordinate for a data-space_ _value_ _._ |




























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




### enum InspectionAxis {#enum-inspectionaxis}

_Data dimension a query measures along, or along which a series' records are ordered._ 
```C++
enum QAccelPlot::InspectionAxis {
    X,
    Y
};
```




<hr>




### typedef InspectionStatus {#typedef-inspectionstatus}

```C++
using QAccelPlot::InspectionStatus = typedef InspectionNS::Status;
```




<hr>




### typedef NanGapMode {#typedef-nangapmode}

```C++
using QAccelPlot::NanGapMode = typedef NanGapModeNS::Mode;
```




<hr>
## Public Attributes Documentation





### variable kMaxDashPatternSize {#variable-kmaxdashpatternsize}

_Largest dash pattern the renderer can carry to the shader._ 
```C++
constexpr int QAccelPlot::kMaxDashPatternSize;
```




<hr>




### variable kNearlyEqualEpsilon {#variable-knearlyequalepsilon}

```C++
constexpr double QAccelPlot::kNearlyEqualEpsilon;
```




<hr>
## Public Functions Documentation





### function distanceToInterval {#function-distancetointerval}

_Returns the signed distance from_ _position_ _to the closed interval between__a_ _and__b_ _._
```C++
double QAccelPlot::distanceToInterval (
    double position,
    double a,
    double b
) noexcept
```




<hr>




### function isEmptyChunk {#function-isemptychunk}

_Returns_ `true` _when__chunk_ _contains no valid sample and can be skipped by hit tests._
```C++
inline bool QAccelPlot::isEmptyChunk (
    const CurveChunk & chunk
) 
```




<hr>




### function isValidSample {#function-isvalidsample}

```C++
inline bool QAccelPlot::isValidSample (
    double value,
    bool logScale
) noexcept
```




<hr>




### function nearly\_equal {#function-nearly_equal}

```C++
inline bool QAccelPlot::nearly_equal (
    double a,
    double b,
    double eps_rel=kNearlyEqualEpsilon,
    double eps_abs=0.0
) noexcept
```




<hr>




### function readEffectStops {#function-readeffectstops}

_Returns the color stops of a gradient effect: the_ _colormap_ _ramp when set, otherwise the stops of__gradient_ _._
```C++
std::vector< GradientStopData > QAccelPlot::readEffectStops (
    const Colormap * colormap,
    QObject * gradient
) 
```



Returns an empty vector when both are null.




**See also:** [**GradientFill**](classQAccelPlot_1_1GradientFill.md), [**GradientStroke**](classQAccelPlot_1_1GradientStroke.md) 



        

<hr>




### function readGradientStopList {#function-readgradientstoplist}

_Reads a list of stop objects, each exposing_ `position` _and_`color` _, into position order._
```C++
std::vector< GradientStopData > QAccelPlot::readGradientStopList (
    const QVariantList & stopObjects
) 
```



Normalized the same way as `readGradientStops()`: a single stop is duplicated and the end colors are extended to 0 and 1. Entries that are not stop-like are skipped.




**See also:** [**Colormap**](classQAccelPlot_1_1Colormap.md) 



        

<hr>




### function readGradientStops {#function-readgradientstops}

_Reads the stops of a QML_ `Gradient` _into position order, covering the full [0, 1] range._
```C++
std::vector< GradientStopData > QAccelPlot::readGradientStops (
    QObject * gradient
) 
```



Stops are read from the `stops` list property, falling back to child objects. A single stop is duplicated, and the first and last colors are extended to positions 0 and 1. Returns an empty vector when _gradient_ is null or has no valid stops.




**See also:** [**GradientFill**](classQAccelPlot_1_1GradientFill.md), [**GradientStroke**](classQAccelPlot_1_1GradientStroke.md), [**PointCloud**](classQAccelPlot_1_1PointCloud.md) 



        

<hr>




### function renderOriginForViewport {#function-renderoriginforviewport}

```C++
inline double QAccelPlot::renderOriginForViewport (
    double origin,
    double viewportMin,
    double viewportMax
) noexcept
```




<hr>




### function renderOriginTooFar {#function-renderorigintoofar}

```C++
inline bool QAccelPlot::renderOriginTooFar (
    double origin,
    double viewportMin,
    double viewportMax
) noexcept
```




<hr>




### function resolveGradientValueRange {#function-resolvegradientvaluerange}

_Fills each unset gradient value bound of_ _payload_ _from [__dataMin_ _,__dataMax_ _]._
```C++
template<typename Payload>
void QAccelPlot::resolveGradientValueRange (
    Payload & payload,
    const qreal dataMin,
    const qreal dataMax
) 
```



Bounds resolve independently, so a `Fixed` bound is kept when the other bound uses `DataRange`. 


        

<hr>




### function unboundedGradientCoordinate {#function-unboundedgradientcoordinate}

_Returns an unbounded palette coordinate for a data-space_ _value_ _._
```C++
inline float QAccelPlot::unboundedGradientCoordinate (
    const GradientDirection direction,
    const qreal value,
    const qreal minimum,
    const qreal maximum
) 
```



Coordinates outside [0, 1] are intentionally preserved for interpolation. Renderers must clamp only after interpolation, immediately before sampling the gradient, so values beyond the configured range retain endpoint colors. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/annotations/DataAnchor.hpp`

