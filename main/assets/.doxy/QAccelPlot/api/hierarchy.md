
# Class Hierarchy

This inheritance list is sorted roughly, but not completely, alphabetically:


* **class** [**QAccelPlot::AxisTickPainter**](classQAccelPlot_1_1AxisTickPainter.md) _Internal helper that paints tick marks and labels onto a QPainter for a single_ [_**Axis**_](classQAccelPlot_1_1Axis.md) _._
* **class** [**QAccelPlot::GradientTexture**](classQAccelPlot_1_1GradientTexture.md) _Cached one-dimensional texture used by gradient materials._ 
* **class** [**QAccelPlot::LineCurveLineRenderer**](classQAccelPlot_1_1LineCurveLineRenderer.md) _Internal renderer responsible for building and updating QSGNode line geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._
* **class** [**QAccelPlot::LineCurvePointRenderer**](classQAccelPlot_1_1LineCurvePointRenderer.md) _Internal renderer responsible for building and updating QSGNode marker geometry for a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _._
* **class** [**QAccelPlot::LineCurveVertexCache**](classQAccelPlot_1_1LineCurveVertexCache.md) _Owns a_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _'s pre-built vertex bytes and the metadata required to use them safely._
* **class** [**QAccelPlot::SpatialGrid**](classQAccelPlot_1_1SpatialGrid.md) _Uniform-grid spatial index for O(1) point-in-rectangle hit-test queries._ 
* **struct** [**QAccelPlot::AxisTickPainter::PaintContext**](structQAccelPlot_1_1AxisTickPainter_1_1PaintContext.md) _Groups the QPainter context arguments passed into sub-painting helpers._ 
* **struct** [**QAccelPlot::AxisTickPainter::Params**](structQAccelPlot_1_1AxisTickPainter_1_1Params.md) _All inputs required for a single paint call, bundled to reduce parameter count._ 
* **struct** [**QAccelPlot::CurveChunk**](structQAccelPlot_1_1CurveChunk.md) _Axis-aligned bounding box (AABB) for a contiguous block of curve points, used for hit-test culling._ 
* **struct** [**QAccelPlot::CurveHitTestParams**](structQAccelPlot_1_1CurveHitTestParams.md) _All inputs required for a_ `contains()` _hit-test, bundled to reduce parameter count._
* **struct** [**QAccelPlot::DashParameters**](structQAccelPlot_1_1DashParameters.md) _Plain-data snapshot of dash rendering parameters._ 
* **struct** [**QAccelPlot::GradientColorPayload**](structQAccelPlot_1_1GradientColorPayload.md) _Render-thread snapshot of gradient stroke (line-color) parameters._ 
* **struct** [**QAccelPlot::GradientFillPayload**](structQAccelPlot_1_1GradientFillPayload.md) _Render-thread snapshot of gradient fill (area-under-curve) parameters._ 
* **struct** [**QAccelPlot::GradientStopData**](structQAccelPlot_1_1GradientStopData.md) _A single color stop within a gradient definition._ 
* **struct** [**QAccelPlot::LineCurveRenderParams**](structQAccelPlot_1_1LineCurveRenderParams.md) _Input parameters for_ [_**LineCurveLineRenderer::paint()**_](classQAccelPlot_1_1LineCurveLineRenderer.md#function-paint) _, assembled on the main thread._
* **struct** [**QAccelPlot::LineVertex**](structQAccelPlot_1_1LineVertex.md) _Vertex layout for line geometry, shared with the main thread for pre-built vertex caches._ 
* **struct** [**QAccelPlot::PointCurveRenderParams**](structQAccelPlot_1_1PointCurveRenderParams.md) _Input parameters for_ [_**LineCurvePointRenderer::paint()**_](classQAccelPlot_1_1LineCurvePointRenderer.md#function-paint) _, assembled on the main thread._
* **struct** [**QAccelPlot::PointVertex**](structQAccelPlot_1_1PointVertex.md) _Vertex layout for point (marker) geometry, shared with the main thread for vertex caches._ 
* **struct** [**QAccelPlot::Axis::DataRange**](structQAccelPlot_1_1Axis_1_1DataRange.md) 
* **struct** [**QAccelPlot::GridNode::GridLineCollectionParams**](structQAccelPlot_1_1GridNode_1_1GridLineCollectionParams.md) 
* **struct** [**QAccelPlot::RectangleList::RectVertex**](structQAccelPlot_1_1RectangleList_1_1RectVertex.md) 
* **struct** [**QAccelPlot::SpatialGrid::ItemBounds**](structQAccelPlot_1_1SpatialGrid_1_1ItemBounds.md) 
* **class** **QQuickPaintedItem**    
    * **class** [**QAccelPlot::Axis**](classQAccelPlot_1_1Axis.md) _A visual axis item that maps a data-space range to pixel coordinates and renders tick marks and labels._ 
* **class** **QObject**    
    * **class** [**QAccelPlot::AxisTicker**](classQAccelPlot_1_1AxisTicker.md) _Controls the visual appearance of ticks, sub-ticks, and tick labels on an_ `Axis` _._
    * **class** [**QAccelPlot::LineStyle**](classQAccelPlot_1_1LineStyle.md) _Abstract base class for all line styles._     
        * **class** [**QAccelPlot::DashLine**](classQAccelPlot_1_1DashLine.md) _A line style that renders the curve as a customisable dashed line._ 
        * **class** [**QAccelPlot::NoLine**](classQAccelPlot_1_1NoLine.md) _A line style that suppresses line rendering entirely, leaving only markers visible._ 
        * **class** [**QAccelPlot::SolidLine**](classQAccelPlot_1_1SolidLine.md) _The default line style — renders a continuous solid line with no gaps._ 
    * **class** [**QAccelPlot::DataTransition**](classQAccelPlot_1_1DataTransition.md) _Abstract base class for animated data transitions on plot elements._     
        * **class** [**QAccelPlot::DrawTransition**](classQAccelPlot_1_1DrawTransition.md) _An animation transition that reveals the target curve by drawing it point-by-point from start to end._ 
        * **class** [**QAccelPlot::MorphTransition**](classQAccelPlot_1_1MorphTransition.md) _An animation transition that smoothly interpolates point positions between two datasets._ 
    * **class** [**QAccelPlot::TickLabelFormatter**](classQAccelPlot_1_1TickLabelFormatter.md) _Abstract base class for tick label formatters._     
        * **class** [**QAccelPlot::DateTimeTickLabelFormatter**](classQAccelPlot_1_1DateTimeTickLabelFormatter.md) _A tick label formatter that displays tick values as formatted date/time strings._ 
        * **class** [**QAccelPlot::LogTickLabelFormatter**](classQAccelPlot_1_1LogTickLabelFormatter.md) _A tick label formatter that produces power-of-ten labels for logarithmic axes._ 
        * **class** [**QAccelPlot::NumericTickLabelFormatter**](classQAccelPlot_1_1NumericTickLabelFormatter.md) _The default tick label formatter — produces numeric labels with automatic decimal precision._ 
        * **class** [**QAccelPlot::TextTickLabelFormatter**](classQAccelPlot_1_1TextTickLabelFormatter.md) _A tick label formatter that maps integer tick indices to a user-supplied list of strings._ 
    * **class** [**QAccelPlot::DataTransition**](classQAccelPlot_1_1DataTransition.md) _Abstract base class for animated data transitions on plot elements._     
        * **class** [**QAccelPlot::DrawTransition**](classQAccelPlot_1_1DrawTransition.md) _An animation transition that reveals the target curve by drawing it point-by-point from start to end._ 
        * **class** [**QAccelPlot::MorphTransition**](classQAccelPlot_1_1MorphTransition.md) _An animation transition that smoothly interpolates point positions between two datasets._ 
    * **class** [**QAccelPlot::LineCurveEffect**](classQAccelPlot_1_1LineCurveEffect.md) _Abstract base class for visual effects applied to a_ `LineCurve` _._    
        * **class** [**QAccelPlot::GradientFill**](classQAccelPlot_1_1GradientFill.md) _A_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _effect that fills the area under the curve with a color gradient._
        * **class** [**QAccelPlot::GradientStroke**](classQAccelPlot_1_1GradientStroke.md) _A_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _effect that replaces the solid line color with a color gradient._
    * **class** [**QAccelPlot::LineCurveEffect**](classQAccelPlot_1_1LineCurveEffect.md) _Abstract base class for visual effects applied to a_ `LineCurve` _._    
        * **class** [**QAccelPlot::GradientFill**](classQAccelPlot_1_1GradientFill.md) _A_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _effect that fills the area under the curve with a color gradient._
        * **class** [**QAccelPlot::GradientStroke**](classQAccelPlot_1_1GradientStroke.md) _A_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _effect that replaces the solid line color with a color gradient._
    * **class** [**QAccelPlot::Grid**](classQAccelPlot_1_1Grid.md) _Configuration object that controls the appearance of the plot grid._ 
    * **class** [**QAccelPlot::LineCurveEffect**](classQAccelPlot_1_1LineCurveEffect.md) _Abstract base class for visual effects applied to a_ `LineCurve` _._    
        * **class** [**QAccelPlot::GradientFill**](classQAccelPlot_1_1GradientFill.md) _A_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _effect that fills the area under the curve with a color gradient._
        * **class** [**QAccelPlot::GradientStroke**](classQAccelPlot_1_1GradientStroke.md) _A_ [_**LineCurve**_](classQAccelPlot_1_1LineCurve.md) _effect that replaces the solid line color with a color gradient._
    * **class** [**QAccelPlot::LineStyle**](classQAccelPlot_1_1LineStyle.md) _Abstract base class for all line styles._     
        * **class** [**QAccelPlot::DashLine**](classQAccelPlot_1_1DashLine.md) _A line style that renders the curve as a customisable dashed line._ 
        * **class** [**QAccelPlot::NoLine**](classQAccelPlot_1_1NoLine.md) _A line style that suppresses line rendering entirely, leaving only markers visible._ 
        * **class** [**QAccelPlot::SolidLine**](classQAccelPlot_1_1SolidLine.md) _The default line style — renders a continuous solid line with no gaps._ 
    * **class** [**QAccelPlot::TickLabelFormatter**](classQAccelPlot_1_1TickLabelFormatter.md) _Abstract base class for tick label formatters._     
        * **class** [**QAccelPlot::DateTimeTickLabelFormatter**](classQAccelPlot_1_1DateTimeTickLabelFormatter.md) _A tick label formatter that displays tick values as formatted date/time strings._ 
        * **class** [**QAccelPlot::LogTickLabelFormatter**](classQAccelPlot_1_1LogTickLabelFormatter.md) _A tick label formatter that produces power-of-ten labels for logarithmic axes._ 
        * **class** [**QAccelPlot::NumericTickLabelFormatter**](classQAccelPlot_1_1NumericTickLabelFormatter.md) _The default tick label formatter — produces numeric labels with automatic decimal precision._ 
        * **class** [**QAccelPlot::TextTickLabelFormatter**](classQAccelPlot_1_1TextTickLabelFormatter.md) _A tick label formatter that maps integer tick indices to a user-supplied list of strings._ 
    * **class** [**QAccelPlot::DataTransition**](classQAccelPlot_1_1DataTransition.md) _Abstract base class for animated data transitions on plot elements._     
        * **class** [**QAccelPlot::DrawTransition**](classQAccelPlot_1_1DrawTransition.md) _An animation transition that reveals the target curve by drawing it point-by-point from start to end._ 
        * **class** [**QAccelPlot::MorphTransition**](classQAccelPlot_1_1MorphTransition.md) _An animation transition that smoothly interpolates point positions between two datasets._ 
    * **class** [**QAccelPlot::LineStyle**](classQAccelPlot_1_1LineStyle.md) _Abstract base class for all line styles._     
        * **class** [**QAccelPlot::DashLine**](classQAccelPlot_1_1DashLine.md) _A line style that renders the curve as a customisable dashed line._ 
        * **class** [**QAccelPlot::NoLine**](classQAccelPlot_1_1NoLine.md) _A line style that suppresses line rendering entirely, leaving only markers visible._ 
        * **class** [**QAccelPlot::SolidLine**](classQAccelPlot_1_1SolidLine.md) _The default line style — renders a continuous solid line with no gaps._ 
    * **class** [**QAccelPlot::TickLabelFormatter**](classQAccelPlot_1_1TickLabelFormatter.md) _Abstract base class for tick label formatters._     
        * **class** [**QAccelPlot::DateTimeTickLabelFormatter**](classQAccelPlot_1_1DateTimeTickLabelFormatter.md) _A tick label formatter that displays tick values as formatted date/time strings._ 
        * **class** [**QAccelPlot::LogTickLabelFormatter**](classQAccelPlot_1_1LogTickLabelFormatter.md) _A tick label formatter that produces power-of-ten labels for logarithmic axes._ 
        * **class** [**QAccelPlot::NumericTickLabelFormatter**](classQAccelPlot_1_1NumericTickLabelFormatter.md) _The default tick label formatter — produces numeric labels with automatic decimal precision._ 
        * **class** [**QAccelPlot::TextTickLabelFormatter**](classQAccelPlot_1_1TextTickLabelFormatter.md) _A tick label formatter that maps integer tick indices to a user-supplied list of strings._ 
    * **class** [**QAccelPlot::PlotBorder**](classQAccelPlot_1_1PlotBorder.md) _Decorative frame configuration exposed by_ `PlotView::border` _._
    * **class** [**QAccelPlot::PlotMouseEvent**](classQAccelPlot_1_1PlotMouseEvent.md) _Carries mouse event data for the mouse signals._ 
    * **class** [**QAccelPlot::LineStyle**](classQAccelPlot_1_1LineStyle.md) _Abstract base class for all line styles._     
        * **class** [**QAccelPlot::DashLine**](classQAccelPlot_1_1DashLine.md) _A line style that renders the curve as a customisable dashed line._ 
        * **class** [**QAccelPlot::NoLine**](classQAccelPlot_1_1NoLine.md) _A line style that suppresses line rendering entirely, leaving only markers visible._ 
        * **class** [**QAccelPlot::SolidLine**](classQAccelPlot_1_1SolidLine.md) _The default line style — renders a continuous solid line with no gaps._ 
    * **class** [**QAccelPlot::TickLabelFormatter**](classQAccelPlot_1_1TickLabelFormatter.md) _Abstract base class for tick label formatters._     
        * **class** [**QAccelPlot::DateTimeTickLabelFormatter**](classQAccelPlot_1_1DateTimeTickLabelFormatter.md) _A tick label formatter that displays tick values as formatted date/time strings._ 
        * **class** [**QAccelPlot::LogTickLabelFormatter**](classQAccelPlot_1_1LogTickLabelFormatter.md) _A tick label formatter that produces power-of-ten labels for logarithmic axes._ 
        * **class** [**QAccelPlot::NumericTickLabelFormatter**](classQAccelPlot_1_1NumericTickLabelFormatter.md) _The default tick label formatter — produces numeric labels with automatic decimal precision._ 
        * **class** [**QAccelPlot::TextTickLabelFormatter**](classQAccelPlot_1_1TextTickLabelFormatter.md) _A tick label formatter that maps integer tick indices to a user-supplied list of strings._ 
    * **class** [**QAccelPlot::TickLabelFormatter**](classQAccelPlot_1_1TickLabelFormatter.md) _Abstract base class for tick label formatters._     
        * **class** [**QAccelPlot::DateTimeTickLabelFormatter**](classQAccelPlot_1_1DateTimeTickLabelFormatter.md) _A tick label formatter that displays tick values as formatted date/time strings._ 
        * **class** [**QAccelPlot::LogTickLabelFormatter**](classQAccelPlot_1_1LogTickLabelFormatter.md) _A tick label formatter that produces power-of-ten labels for logarithmic axes._ 
        * **class** [**QAccelPlot::NumericTickLabelFormatter**](classQAccelPlot_1_1NumericTickLabelFormatter.md) _The default tick label formatter — produces numeric labels with automatic decimal precision._ 
        * **class** [**QAccelPlot::TextTickLabelFormatter**](classQAccelPlot_1_1TextTickLabelFormatter.md) _A tick label formatter that maps integer tick indices to a user-supplied list of strings._ 
* **class** **QQuickItem**    
    * **class** [**QAccelPlot::DataAnchor**](classQAccelPlot_1_1DataAnchor.md) _A QQuickItem that tracks a data-coordinate rectangle in pixel space._ 
    * **class** [**QAccelPlot::PlotSeries**](classQAccelPlot_1_1PlotSeries.md) _Common QML item contract for data series hosted by_ `PlotView` _._    
        * **class** [**QAccelPlot::LineCurve**](classQAccelPlot_1_1LineCurve.md) _A hardware-accelerated QML item that renders a 2D line curve with optional markers, dashing, and gradient effects._ 
        * **class** [**QAccelPlot::RectangleList**](classQAccelPlot_1_1RectangleList.md) _A hardware-accelerated QML item that renders a large list of axis-aligned rectangles._ 
    * **class** [**QAccelPlot::PlotSeries**](classQAccelPlot_1_1PlotSeries.md) _Common QML item contract for data series hosted by_ `PlotView` _._    
        * **class** [**QAccelPlot::LineCurve**](classQAccelPlot_1_1LineCurve.md) _A hardware-accelerated QML item that renders a 2D line curve with optional markers, dashing, and gradient effects._ 
        * **class** [**QAccelPlot::RectangleList**](classQAccelPlot_1_1RectangleList.md) _A hardware-accelerated QML item that renders a large list of axis-aligned rectangles._ 
    * **class** [**QAccelPlot::QAccelPlot**](classQAccelPlot_1_1QAccelPlot.md) _The main plot canvas QML item — hosts axes, curves, and a grid._ 
    * **class** [**QAccelPlot::PlotSeries**](classQAccelPlot_1_1PlotSeries.md) _Common QML item contract for data series hosted by_ `PlotView` _._    
        * **class** [**QAccelPlot::LineCurve**](classQAccelPlot_1_1LineCurve.md) _A hardware-accelerated QML item that renders a 2D line curve with optional markers, dashing, and gradient effects._ 
        * **class** [**QAccelPlot::RectangleList**](classQAccelPlot_1_1RectangleList.md) _A hardware-accelerated QML item that renders a large list of axis-aligned rectangles._ 
* **class** **QSGMaterial**    
    * **class** [**QAccelPlot::DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md) _Base QSGMaterial that uploads curve data as a floating-point texture and exposes shared shader uniforms._     
        * **class** [**QAccelPlot::LineMaterial**](classQAccelPlot_1_1LineMaterial.md) _QSGMaterial for line rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with line-specific uniforms._    
            * **class** [**QAccelPlot::GradientLineMaterial**](classQAccelPlot_1_1GradientLineMaterial.md) _Line material variant that samples a one-dimensional gradient texture._ 
        * **class** [**QAccelPlot::RectMaterial**](classQAccelPlot_1_1RectMaterial.md) _QSGMaterial for rectangle list rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with a rect-count uniform._
    * **class** [**QAccelPlot::GradientFillMaterial**](classQAccelPlot_1_1GradientFillMaterial.md) _Scene-graph material that evaluates fill gradients per fragment._ 
    * **class** [**QAccelPlot::DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md) _Base QSGMaterial that uploads curve data as a floating-point texture and exposes shared shader uniforms._     
        * **class** [**QAccelPlot::LineMaterial**](classQAccelPlot_1_1LineMaterial.md) _QSGMaterial for line rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with line-specific uniforms._    
            * **class** [**QAccelPlot::GradientLineMaterial**](classQAccelPlot_1_1GradientLineMaterial.md) _Line material variant that samples a one-dimensional gradient texture._ 
        * **class** [**QAccelPlot::RectMaterial**](classQAccelPlot_1_1RectMaterial.md) _QSGMaterial for rectangle list rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with a rect-count uniform._
    * **class** [**QAccelPlot::DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md) _Base QSGMaterial that uploads curve data as a floating-point texture and exposes shared shader uniforms._     
        * **class** [**QAccelPlot::LineMaterial**](classQAccelPlot_1_1LineMaterial.md) _QSGMaterial for line rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with line-specific uniforms._    
            * **class** [**QAccelPlot::GradientLineMaterial**](classQAccelPlot_1_1GradientLineMaterial.md) _Line material variant that samples a one-dimensional gradient texture._ 
        * **class** [**QAccelPlot::RectMaterial**](classQAccelPlot_1_1RectMaterial.md) _QSGMaterial for rectangle list rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with a rect-count uniform._
    * **class** [**QAccelPlot::PointMaterial**](classQAccelPlot_1_1PointMaterial.md) _QSGMaterial for marker (point) rendering._ 
    * **class** [**QAccelPlot::DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md) _Base QSGMaterial that uploads curve data as a floating-point texture and exposes shared shader uniforms._     
        * **class** [**QAccelPlot::LineMaterial**](classQAccelPlot_1_1LineMaterial.md) _QSGMaterial for line rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with line-specific uniforms._    
            * **class** [**QAccelPlot::GradientLineMaterial**](classQAccelPlot_1_1GradientLineMaterial.md) _Line material variant that samples a one-dimensional gradient texture._ 
        * **class** [**QAccelPlot::RectMaterial**](classQAccelPlot_1_1RectMaterial.md) _QSGMaterial for rectangle list rendering, extending_ [_**DataTextureMaterial**_](classQAccelPlot_1_1DataTextureMaterial.md) _with a rect-count uniform._
* **class** **QSGNode**    
    * **class** [**QAccelPlot::GridNode**](classQAccelPlot_1_1GridNode.md) _Internal QSGNode responsible for rendering the plot grid into the scene graph._ 

