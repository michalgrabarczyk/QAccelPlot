
# Class List


Here are the classes, structs, unions and interfaces with brief descriptions:

* **namespace** [**QAccelPlot**](namespaceQAccelPlot.md)     
    * **class** [**Axis**](classQAccelPlot_1_1Axis.md) _A visual axis item that maps a data-space range to pixel coordinates and renders tick marks and labels._     
    * **class** [**AxisTickPainter**](classQAccelPlot_1_1AxisTickPainter.md) _Internal helper that paints tick marks and labels onto a QPainter for a single_ [_**Axis**_](classQAccelPlot_1_1Axis.md) _._    
        * **struct** [**PaintContext**](structQAccelPlot_1_1AxisTickPainter_1_1PaintContext.md) _Groups the QPainter context arguments passed into sub-painting helpers._     
        * **struct** [**Params**](structQAccelPlot_1_1AxisTickPainter_1_1Params.md) _All inputs required for a single paint call, bundled to reduce parameter count._     
    * **class** [**AxisTicker**](classQAccelPlot_1_1AxisTicker.md) _Controls the visual appearance of ticks, sub-ticks, and tick labels on an_ `Axis` _._    
    * **struct** [**CurveChunk**](structQAccelPlot_1_1CurveChunk.md) _Axis-aligned bounding box (AABB) for a contiguous block of curve points, used for hit-test culling._     
    * **struct** [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) _Read-only view over either interleaved float or double curve coordinates._     
    * **struct** [**CurveHitTestParams**](structQAccelPlot_1_1CurveHitTestParams.md) _All inputs required for a_ `contains()` _hit-test, bundled to reduce parameter count._    
    * **class** [**DashLine**](classQAccelPlot_1_1DashLine.md) _A line style that renders the curve as a customisable dashed line._     
    * **struct** [**DashParameters**](structQAccelPlot_1_1DashParameters.md) _Plain-data snapshot of dash rendering parameters._     
    * **class** [**DataAnchor**](classQAccelPlot_1_1DataAnchor.md) _A QQuickItem that tracks a data-coordinate rectangle in pixel space._     
    * **class** [**DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md) _Base QSGMaterial that uploads curve data as a floating-point texture and exposes shared shader uniforms._     
    * **class** [**DataTransition**](classQAccelPlot_1_1DataTransition.md) _Abstract base class for animated data transitions on plot elements._     
    * **class** [**DateTimeTickLabelFormatter**](classQAccelPlot_1_1DateTimeTickLabelFormatter.md) _A tick label formatter that displays tick values as formatted date/time strings._     
    * **class** [**DrawTransition**](classQAccelPlot_1_1DrawTransition.md) _An animation transition that reveals the target curve by drawing it point-by-point from start to end._     
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
    * **namespace** [**Internal**](namespaceQAccelPlot_1_1Internal.md)     
    * **class** [**LineCurve**](classQAccelPlot_1_1LineCurve.md) _A hardware-accelerated QML item that renders a 2D line curve with optional markers, dashing, and gradient effects._     
    * **class** [**LineCurveEffect**](classQAccelPlot_1_1LineCurveEffect.md) _Abstract base class for visual effects applied to a_ `LineCurve` _._    
    * **class** [**LineCurveLineRenderer**](classQAccelPlot_1_1LineCurveLineRenderer.md) _Internal renderer responsible for building and updating QSGNode line geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._    
    * **class** [**LineCurvePointRenderer**](classQAccelPlot_1_1LineCurvePointRenderer.md) _Internal renderer responsible for building and updating QSGNode marker geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._    
    * **struct** [**LineCurveRenderParams**](structQAccelPlot_1_1LineCurveRenderParams.md) _Input parameters for_ [_**LineCurveLineRenderer::paint()**_](classQAccelPlot_1_1LineCurveLineRenderer.md#function-paint) _, assembled on the main thread._    
    * **class** [**LineCurveVertexCache**](classQAccelPlot_1_1LineCurveVertexCache.md) _Owns a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _'s pre-built vertex bytes and the metadata required to use them safely._    
    * **class** [**LineMaterial**](classQAccelPlot_1_1LineMaterial.md) _QSGMaterial for line rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with line-specific uniforms._    
    * **class** [**LineStyle**](classQAccelPlot_1_1LineStyle.md) _Abstract base class for all line styles._     
    * **struct** [**LineVertex**](structQAccelPlot_1_1LineVertex.md) _Vertex layout for line geometry, shared with the main thread for pre-built vertex caches._     
    * **class** [**LogTickLabelFormatter**](classQAccelPlot_1_1LogTickLabelFormatter.md) _A tick label formatter that produces power-of-ten labels for logarithmic axes._     
    * **class** [**MorphTransition**](classQAccelPlot_1_1MorphTransition.md) _An animation transition that smoothly interpolates point positions between two datasets._     
    * **class** [**NoLine**](classQAccelPlot_1_1NoLine.md) _A line style that suppresses line rendering entirely, leaving only markers visible._     
    * **class** [**NumericTickLabelFormatter**](classQAccelPlot_1_1NumericTickLabelFormatter.md) _The default tick label formatter — produces numeric labels with automatic decimal precision._     
    * **class** [**PlotBorder**](classQAccelPlot_1_1PlotBorder.md) _Decorative frame configuration exposed by_ `PlotView::border` _._    
    * **class** [**PlotMouseEvent**](classQAccelPlot_1_1PlotMouseEvent.md) _Carries mouse event data for the mouse signals._     
    * **class** [**PlotSeries**](classQAccelPlot_1_1PlotSeries.md) _Common QML item contract for data series hosted by_ `PlotView` _._    
    * **struct** [**PointCurveRenderParams**](structQAccelPlot_1_1PointCurveRenderParams.md) _Input parameters for_ [_**LineCurvePointRenderer::paint()**_](classQAccelPlot_1_1LineCurvePointRenderer.md#function-paint) _, assembled on the main thread._    
    * **class** [**PointMaterial**](classQAccelPlot_1_1PointMaterial.md) _QSGMaterial for marker (point) rendering._     
    * **struct** [**PointVertex**](structQAccelPlot_1_1PointVertex.md) _Vertex layout for point (marker) geometry, shared with the main thread for vertex caches._     
    * **class** [**QAccelPlot**](classQAccelPlot_1_1QAccelPlot.md) _The main plot canvas QML item — hosts axes, curves, and a grid._     
    * **class** [**RectMaterial**](classQAccelPlot_1_1RectMaterial.md) _QSGMaterial for rectangle list rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with a rect-count uniform._    
    * **class** [**RectangleList**](classQAccelPlot_1_1RectangleList.md) _A hardware-accelerated QML item that renders a large list of axis-aligned rectangles._     
    * **class** [**SolidLine**](classQAccelPlot_1_1SolidLine.md) _The default line style — renders a continuous solid line with no gaps._     
    * **class** [**SpatialGrid**](classQAccelPlot_1_1SpatialGrid.md) _Uniform-grid spatial index for O(1) point-in-rectangle hit-test queries._     
    * **class** [**TextTickLabelFormatter**](classQAccelPlot_1_1TextTickLabelFormatter.md) _A tick label formatter that maps integer tick indices to a user-supplied list of strings._     
    * **class** [**TickLabelFormatter**](classQAccelPlot_1_1TickLabelFormatter.md) _Abstract base class for tick label formatters._     
* **struct** [**DataRange**](structQAccelPlot_1_1Axis_1_1DataRange.md)     
* **struct** [**GridLineCollectionParams**](structQAccelPlot_1_1GridNode_1_1GridLineCollectionParams.md)     
* **struct** [**RectVertex**](structQAccelPlot_1_1RectangleList_1_1RectVertex.md)     
* **struct** [**ItemBounds**](structQAccelPlot_1_1SpatialGrid_1_1ItemBounds.md)     

