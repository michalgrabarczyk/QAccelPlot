
# Class List


Here are the classes, structs, unions and interfaces with brief descriptions:

* **namespace** [**QAccelPlot**](namespaceQAccelPlot.md)     
    * **class** [**Axis**](classQAccelPlot_1_1Axis.md) _A visual axis item that maps a data-space range to pixel coordinates and renders tick marks and labels._     
    * **struct** [**AxisMapping**](structQAccelPlot_1_1AxisMapping.md) _Snapshot of an axis viewport that maps between data values and pixel positions._     
    * **struct** [**AxisTick**](structQAccelPlot_1_1AxisTick.md) _A single major tick: its data-space value and its formatted label._     
    * **class** [**AxisTickPainter**](classQAccelPlot_1_1AxisTickPainter.md) _Internal helper that computes and paints tick marks and labels for a single_ [_**Axis**_](classQAccelPlot_1_1Axis.md) _._    
        * **struct** [**PaintContext**](structQAccelPlot_1_1AxisTickPainter_1_1PaintContext.md) _Groups the QPainter context arguments passed into sub-painting helpers._     
        * **struct** [**Params**](structQAccelPlot_1_1AxisTickPainter_1_1Params.md) _All style inputs required for a single paint call, bundled to reduce parameter count._     
    * **class** [**AxisTicker**](classQAccelPlot_1_1AxisTicker.md) _Controls the visual appearance of ticks, sub-ticks, and tick labels on an_ `Axis` _._    
    * **struct** [**AxisTicks**](structQAccelPlot_1_1AxisTicks.md) _The visible tick and subtick values, with formatted labels, for one axis viewport._     
    * **class** [**BandEdgeMaterial**](classQAccelPlot_1_1BandEdgeMaterial.md) _Line material for the lower or upper edge line of a band, reading_ `(x, low, high)` _samples._    
    * **struct** [**BandEdgeRenderParams**](structQAccelPlot_1_1BandEdgeRenderParams.md) _Inputs for_ [_**BandEdgeRenderer::paint()**_](classQAccelPlot_1_1BandEdgeRenderer.md#function-paint) _, assembled while the GUI thread is blocked._    
    * **class** [**BandEdgeRenderer**](classQAccelPlot_1_1BandEdgeRenderer.md) _Internal renderer for the lower or upper edge line of a_ `BandSeries` _._    
    * **class** [**BandEdges**](classQAccelPlot_1_1BandEdges.md) _Controls the lines a_ `BandSeries` _draws along its lower and upper edges._    
    * **class** [**BandMaterial**](classQAccelPlot_1_1BandMaterial.md) _QSGMaterial that fills a band between the low and high values of_ `(x, low, high)` _samples._    
        * **struct** [**Vertex**](structQAccelPlot_1_1BandMaterial_1_1Vertex.md) [_**Vertex**_](structQAccelPlot_1_1BandMaterial_1_1Vertex.md) _layout of the band triangle strip: two vertices per sample._    
    * **struct** [**BandSamples**](structQAccelPlot_1_1BandSamples.md) _Read-only view over interleaved_ `(x, low, high)` _band samples in float or double precision._    
    * **class** [**BandSeries**](classQAccelPlot_1_1BandSeries.md) _A hardware-accelerated QML item that fills the area between a low and a high value at each X._     
    * **class** [**BarMaterial**](classQAccelPlot_1_1BarMaterial.md) _QSGMaterial for bar series rendering, extending_ [_**RectMaterial**_](classQAccelPlot_1_1RectMaterial.md) _with the bar geometry uniforms._    
    * **class** [**BarSeries**](classQAccelPlot_1_1BarSeries.md) _A hardware-accelerated QML item that renders a bar chart._     
    * **class** [**ColorBar**](classQAccelPlot_1_1ColorBar.md) _A continuous key that shows how a series'_ `Colormap` _maps values to colors._    
    * **class** [**ColorPalette**](classQAccelPlot_1_1ColorPalette.md) _A named set of theme colors shared by QML (via the_ `Colors` _singleton) and C++ defaults._    
    * **class** [**Colormap**](classQAccelPlot_1_1Colormap.md) _Maps data values to colors: a color ramp plus the rule that places a value on it._     
    * **class** [**Colors**](classQAccelPlot_1_1Colors.md) _QML singleton exposing_ [_**QAccelPlot**_](classQAccelPlot_1_1QAccelPlot.md) _'s built-in color palettes._    
    * **struct** [**CurveChunk**](structQAccelPlot_1_1CurveChunk.md) _Axis-aligned bounding box (AABB) for a contiguous block of curve points, used for hit-test culling._     
    * **struct** [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) _Read-only view over either interleaved float or double curve coordinates._     
    * **struct** [**CurveHitTestParams**](structQAccelPlot_1_1CurveHitTestParams.md) _All inputs required for a_ `contains()` _hit-test, bundled to reduce parameter count._    
    * **class** [**DashLine**](classQAccelPlot_1_1DashLine.md) _A line style that renders the curve as a customisable dashed line._     
    * **struct** [**DashParameters**](structQAccelPlot_1_1DashParameters.md) _Plain-data snapshot of dash rendering parameters._     
    * **class** [**DataAnchor**](classQAccelPlot_1_1DataAnchor.md) _A QQuickItem that tracks a data-coordinate rectangle in pixel space._     
    * **class** [**DataTexture**](classQAccelPlot_1_1DataTexture.md) _Series data uploaded to the GPU as an RGBA8888 texture, one float per texel._     
    * **class** [**DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md) _Base QSGMaterial that samples series data from a_ `DataTexture` _and exposes shared shader uniforms._    
    * **class** [**DataTransition**](classQAccelPlot_1_1DataTransition.md) _Abstract base class for animated data transitions on plot elements._     
        * **class** [**Run**](classQAccelPlot_1_1DataTransition_1_1Run.md) _One animation of a transition on one host element._     
    * **class** [**DateTimeTickLabelFormatter**](classQAccelPlot_1_1DateTimeTickLabelFormatter.md) _A tick label formatter that displays tick values as formatted date/time strings._     
    * **class** [**DrawTransition**](classQAccelPlot_1_1DrawTransition.md) _An animation transition that reveals the target curve by drawing it point-by-point from start to end._     
    * **struct** [**FillSamples**](structQAccelPlot_1_1FillSamples.md) _Samples of a gradient fill: one group per valid-sample run, broken at gaps._     
    * **struct** [**GradientColorPayload**](structQAccelPlot_1_1GradientColorPayload.md) _Render-thread snapshot of gradient stroke (line-color) parameters._     
    * **namespace** [**GradientDirectionNS**](namespaceQAccelPlot_1_1GradientDirectionNS.md) _Namespace exposing the_ `GradientDirection` _enum to QML._    
    * **class** [**GradientFill**](classQAccelPlot_1_1GradientFill.md) _A_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _effect that fills the area under the curve with a color gradient._    
    * **namespace** [**GradientFillBaselineNS**](namespaceQAccelPlot_1_1GradientFillBaselineNS.md) _Namespace exposing the_ `GradientFillBaseline` _enum to QML._    
    * **class** [**GradientFillMaterial**](classQAccelPlot_1_1GradientFillMaterial.md) _Scene-graph material that evaluates fill gradients per fragment._     
    * **struct** [**GradientFillPayload**](structQAccelPlot_1_1GradientFillPayload.md) _Render-thread snapshot of gradient fill (area-under-curve) parameters._     
    * **class** [**GradientLineMaterial**](classQAccelPlot_1_1GradientLineMaterial.md) _Line material variant that samples a one-dimensional gradient texture._     
    * **struct** [**GradientStopData**](structQAccelPlot_1_1GradientStopData.md) _A single color stop within a gradient definition._     
    * **class** [**GradientStroke**](classQAccelPlot_1_1GradientStroke.md) _A_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _effect that replaces the solid line color with a color gradient._    
    * **class** [**GradientTexture**](classQAccelPlot_1_1GradientTexture.md) _Cached one-dimensional texture used by gradient materials._     
    * **namespace** [**GradientValueSourceNS**](namespaceQAccelPlot_1_1GradientValueSourceNS.md) _Namespace exposing the_ `GradientValueSource` _enum to QML._    
    * **class** [**Grid**](classQAccelPlot_1_1Grid.md) _Configuration object that controls the appearance of the plot grid._     
    * **class** [**GridNode**](classQAccelPlot_1_1GridNode.md) _Internal QSGNode responsible for rendering the plot grid into the scene graph._     
    * **struct** [**InspectionBounds**](structQAccelPlot_1_1InspectionBounds.md) _Inclusive data-space region; infinite limits leave a dimension unbounded._     
    * **struct** [**InspectionBracket**](structQAccelPlot_1_1InspectionBracket.md) _The valid samples on either side of a position on one axis._     
    * **class** [**InspectionCache**](classQAccelPlot_1_1InspectionCache.md) _Builds an_ `InspectionIndex` _on a worker thread from a snapshot of a series' records._    
        * **class** [**Host**](classQAccelPlot_1_1InspectionCache_1_1Host.md) _Series-side callbacks; all are invoked on the series' thread._     
    * **struct** [**InspectionHit**](structQAccelPlot_1_1InspectionHit.md) _Nearest-sample result: source index and pixel distance, or index -1._     
    * **class** [**InspectionIndex**](classQAccelPlot_1_1InspectionIndex.md) _Immutable k-d tree over the valid samples of a series, with their order along each axis._     
        * **struct** [**Point**](structQAccelPlot_1_1InspectionIndex_1_1Point.md)     
    * **struct** [**InspectionMetric**](structQAccelPlot_1_1InspectionMetric.md) _Pixel mapping of a series' plot area, used to measure on-screen distances._     
    * **namespace** [**InspectionNS**](namespaceQAccelPlot_1_1InspectionNS.md) _Namespace exposing the inspection_ `Status` _enum to QML as_`Inspection` _._    
    * **struct** [**InspectionNeighbors**](structQAccelPlot_1_1InspectionNeighbors.md) _Source indices of the valid samples on either side of an X value, or -1._     
    * **struct** [**InspectionPage**](structQAccelPlot_1_1InspectionPage.md) _One page of source indices inside a region._     
    * **struct** [**InspectionRecord**](structQAccelPlot_1_1InspectionRecord.md) _A native record of a series that is not a plain XY series, such as a bar, rectangle, or band._     
    * **struct** [**InspectionRow**](structQAccelPlot_1_1InspectionRow.md) _Inspection result of one series, as shown by one row of an_ `InspectionRowModel` _._    
    * **class** [**InspectionRowModel**](classQAccelPlot_1_1InspectionRowModel.md) _List model with one stable row per inspected series._     
    * **struct** [**InspectionSample**](structQAccelPlot_1_1InspectionSample.md) _One XY source sample returned by an inspection query._     
    * **namespace** [**InspectionScan**](namespaceQAccelPlot_1_1InspectionScan.md) _Inspection queries that scan every record; used for small series that are not ordered along the queried axis._     
    * **struct** [**InspectionSource**](structQAccelPlot_1_1InspectionSource.md) _Read-only view over the XY records a series exposes to inspection queries._     
    * **struct** [**InspectionSummary**](structQAccelPlot_1_1InspectionSummary.md) _Sample-weighted Y statistics over the valid samples inside a region._     
    * **namespace** [**Internal**](namespaceQAccelPlot_1_1Internal.md)     
        * **struct** [**RectUbo**](structQAccelPlot_1_1Internal_1_1RectUbo.md) _Mirrors the std140 uniform block of rect.vert._     
    * **class** [**LineCurve**](classQAccelPlot_1_1LineCurve.md) _A hardware-accelerated QML item that renders a 2D line curve with optional markers, dashing, and gradient effects._     
    * **class** [**LineCurveEffect**](classQAccelPlot_1_1LineCurveEffect.md) _Abstract base class for visual effects applied to a_ `LineCurve` _._    
    * **namespace** [**LineCurveGapFilter**](namespaceQAccelPlot_1_1LineCurveGapFilter.md) _Stateless helpers implementing the invalid-sample contract shared by_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _subsystems._    
    * **class** [**LineCurveGaps**](classQAccelPlot_1_1LineCurveGaps.md) _Controls how a_ `LineCurve` _renders gaps in its data._    
    * **class** [**LineCurveLineRenderer**](classQAccelPlot_1_1LineCurveLineRenderer.md) _Internal renderer responsible for building and updating QSGNode line geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._    
    * **class** [**LineCurvePointRenderer**](classQAccelPlot_1_1LineCurvePointRenderer.md) _Internal renderer responsible for building and updating QSGNode marker geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._    
    * **struct** [**LineCurveRenderParams**](structQAccelPlot_1_1LineCurveRenderParams.md) _Input parameters for_ [_**LineCurveLineRenderer::paint()**_](classQAccelPlot_1_1LineCurveLineRenderer.md#function-paint) _, assembled on the main thread._    
    * **class** [**LineCurveVertexCache**](classQAccelPlot_1_1LineCurveVertexCache.md) _Owns a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _'s pre-built vertex bytes and the metadata required to use them safely._    
    * **class** [**LineMaterial**](classQAccelPlot_1_1LineMaterial.md) _QSGMaterial for line rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with line-specific uniforms._    
    * **namespace** [**LineStroke**](namespaceQAccelPlot_1_1LineStroke.md) _Building blocks for the line ribbon drawn by the line shaders, shared by the line renderers._     
        * **struct** [**Uniforms**](structQAccelPlot_1_1LineStroke_1_1Uniforms.md) [_**Uniforms**_](structQAccelPlot_1_1LineStroke_1_1Uniforms.md) _of a line material that do not depend on its shader variant._    
    * **class** [**LineStyle**](classQAccelPlot_1_1LineStyle.md) _Abstract base class for all line styles._     
    * **struct** [**LineVertex**](structQAccelPlot_1_1LineVertex.md) _Vertex layout for line geometry, shared with the main thread for pre-built vertex caches._     
    * **class** [**MorphTransition**](classQAccelPlot_1_1MorphTransition.md) _An animation transition that smoothly interpolates point positions between two datasets._     
    * **namespace** [**NanGapModeNS**](namespaceQAccelPlot_1_1NanGapModeNS.md) _Namespace exposing the_ `NanGapMode` _enum to QML._    
    * **class** [**NoLine**](classQAccelPlot_1_1NoLine.md) _A line style that suppresses line rendering entirely, leaving only markers visible._     
    * **class** [**NumericTickLabelFormatter**](classQAccelPlot_1_1NumericTickLabelFormatter.md) _The default tick label formatter — produces numeric labels with automatic decimal precision._     
    * **class** [**OutlinedRectangle**](classQAccelPlot_1_1OutlinedRectangle.md) _Item that fills its bounds and outlines them with a line of one logical pixel._     
    * **class** [**OverlayChildren**](classQAccelPlot_1_1OverlayChildren.md) _Objects declared inside an inspection tool in QML._     
    * **class** [**PlotBorder**](classQAccelPlot_1_1PlotBorder.md) _Decorative frame configuration exposed by_ `PlotView::border` _._    
    * **class** [**PlotDragRect**](classQAccelPlot_1_1PlotDragRect.md) _Rectangle dragged out inside the plot area, shared by rectangle zoom and data selection._     
    * **class** [**PlotInspector**](classQAccelPlot_1_1PlotInspector.md) _Inspects every visible XY series of a plot at a cursor and publishes one model row per series._     
    * **class** [**PlotMouseEvent**](classQAccelPlot_1_1PlotMouseEvent.md) _Carries mouse event data for the mouse signals._     
    * **class** [**PlotRectangleZoom**](classQAccelPlot_1_1PlotRectangleZoom.md) _Rectangle zoom configuration and selection state exposed by PlotView._     
    * **class** [**PlotSeries**](classQAccelPlot_1_1PlotSeries.md) _Common QML item contract for data series hosted by_ `PlotView` _._    
        * **struct** [**DataExtent**](structQAccelPlot_1_1PlotSeries_1_1DataExtent.md) _Extent of the valid coordinates in one dimension._     
    * **class** [**PointCloud**](classQAccelPlot_1_1PointCloud.md) _A hardware-accelerated QML item that renders large sets of unconnected 2D points as markers._     
    * **class** [**PointCloudMaterial**](classQAccelPlot_1_1PointCloudMaterial.md) _QSGMaterial for_ `PointCloud` _rendering._    
    * **struct** [**PointCurveRenderParams**](structQAccelPlot_1_1PointCurveRenderParams.md) _Input parameters for_ [_**LineCurvePointRenderer::paint()**_](classQAccelPlot_1_1LineCurvePointRenderer.md#function-paint) _, assembled on the main thread._    
    * **class** [**PointMaterial**](classQAccelPlot_1_1PointMaterial.md) _QSGMaterial for marker (point) rendering._     
    * **class** [**PointSpatialIndex**](classQAccelPlot_1_1PointSpatialIndex.md) _Uniform-grid spatial index for nearest-point queries over large point sets._     
        * **struct** [**Mapping**](structQAccelPlot_1_1PointSpatialIndex_1_1Mapping.md) _Coordinate mapping applied before indexing._     
    * **struct** [**PointVertex**](structQAccelPlot_1_1PointVertex.md) _Vertex layout for point (marker) geometry, shared with the main thread for vertex caches._     
    * **class** [**QAccelPlot**](classQAccelPlot_1_1QAccelPlot.md) _The main plot canvas QML item — hosts axes, curves, and a grid._     
    * **class** [**RectMaterial**](classQAccelPlot_1_1RectMaterial.md) _QSGMaterial for rectangle series rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with a rect-count uniform._    
    * **class** [**RectVertexCache**](classQAccelPlot_1_1RectVertexCache.md) _Vertices of a series that draws each rectangle as a quad positioned from a data texture._     
    * **class** [**RectangleBorder**](classQAccelPlot_1_1RectangleBorder.md) _Controls the outline a_ `RectangleSeries` _draws inside each rectangle's edges._    
    * **class** [**RectangleSeries**](classQAccelPlot_1_1RectangleSeries.md) _A hardware-accelerated QML item that renders a large list of axis-aligned rectangles._     
    * **class** [**RectangleZoomOverlay**](classQAccelPlot_1_1RectangleZoomOverlay.md)     
    * **struct** [**SampleRun**](structQAccelPlot_1_1SampleRun.md) _Contiguous range of valid curve samples, used to break fills and hit tests at gaps._     
    * **class** [**SelectionRectangle**](classQAccelPlot_1_1SelectionRectangle.md) _Draws the gesture or the selected region of a_ [_**SelectionTool**_](classQAccelPlot_1_1SelectionTool.md) _, clipped to the plot area._    
    * **class** [**SelectionTool**](classQAccelPlot_1_1SelectionTool.md) _Selects a data-space region by dragging, without changing the plot viewport._     
    * **class** [**SeriesInspection**](classQAccelPlot_1_1SeriesInspection.md) _Data queries for one series: nearest samples, brackets, region statistics, and index pages._     
    * **class** [**SeriesMarker**](classQAccelPlot_1_1SeriesMarker.md) _Controls the markers a series draws at its data points._     
    * **class** [**SolidLine**](classQAccelPlot_1_1SolidLine.md) _The default line style — renders a continuous solid line with no gaps._     
    * **class** [**SourceInspection**](classQAccelPlot_1_1SourceInspection.md) _Inspection queries that read a series' own buffer whose records are ordered along one axis._     
    * **class** [**SpatialGrid**](classQAccelPlot_1_1SpatialGrid.md) _Uniform-grid spatial index with bounded per-rectangle storage._     
    * **struct** [**SummaryAccumulator**](structQAccelPlot_1_1SummaryAccumulator.md) _Running sample-weighted Y statistics that can be merged across disjoint sample sets._     
    * **class** [**TextTickLabelFormatter**](classQAccelPlot_1_1TextTickLabelFormatter.md) _A tick label formatter that maps integer tick indices to a user-supplied list of strings._     
    * **class** [**TickLabelFormatter**](classQAccelPlot_1_1TickLabelFormatter.md) _Abstract base class for tick label formatters._     
* **struct** [**DataRange**](structQAccelPlot_1_1Axis_1_1DataRange.md)     
* **struct** [**ArcLengthScale**](structQAccelPlot_1_1BandEdgeRenderer_1_1ArcLengthScale.md)     
* **struct** [**RenderView**](structQAccelPlot_1_1BandSeries_1_1RenderView.md)     
* **struct** [**Span**](structQAccelPlot_1_1BandSeries_1_1Span.md)     
* **struct** [**Layout**](structQAccelPlot_1_1ColorBar_1_1Layout.md)     
* **struct** [**GridLineCollectionParams**](structQAccelPlot_1_1GridNode_1_1GridLineCollectionParams.md)     
* **struct** [**Job**](structQAccelPlot_1_1InspectionCache_1_1Job.md)     
* **struct** [**Node**](structQAccelPlot_1_1InspectionIndex_1_1Node.md)     
* **struct** [**FillSampleCache**](structQAccelPlot_1_1LineCurveLineRenderer_1_1FillSampleCache.md)     
* **struct** [**IndexedPoint**](structQAccelPlot_1_1PointSpatialIndex_1_1IndexedPoint.md)     
* **struct** [**Vertex**](structQAccelPlot_1_1RectVertexCache_1_1Vertex.md)     
* **struct** [**Region**](structQAccelPlot_1_1SelectionTool_1_1Region.md)     
* **struct** [**Block**](structQAccelPlot_1_1SourceInspection_1_1Block.md)     
* **struct** [**ItemBounds**](structQAccelPlot_1_1SpatialGrid_1_1ItemBounds.md)     

