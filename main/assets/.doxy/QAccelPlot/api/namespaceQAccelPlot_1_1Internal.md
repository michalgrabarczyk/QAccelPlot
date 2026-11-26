








# Namespace QAccelPlot::Internal



[**Namespace List**](namespaces.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**Internal**](namespaceQAccelPlot_1_1Internal.md)




















## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**InspectionScan**](namespaceQAccelPlot_1_1Internal_1_1InspectionScan.md) <br>_Inspection queries that scan every record; used for small series that are not ordered along the queried axis._  |


## Classes

| Type | Name |
| ---: | :--- |
| struct | [**AxisLabelLayoutParams**](structQAccelPlot_1_1Internal_1_1AxisLabelLayoutParams.md) <br>_Inputs that place an axis label relative to the axis baseline._  |
| class | [**CategoryList**](classQAccelPlot_1_1Internal_1_1CategoryList.md) <br>_The_ `categories` _list of a series: the_`Category` _objects and a lookup table that follows their edits._ |
| class | [**CategoryTable**](classQAccelPlot_1_1Internal_1_1CategoryTable.md) <br>_A snapshot of the categories of a series, looked up by value or name._  |
| struct | [**DataTextureUploadTotals**](structQAccelPlot_1_1Internal_1_1DataTextureUploadTotals.md) <br>_How many data textures were uploaded since the last reset, and how many bytes they held._  |
| class | [**FrontStarts**](classQAccelPlot_1_1Internal_1_1FrontStarts.md) <br>_When an eased reveal front reaches each share of the items._  |
| struct | [**GlowStyle**](structQAccelPlot_1_1Internal_1_1GlowStyle.md) <br>_How the shaders draw a_ `Glow` _halo; see shaders/glow.glsl._ |
| struct | [**GlowUniforms**](structQAccelPlot_1_1Internal_1_1GlowUniforms.md) <br>_The std140 uniform members of a_ `Glow` _halo,_`glowColor` _and_`glowParams` _, at a 16-byte aligned offset._ |
| struct | [**GrabSize**](structQAccelPlot_1_1Internal_1_1GrabSize.md) <br>_Sizes of one item grab._  |
| struct | [**GrowViewport**](structQAccelPlot_1_1Internal_1_1GrowViewport.md) <br>_The viewport of the axis items grow along, which places a baseline the axis cannot show._  |
| struct | [**HeatmapGridDimension**](structQAccelPlot_1_1Internal_1_1HeatmapGridDimension.md) <br>_One dimension of a heatmap grid:_ `count` _cells between the edges_`from` _and_`to` _._ |
| struct | [**HeatmapShaderMapping**](structQAccelPlot_1_1Internal_1_1HeatmapShaderMapping.md) <br>_How heatmap.frag maps an item-local pixel position_ _p_ _to a fractional cell coordinate along one dimension._ |
| class | [**HoverIndexBudget**](classQAccelPlot_1_1Internal_1_1HoverIndexBudget.md) <br>_Decides when queried data is worth a hover index._  |
| struct | [**InspectionBounds**](structQAccelPlot_1_1Internal_1_1InspectionBounds.md) <br>_Inclusive data-space region; infinite limits leave a dimension unbounded._  |
| class | [**InspectionCache**](classQAccelPlot_1_1Internal_1_1InspectionCache.md) <br>_Builds an_ `InspectionIndex` _on a worker thread from a snapshot of a series' records._ |
| struct | [**InspectionHit**](structQAccelPlot_1_1Internal_1_1InspectionHit.md) <br>_Nearest-data-point result: source index and pixel distance, or index -1._  |
| class | [**InspectionIndex**](classQAccelPlot_1_1Internal_1_1InspectionIndex.md) <br>_Immutable k-d tree over the valid data points of a series, with their order along each axis._  |
| struct | [**InspectionMetric**](structQAccelPlot_1_1Internal_1_1InspectionMetric.md) <br>_Pixel mapping of a series' plot area, used to measure on-screen distances._  |
| struct | [**InspectionNeighbors**](structQAccelPlot_1_1Internal_1_1InspectionNeighbors.md) <br>_Source indices of the valid data points on either side of an X value, or -1._  |
| struct | [**InspectionSource**](structQAccelPlot_1_1Internal_1_1InspectionSource.md) <br>_Read-only view over the XY records a series exposes to inspection queries._  |
| class | [**ItemClock**](classQAccelPlot_1_1Internal_1_1ItemClock.md) <br>_The start time and movement of each item of one group at the frame._  |
| struct | [**ItemFade**](structQAccelPlot_1_1Internal_1_1ItemFade.md) <br>_The transition groups of a frame whose opacity the shaders animate._  |
| struct | [**ItemFadeGroup**](structQAccelPlot_1_1Internal_1_1ItemFadeGroup.md) <br>_The items of one transition group whose opacity the shaders animate._  |
| struct | [**ItemFadeGroupUniforms**](structQAccelPlot_1_1Internal_1_1ItemFadeGroupUniforms.md) <br>_The std140 uniform members of one fading transition group, see_ `ItemFadeGroup` _._ |
| struct | [**ItemOpacityUniforms**](structQAccelPlot_1_1Internal_1_1ItemOpacityUniforms.md) <br>_The std140 uniform members that shaders/item\_opacity.glsl reads, at a 16-byte aligned offset._  |
| struct | [**ItemTiming**](structQAccelPlot_1_1Internal_1_1ItemTiming.md) <br>_The timing of the items of one group, as shares of the transition duration._  |
| struct | [**ItemTrail**](structQAccelPlot_1_1Internal_1_1ItemTrail.md) <br>_How the shaders fade the oldest items under a_ `FadeTrail` _effect._ |
| struct | [**OpacitySegment**](structQAccelPlot_1_1Internal_1_1OpacitySegment.md) <br>_One animation of item opacity, on the timeline of an item._  |
| class | [**OpacityTimeline**](classQAccelPlot_1_1Internal_1_1OpacityTimeline.md) <br>_The opacity of an item over the time since it started, built from opacity animations._  |
| class | [**OutlinedRectangle**](classQAccelPlot_1_1Internal_1_1OutlinedRectangle.md) <br>_Item that fills its bounds and outlines them inside its edges._  |
| class | [**OverlayChildren**](classQAccelPlot_1_1Internal_1_1OverlayChildren.md) <br>_Objects declared inside an inspection tool in QML._  |
| class | [**PointerTracker**](classQAccelPlot_1_1Internal_1_1PointerTracker.md) <br>_Tracks the pointer over a plot from the plot's mouse and hover events._  |
| struct | [**RecordFrame**](structQAccelPlot_1_1Internal_1_1RecordFrame.md) <br>_What a series that draws one record per data point draws in a frame._  |
| class | [**RecordHitIndex**](classQAccelPlot_1_1Internal_1_1RecordHitIndex.md) <br>_Finds the drawn record nearest to a pixel position, for series that draw one record per data point._  |
| struct | [**RectUbo**](structQAccelPlot_1_1Internal_1_1RectUbo.md) <br>_Mirrors the std140 uniform block of rect.vert._  |
| class | [**RectVertexCache**](classQAccelPlot_1_1Internal_1_1RectVertexCache.md) <br>_Vertices of a series that draws each rectangle as a quad positioned from a data texture._  |
| class | [**RectangleZoomOverlay**](classQAccelPlot_1_1Internal_1_1RectangleZoomOverlay.md) <br> |
| class | [**ResettableValue**](classQAccelPlot_1_1Internal_1_1ResettableValue.md) &lt;typename T&gt;<br>_A property value that is either set explicitly or follows a default._  |
| class | [**ScopedPaintedItemTextures**](classQAccelPlot_1_1Internal_1_1ScopedPaintedItemTextures.md) <br>_Rasterizes the painted items under a root item at a capture scale until destroyed._  |
| class | [**SelectionRectangle**](classQAccelPlot_1_1Internal_1_1SelectionRectangle.md) <br>_Draws the gesture or the selected region of a_ [_**SelectionTool**_](classQAccelPlot_1_1SelectionTool.md) _, clipped to the plot area._ |
| class | [**SeriesEffectList**](classQAccelPlot_1_1Internal_1_1SeriesEffectList.md) <br>_The_ `effects` _list of one series._ |
| class | [**SourceInspection**](classQAccelPlot_1_1Internal_1_1SourceInspection.md) <br>_Inspection queries that read a series' own buffer whose records are ordered along one axis._  |
| class | [**StoreTransition**](classQAccelPlot_1_1Internal_1_1StoreTransition.md) <br>_Shows the runs of a_ `TransitionRunner` _as the drawn data points of a_`DataPointStore` _._ |
| struct | [**SummaryAccumulator**](structQAccelPlot_1_1Internal_1_1SummaryAccumulator.md) <br>_Running data-point-weighted Y statistics that can be merged across disjoint data point sets._  |
| struct | [**TransitionFrameInput**](structQAccelPlot_1_1Internal_1_1TransitionFrameInput.md) <br>_The two sides of a transition and the steps of its slots at one frame._  |
| struct | [**TransitionItems**](structQAccelPlot_1_1Internal_1_1TransitionItems.md) <br>_Items on one side of a transition,_ `TransitionLayout::stride` _values each._ |
| struct | [**TransitionLayout**](structQAccelPlot_1_1Internal_1_1TransitionLayout.md) <br>_How a series' items animate._  |
| struct | [**TransitionSlots**](structQAccelPlot_1_1Internal_1_1TransitionSlots.md) <br>_Slots a data change animates._  |
| struct | [**TransitionStep**](structQAccelPlot_1_1Internal_1_1TransitionStep.md) <br>_The state of one slot's transition at the frame._  |
| struct | [**UnitPrefix**](structQAccelPlot_1_1Internal_1_1UnitPrefix.md) <br>_An SI unit prefix: its output symbol and its power of ten._  |
| struct | [**ValueRange**](structQAccelPlot_1_1Internal_1_1ValueRange.md) <br>_Bounds of an axis viewport or a colormap value range._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**InspectionAxis**](#enum-inspectionaxis)  <br>_Data dimension a query measures along, or along which a series' records are ordered._  |
| enum  | [**InspectionMatching**](#enum-inspectionmatching)  <br>_How_ `PlotInspector` _and_`SelectionTool` _match a series' data points to a position._ |
| enum  | [**TransitionKind**](#enum-transitionkind)  <br>_How a transition moves its items: one value per transition class._  |




## Public Attributes

| Type | Name |
| ---: | :--- |
|  int | [**kFadeCurveSamples**](#variable-kfadecurvesamples)   = `32`<br>_Number of opacity samples over the timeline of an item._  |
|  int | [**kFadeGroupCount**](#variable-kfadegroupcount)   = `2`<br>_Most transition groups that fade at once: the kept items, and the added or removed ones._  |
|  int | [**kFadeStartSamples**](#variable-kfadestartsamples)   = `16`<br>_Number of samples of the start times of the items along an eased reveal front._  |
|  int | [**kMaxDataTextureWidth**](#variable-kmaxdatatexturewidth)   = `{8192}`<br>_Largest number of floats stored per data texture row, one float per RGBA8 texel._  |
|  int | [**kTrailEasingSamples**](#variable-ktraileasingsamples)   = `16`<br>_Number of samples of the_ `FadeTrail` _easing curve._ |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**allocateRecordVertices**](#function-allocaterecordvertices) (QSGGeometryNode & node, const [**RecordFrame**](structQAccelPlot_1_1Internal_1_1RecordFrame.md) & frame) <br>_Sizes the vertex buffer of_ _node_ _for_`reservedRecordCount()` _records of__frame_ _._ |
|  bool | [**assignNonNegative**](#function-assignnonnegative) (qreal & member, qreal requested) <br>_Stores_ _requested_ _, clamped to 0 or more, in__member_ _and returns_`true` _when that changes it._ |
|  QPointF | [**axisLabelCenter**](#function-axislabelcenter) (const [**AxisLabelLayoutParams**](structQAccelPlot_1_1Internal_1_1AxisLabelLayoutParams.md) & params) <br>_Returns the center of the axis label described by_ _params_ _, in item coordinates._ |
|  qreal | [**clampAntialiasingFeather**](#function-clampantialiasingfeather) (qreal feather) <br>_Returns_ _feather_ _clamped to the [0, 10] pixel range of the series'_`antialiasingFeather` _properties._ |
|  [**ItemFadeGroup**](structQAccelPlot_1_1Internal_1_1ItemFadeGroup.md) | [**compactedFadeGroup**](#function-compactedfadegroup) (const [**ItemFadeGroup**](structQAccelPlot_1_1Internal_1_1ItemFadeGroup.md) & group, int first, const std::vector&lt; int &gt; & kept) <br>_Returns_ _group_ _after some of its items are left out of the drawn items; it starts at drawn index__first_ _._ |
|  int | [**compareGlowStyles**](#function-compareglowstyles) (const [**GlowStyle**](structQAccelPlot_1_1Internal_1_1GlowStyle.md) & left, const [**GlowStyle**](structQAccelPlot_1_1Internal_1_1GlowStyle.md) & right) noexcept<br>_Orders_ _left_ _and__right_ _for_`QSGMaterial::compare()` _: negative, 0, or positive._ |
|  int | [**compareItemFades**](#function-compareitemfades) (const [**ItemFade**](structQAccelPlot_1_1Internal_1_1ItemFade.md) & left, const [**ItemFade**](structQAccelPlot_1_1Internal_1_1ItemFade.md) & right) noexcept<br>_Orders_ _left_ _and__right_ _for_`QSGMaterial::compare()` _: negative, 0, or positive._ |
|  int | [**compareItemTrails**](#function-compareitemtrails) (const [**ItemTrail**](structQAccelPlot_1_1Internal_1_1ItemTrail.md) & left, const [**ItemTrail**](structQAccelPlot_1_1Internal_1_1ItemTrail.md) & right) noexcept<br>_Orders_ _left_ _and__right_ _for_`QSGMaterial::compare()` _: negative, 0, or positive._ |
|  QSGGeometryNode \* | [**createRecordNode**](#function-createrecordnode) (const QSGGeometry::AttributeSet & attributes, QSGMaterial \* material) <br>_Returns an empty triangle node of_ _attributes_ _that owns its geometry and__material_ _._ |
|  int | [**dataTextureHeight**](#function-datatextureheight) (const int floatCount, const int width) <br>_Returns the number of rows of_ _width_ _floats needed to store__floatCount_ _floats._ |
|  qint64 | [**dataTextureItemCapacity**](#function-datatextureitemcapacity) (const int maxTextureSize, const int floatsPerItem) <br>_Returns how many items of_ _floatsPerItem_ _floats fit in a data texture on a GPU with texture limit__maxTextureSize_ _._ |
|  [**DataTextureUploadTotals**](structQAccelPlot_1_1Internal_1_1DataTextureUploadTotals.md) | [**dataTextureUploadTotals**](#function-datatextureuploadtotals) () <br>_Returns the totals since the last reset. Thread-safe._  |
|  int | [**dataTextureWidth**](#function-datatexturewidth) (const int maxTextureSize) <br>_Returns the floats per data texture row on a GPU with texture limit_ _maxTextureSize_ _._ |
|  const QList&lt; [**UnitPrefix**](structQAccelPlot_1_1Internal_1_1UnitPrefix.md) &gt; & | [**defaultUnitPrefixes**](#function-defaultunitprefixes) () <br>_Returns every power-of-1000 prefix from quecto to quetta, including the bare unit, by ascending exponent._  |
|  qreal | [**distanceToBox**](#function-distancetobox) (qreal x, qreal y, const std::pair&lt; qreal, qreal &gt; & xs, const std::pair&lt; qreal, qreal &gt; & ys) <br>_Returns the pixel distance from (_ _x_ _,__y_ _) to the box spanning__xs_ _and__ys_ _; 0 inside._ |
|  double | [**distanceToInterval**](#function-distancetointerval) (double position, double a, double b) noexcept<br>_Returns the signed distance from_ _position_ _to the closed interval between__a_ _and__b_ _._ |
|  qreal | [**edgePixel**](#function-edgepixel) (double value, const [**Axis**](classQAccelPlot_1_1Axis.md) & axis, qreal length) <br>_Maps rectangle edge_ _value_ _on__axis_ _to item pixels along an item side__length_ _pixels long._ |
|  [**GlowUniforms**](structQAccelPlot_1_1Internal_1_1GlowUniforms.md) | [**glowUniforms**](#function-glowuniforms) (const [**GlowStyle**](structQAccelPlot_1_1Internal_1_1GlowStyle.md) & style) <br>_Returns the uniforms that draw_ _style_ _._ |
|  [**GrabSize**](structQAccelPlot_1_1Internal_1_1GrabSize.md) | [**grabSizeFor**](#function-grabsizefor) (const QSize & pixels, qreal devicePixelRatio) <br>_Returns the grab whose image is closest to_ _pixels_ _in a window at__devicePixelRatio_ _._ |
|  double | [**growBaseline**](#function-growbaseline) (double baseline, const [**Axis**](classQAccelPlot_1_1Axis.md) \* valueAxis) <br>_Returns the value that items drawn from_ _baseline_ _on__valueAxis_ _grow from._ |
|  [**GrowViewport**](structQAccelPlot_1_1Internal_1_1GrowViewport.md) | [**growViewport**](#function-growviewport) (const [**Axis**](classQAccelPlot_1_1Axis.md) \* valueAxis) <br>_Returns the viewport of_ _valueAxis_ _that places grow baselines; without an axis, both edges are 0._ |
|  bool | [**hoverEnabled**](#function-hoverenabled) () <br>_Returns whether series accept hover events:_ `false` _only when_`QACCELPLOT_HOVER_ENABLED` _is 0._ |
|  QByteArray | [**imageFormatForPath**](#function-imageformatforpath) (const QString & path) <br>_Returns the lowercase_ `QImageWriter` _format for__path's_ _suffix, or an empty array when none can write it._ |
|  std::vector&lt; double &gt; | [**interleave**](#function-interleave) (std::initializer\_list&lt; std::reference\_wrapper&lt; const QList&lt; qreal &gt; &gt; &gt; columns) <br>_Interleaves the first values of_ _columns_ _, as many as the shortest column holds, into data points of one value per column._ |
|  std::vector&lt; double &gt; | [**interleave**](#function-interleave) (std::initializer\_list&lt; std::reference\_wrapper&lt; const std::vector&lt; double &gt; &gt; &gt; columns) <br>`std::vector` _overload of_`interleave` _(__columns_ _)._ |
|  [**ItemOpacityUniforms**](structQAccelPlot_1_1Internal_1_1ItemOpacityUniforms.md) | [**itemOpacityUniforms**](#function-itemopacityuniforms) (const [**ItemFade**](structQAccelPlot_1_1Internal_1_1ItemFade.md) & fade, const [**ItemTrail**](structQAccelPlot_1_1Internal_1_1ItemTrail.md) & trail) <br>_Returns the uniforms that fade the items of_ _fade_ _and__trail_ _._ |
|  [**ItemTiming**](structQAccelPlot_1_1Internal_1_1ItemTiming.md) | [**itemTiming**](#function-itemtiming) (const [**TransitionStep**](structQAccelPlot_1_1Internal_1_1TransitionStep.md) & step, int itemCount) <br>_Returns the timing of a group of_ _itemCount_ _items under__step_ _._ |
|  bool | [**keptItemsChanged**](#function-keptitemschanged) (const [**TransitionItems**](structQAccelPlot_1_1Internal_1_1TransitionItems.md) & from, const [**TransitionItems**](structQAccelPlot_1_1Internal_1_1TransitionItems.md) & to, int stride) <br>_Returns_ `true` _when an item below both counts of__from_ _and__to_ _,__stride_ _values each, changes._ |
|  [**UnitPrefix**](structQAccelPlot_1_1Internal_1_1UnitPrefix.md) | [**largestMagnitudePrefix**](#function-largestmagnitudeprefix) (qreal magnitude, const QList&lt; [**UnitPrefix**](structQAccelPlot_1_1Internal_1_1UnitPrefix.md) &gt; & prefixes) <br>_Returns the prefix of_ _prefixes_ _with the largest exponent at which__magnitude_ _is still at least 1._ |
|  std::optional&lt; QPointF &gt; | [**linePathPoint**](#function-linepathpoint) (const [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) & data, int pointCount, int index, int corner, [**StepMode**](namespaceQAccelPlot_1_1StepModeNS.md#enum-mode) mode, bool logScaleX, bool logScaleY) <br>_Returns path point_ _corner_ _of data point__index_ _on the drawn line of__pointCount_ _data points._ |
|  QString | [**localFilePath**](#function-localfilepath) (const QString & pathOrUrl) <br>_Returns_ _pathOrUrl_ _as a local file path; a_`file:` _URL, as a QML file dialog returns, is converted._ |
|  QString | [**logTickLabel**](#function-logticklabel) (qreal value) <br>_Returns a decade_ _value_ _as a superscript power of ten, such as "10²" or "10⁻³"._ |
|  int | [**maxTextureSize**](#function-maxtexturesize) (QQuickWindow \* window) <br>_Returns the largest texture width and height, in pixels, that_ _window's_ _GPU supports._ |
|  QString | [**numericLabel**](#function-numericlabel) (qreal value, qreal tickStep) <br>_Returns_ _value_ _with the decimal places needed to write__tickStep_ _exactly._ |
|  [**ValueRange**](structQAccelPlot_1_1Internal_1_1ValueRange.md) | [**pannedRange**](#function-pannedrange) (const [**ValueRange**](structQAccelPlot_1_1Internal_1_1ValueRange.md) & range, qreal fraction, bool logarithmic) <br>_Returns_ _range_ _shifted by__fraction_ _of its extent; a positive__fraction_ _raises both bounds._ |
|  double | [**placeGrowBaseline**](#function-placegrowbaseline) (double baseline, const [**GrowViewport**](structQAccelPlot_1_1Internal_1_1GrowViewport.md) & viewport) <br>_Returns the value that items drawn from_ _baseline_ _grow from in__viewport_ _._ |
|  [**TransitionSlots**](structQAccelPlot_1_1Internal_1_1TransitionSlots.md) | [**plannedSlots**](#function-plannedslots) (int fromCount, int toCount, bool keptChanged, std::optional&lt; TransitionKind &gt; enter, std::optional&lt; TransitionKind &gt; update, std::optional&lt; TransitionKind &gt; exit) <br>_Returns the slots that animate a change from_ _fromCount_ _to__toCount_ _items._ |
|  [**UnitPrefix**](structQAccelPlot_1_1Internal_1_1UnitPrefix.md) | [**prefixWithExponent**](#function-prefixwithexponent) (int exponent, const QList&lt; [**UnitPrefix**](structQAccelPlot_1_1Internal_1_1UnitPrefix.md) &gt; & prefixes) <br>_Returns the prefix of_ _prefixes_ _whose power of ten is__exponent_ _, or the bare unit when none matches._ |
|  std::vector&lt; [**GradientStopData**](structQAccelPlot_1_1GradientStopData.md) &gt; | [**readEffectStops**](#function-readeffectstops) (const [**Colormap**](classQAccelPlot_1_1Colormap.md) \* colormap, QObject \* gradient) <br>_Returns the color stops of a gradient effect: the_ _colormap_ _ramp when set, otherwise the stops of the QML_`Gradient` ___gradient_ _._ |
|  std::vector&lt; [**GradientStopData**](structQAccelPlot_1_1GradientStopData.md) &gt; | [**readGradientStopList**](#function-readgradientstoplist) (const QVariantList & stopObjects) <br>_Reads a list of stop objects, each exposing_ `position` _and_`color` _, into position order._ |
|  [**OpacityTimeline**](classQAccelPlot_1_1Internal_1_1OpacityTimeline.md) | [**readOpacityAnimations**](#function-readopacityanimations) (const QList&lt; QObject \* &gt; & animations, QStringList \* problems=nullptr) <br>_Reads the Qt Quick animations of_ `"opacity"` _among__animations_ _, which run together, as one timeline._ |
|  int | [**recordCapacity**](#function-recordcapacity) (QQuickWindow \* window, int floatsPerRecord, int vertexBytesPerRecord) <br>_Returns how many records a series draws that reads them from the data texture by a float vertex id._  |
|  void | [**recordDataTextureUpload**](#function-recorddatatextureupload) (std::int64\_t bytes) <br>_Adds one upload of_ _bytes_ _to the totals. Thread-safe._ |
|  bool | [**recordFrameDrawn**](#function-recordframedrawn) (const [**RecordFrame**](structQAccelPlot_1_1Internal_1_1RecordFrame.md) & frame, bool & softwareWarned) <br>_Returns whether_ _frame_ _is drawn; warns once through__softwareWarned_ _when the backend cannot run custom shaders._ |
|  int | [**reservedRecordCount**](#function-reservedrecordcount) (int drawnCount) <br>_Returns how many data points the vertex buffer of a record node holds room for, given_ _drawnCount_ _drawn ones._ |
|  void | [**resetDataTextureUploadTotals**](#function-resetdatatextureuploadtotals) () <br>_Sets the totals back to zero. Thread-safe._  |
|  void | [**resolveGradientValueRange**](#function-resolvegradientvaluerange) (Payload & payload, const qreal dataMin, const qreal dataMax) <br>_Fills each unset gradient value bound of_ _payload_ _from [__dataMin_ _,__dataMax_ _]._ |
|  QSizeF | [**rotatedBoundingSize**](#function-rotatedboundingsize) (const QSizeF & size, qreal degrees) <br>_Returns the size of the box that holds_ _size_ _rotated by__degrees_ _._ |
|  void | [**setImageResolution**](#function-setimageresolution) (QImage & image, qreal dpi) <br>_Sets the resolution stored with_ _image_ _to__dpi_ _dots per inch._ |
|  std::size\_t | [**shortestLength**](#function-shortestlength) (const Lists &... lists) <br>_Returns the size of the shortest of_ _lists_ _._ |
|  const QList&lt; [**UnitPrefix**](structQAccelPlot_1_1Internal_1_1UnitPrefix.md) &gt; & | [**siUnitPrefixes**](#function-siunitprefixes) () <br>_Returns every SI prefix from quecto to quetta, including the bare unit, by ascending exponent._  |
|  [**UnitPrefix**](structQAccelPlot_1_1Internal_1_1UnitPrefix.md) | [**snappedUnitPrefix**](#function-snappedunitprefix) (int exponent, const QList&lt; [**UnitPrefix**](structQAccelPlot_1_1Internal_1_1UnitPrefix.md) &gt; & prefixes) <br>_Returns the prefix of_ _prefixes_ _with the largest exponent not above__exponent_ _._ |
|  QList&lt; [**UnitPrefix**](structQAccelPlot_1_1Internal_1_1UnitPrefix.md) &gt; | [**sortedUnitPrefixes**](#function-sortedunitprefixes) (QList&lt; [**UnitPrefix**](structQAccelPlot_1_1Internal_1_1UnitPrefix.md) &gt; prefixes) <br>_Returns_ _prefixes_ _sorted by ascending exponent, without duplicates._ |
|  std::optional&lt; QPointF &gt; | [**stepCorner**](#function-stepcorner) (const [**CurveDataView**](structQAccelPlot_1_1CurveDataView.md) & data, int from, int to, int corner, [**StepMode**](namespaceQAccelPlot_1_1StepModeNS.md#enum-mode) mode, bool logScaleX, bool logScaleY) <br>_Returns corner_ _corner_ _of the step from data point__from_ _to data point__to_ _, in data space._ |
|  int | [**stepCornerCount**](#function-stepcornercount) ([**StepMode**](namespaceQAccelPlot_1_1StepModeNS.md#enum-mode) mode) noexcept<br>_Returns the number of line path points per data point for_ _mode:_ _the data point and the step corners after it._ |
|  bool | [**supportsCustomShaderRendering**](#function-supportscustomshaderrendering) (const QQuickWindow \* window) <br>_Returns_ `true` _when__window_ _renders through a hardware scene graph backend that runs custom shaders._ |
|  qreal | [**toPrefix**](#function-toprefix) (qreal value, const [**UnitPrefix**](structQAccelPlot_1_1Internal_1_1UnitPrefix.md) & prefix) <br>_Returns_ _value_ _, given in the base unit, expressed in__prefix_ _._ |
|  float | [**unboundedGradientCoordinate**](#function-unboundedgradientcoordinate) (const Qt::Orientation direction, const qreal value, const qreal minimum, const qreal maximum) <br>_Returns an unbounded palette coordinate for a data-space_ _value_ _._ |
|  std::optional&lt; [**UnitPrefix**](structQAccelPlot_1_1Internal_1_1UnitPrefix.md) &gt; | [**unitPrefixFromSymbol**](#function-unitprefixfromsymbol) (const QString & symbol) <br>_Returns the SI prefix written as_ _symbol_ _, or_`std::nullopt` _when__symbol_ _is not an SI prefix._ |
|  QSGGeometryNode \* | [**updateRecordNode**](#function-updaterecordnode) (QSGNode \* oldNode, const [**RecordFrame**](structQAccelPlot_1_1Internal_1_1RecordFrame.md) & frame, const [**DataPointStore**](classQAccelPlot_1_1DataPointStore.md) & store, std::uint64\_t & uploadedRevision, UpdateMaterial && updateMaterial) <br>_Returns the node that draws_ _frame_ _with a__Material_ _, reusing__oldNode_ _, or_`nullptr` _when__frame_ _is not drawn._ |
|  void | [**uploadDataTexture**](#function-uploaddatatexture) (std::unique\_ptr&lt; QSGTexture &gt; & texture, QQuickWindow \* window, const QImage & image) <br>_Uploads an image to the live-data texture using the configured Qt API path._  |
|  void | [**uploadRecordData**](#function-uploadrecorddata) ([**DataTextureMaterial**](classQAccelPlot_1_1DataTextureMaterial.md) & material, const [**RecordFrame**](structQAccelPlot_1_1Internal_1_1RecordFrame.md) & frame, const [**DataPointStore**](classQAccelPlot_1_1DataPointStore.md) & store, std::uint64\_t & uploadedRevision, bool newMaterial) <br>_Uploads the render data of_ _store_ _to__material_ _unless__uploadedRevision_ _is its render revision._ |
|  std::optional&lt; qreal &gt; | [**valueIfSet**](#function-valueifset) (const qreal value) <br>_Returns_ _value_ _, or_`std::nullopt` _when it is NaN, the unset state of an effect property._ |
|  qreal | [**wheelZoomFactor**](#function-wheelzoomfactor) (qreal zoomScaleFactor, bool zoomingIn, qreal steps=1.0) <br>_Returns the extent multiplier for mouse-wheel zoom with a_ _zoomScaleFactor_ _in (0, 1)._ |
|  std::pair&lt; qreal, qreal &gt; | [**widenedSpan**](#function-widenedspan) (qreal a, qreal b, qreal minimumSize) <br>_Returns the pixel span between edges_ _a_ _and__b_ _, widened around its center to at least__minimumSize_ _._ |
|  QString | [**writeImageFile**](#function-writeimagefile) (const QImage & image, const QString & path, const QByteArray & format, int quality) <br>_Writes_ _image_ _to__path_ _in__format_ _and returns an error message, or an empty string on success._ |
|  void | [**writeRectUniforms**](#function-writerectuniforms) ([**RectUbo**](structQAccelPlot_1_1Internal_1_1RectUbo.md) & ubo, const QSGMaterialShader::RenderState & state, const [**RectMaterial**](classQAccelPlot_1_1RectMaterial.md) & material) <br>_Fills_ _ubo_ _from__material_ _and the matrix and opacity of the render__state_ _._ |
|  int | [**writeTransitionFrame**](#function-writetransitionframe) (const [**TransitionFrameInput**](structQAccelPlot_1_1Internal_1_1TransitionFrameInput.md) & input, std::vector&lt; double &gt; & out, [**ItemFade**](structQAccelPlot_1_1Internal_1_1ItemFade.md) & fade) <br>_Writes the frame of_ _input_ _into__out_ _and the items it fades into__fade_ _._ |
|  [**ValueRange**](structQAccelPlot_1_1Internal_1_1ValueRange.md) | [**zoomedRange**](#function-zoomedrange) (const [**ValueRange**](structQAccelPlot_1_1Internal_1_1ValueRange.md) & range, qreal factor, qreal anchorRatio, bool logarithmic) <br>_Returns_ _range_ _with its extent multiplied by__factor_ _, keeping the value at__anchorRatio_ _in place._ |




























## Public Types Documentation





### enum InspectionAxis {#enum-inspectionaxis}

_Data dimension a query measures along, or along which a series' records are ordered._ 
```C++
enum QAccelPlot::Internal::InspectionAxis {
    X,
    Y
};
```




<hr>




### enum InspectionMatching {#enum-inspectionmatching}

_How_ `PlotInspector` _and_`SelectionTool` _match a series' data points to a position._
```C++
enum QAccelPlot::Internal::InspectionMatching {
    None,
    Nearest,
    Containing
};
```




<hr>




### enum TransitionKind {#enum-transitionkind}

_How a transition moves its items: one value per transition class._ 
```C++
enum QAccelPlot::Internal::TransitionKind {
    Morph,
    Draw,
    Grow
};
```




<hr>
## Public Attributes Documentation





### variable kFadeCurveSamples {#variable-kfadecurvesamples}

_Number of opacity samples over the timeline of an item._ 
```C++
int QAccelPlot::Internal::kFadeCurveSamples;
```




<hr>




### variable kFadeGroupCount {#variable-kfadegroupcount}

_Most transition groups that fade at once: the kept items, and the added or removed ones._ 
```C++
int QAccelPlot::Internal::kFadeGroupCount;
```




<hr>




### variable kFadeStartSamples {#variable-kfadestartsamples}

_Number of samples of the start times of the items along an eased reveal front._ 
```C++
int QAccelPlot::Internal::kFadeStartSamples;
```




<hr>




### variable kMaxDataTextureWidth {#variable-kmaxdatatexturewidth}

_Largest number of floats stored per data texture row, one float per RGBA8 texel._ 
```C++
int QAccelPlot::Internal::kMaxDataTextureWidth;
```




<hr>




### variable kTrailEasingSamples {#variable-ktraileasingsamples}

_Number of samples of the_ `FadeTrail` _easing curve._
```C++
int QAccelPlot::Internal::kTrailEasingSamples;
```




<hr>
## Public Functions Documentation





### function allocateRecordVertices {#function-allocaterecordvertices}

_Sizes the vertex buffer of_ _node_ _for_`reservedRecordCount()` _records of__frame_ _._
```C++
void QAccelPlot::Internal::allocateRecordVertices (
    QSGGeometryNode & node,
    const RecordFrame & frame
) 
```




<hr>




### function assignNonNegative {#function-assignnonnegative}

_Stores_ _requested_ _, clamped to 0 or more, in__member_ _and returns_`true` _when that changes it._
```C++
bool QAccelPlot::Internal::assignNonNegative (
    qreal & member,
    qreal requested
) 
```



A non-finite _requested_, or one nearly equal to _member_, leaves _member_ unchanged. 


        

<hr>




### function axisLabelCenter {#function-axislabelcenter}

_Returns the center of the axis label described by_ _params_ _, in item coordinates._
```C++
QPointF QAccelPlot::Internal::axisLabelCenter (
    const AxisLabelLayoutParams & params
) 
```



The rotated text box is kept flush with the axis ends: at fraction 0 or 1 its edge meets the end of `dataRect`. On vertical axes the minimum-value end is the bottom. 


        

<hr>




### function clampAntialiasingFeather {#function-clampantialiasingfeather}

_Returns_ _feather_ _clamped to the [0, 10] pixel range of the series'_`antialiasingFeather` _properties._
```C++
qreal QAccelPlot::Internal::clampAntialiasingFeather (
    qreal feather
) 
```




<hr>




### function compactedFadeGroup {#function-compactedfadegroup}

_Returns_ _group_ _after some of its items are left out of the drawn items; it starts at drawn index__first_ _._
```C++
ItemFadeGroup QAccelPlot::Internal::compactedFadeGroup (
    const ItemFadeGroup & group,
    int first,
    const std::vector< int > & kept
) 
```



_kept_ lists, in order, the positions in _group_ of the remaining items. They keep their start times through `starts`, which holds them at `kFadeStartSamples` evenly spaced shares of the remaining items; items between two samples start at an interpolated time. 


        

<hr>




### function compareGlowStyles {#function-compareglowstyles}

_Orders_ _left_ _and__right_ _for_`QSGMaterial::compare()` _: negative, 0, or positive._
```C++
int QAccelPlot::Internal::compareGlowStyles (
    const GlowStyle & left,
    const GlowStyle & right
) noexcept
```




<hr>




### function compareItemFades {#function-compareitemfades}

_Orders_ _left_ _and__right_ _for_`QSGMaterial::compare()` _: negative, 0, or positive._
```C++
int QAccelPlot::Internal::compareItemFades (
    const ItemFade & left,
    const ItemFade & right
) noexcept
```




<hr>




### function compareItemTrails {#function-compareitemtrails}

_Orders_ _left_ _and__right_ _for_`QSGMaterial::compare()` _: negative, 0, or positive._
```C++
int QAccelPlot::Internal::compareItemTrails (
    const ItemTrail & left,
    const ItemTrail & right
) noexcept
```




<hr>




### function createRecordNode {#function-createrecordnode}

_Returns an empty triangle node of_ _attributes_ _that owns its geometry and__material_ _._
```C++
QSGGeometryNode * QAccelPlot::Internal::createRecordNode (
    const QSGGeometry::AttributeSet & attributes,
    QSGMaterial * material
) 
```



For series whose shader derives every vertex from its index and reads the data points from the data texture. The vertex buffer is in static GPU memory, as it only changes with `reservedRecordCount()`. 


        

<hr>




### function dataTextureHeight {#function-datatextureheight}

_Returns the number of rows of_ _width_ _floats needed to store__floatCount_ _floats._
```C++
int QAccelPlot::Internal::dataTextureHeight (
    const int floatCount,
    const int width
) 
```




<hr>




### function dataTextureItemCapacity {#function-datatextureitemcapacity}

_Returns how many items of_ _floatsPerItem_ _floats fit in a data texture on a GPU with texture limit__maxTextureSize_ _._
```C++
qint64 QAccelPlot::Internal::dataTextureItemCapacity (
    const int maxTextureSize,
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




### function dataTextureWidth {#function-datatexturewidth}

_Returns the floats per data texture row on a GPU with texture limit_ _maxTextureSize_ _._
```C++
int QAccelPlot::Internal::dataTextureWidth (
    const int maxTextureSize
) 
```



data\_texture.glsl reads the width from the bound texture, so it follows this value. 


        

<hr>




### function defaultUnitPrefixes {#function-defaultunitprefixes}

_Returns every power-of-1000 prefix from quecto to quetta, including the bare unit, by ascending exponent._ 
```C++
const QList< UnitPrefix > & QAccelPlot::Internal::defaultUnitPrefixes () 
```




<hr>




### function distanceToBox {#function-distancetobox}

_Returns the pixel distance from (_ _x_ _,__y_ _) to the box spanning__xs_ _and__ys_ _; 0 inside._
```C++
qreal QAccelPlot::Internal::distanceToBox (
    qreal x,
    qreal y,
    const std::pair< qreal, qreal > & xs,
    const std::pair< qreal, qreal > & ys
) 
```




<hr>




### function distanceToInterval {#function-distancetointerval}

_Returns the signed distance from_ _position_ _to the closed interval between__a_ _and__b_ _._
```C++
double QAccelPlot::Internal::distanceToInterval (
    double position,
    double a,
    double b
) noexcept
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



Infinite edges, and non-positive edges on a log axis, map to infinity on the matching side, as axis\_pixel.glsl extends them past the plot edge. 


        

<hr>




### function glowUniforms {#function-glowuniforms}

_Returns the uniforms that draw_ _style_ _._
```C++
GlowUniforms QAccelPlot::Internal::glowUniforms (
    const GlowStyle & style
) 
```




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




### function growBaseline {#function-growbaseline}

_Returns the value that items drawn from_ _baseline_ _on__valueAxis_ _grow from._
```C++
double QAccelPlot::Internal::growBaseline (
    double baseline,
    const Axis * valueAxis
) 
```



It is _baseline_, or the viewport edge the items are drawn from when _valueAxis_ cannot place it: an infinite baseline, or one at or below 0 on a log axis. Without an axis, an infinite baseline gives 0. 


        

<hr>




### function growViewport {#function-growviewport}

_Returns the viewport of_ _valueAxis_ _that places grow baselines; without an axis, both edges are 0._
```C++
GrowViewport QAccelPlot::Internal::growViewport (
    const Axis * valueAxis
) 
```




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




### function interleave {#function-interleave}

_Interleaves the first values of_ _columns_ _, as many as the shortest column holds, into data points of one value per column._
```C++
std::vector< double > QAccelPlot::Internal::interleave (
    std::initializer_list< std::reference_wrapper< const QList< qreal > > > columns
) 
```




<hr>




### function interleave {#function-interleave}

`std::vector` _overload of_`interleave` _(__columns_ _)._
```C++
std::vector< double > QAccelPlot::Internal::interleave (
    std::initializer_list< std::reference_wrapper< const std::vector< double > > > columns
) 
```




<hr>




### function itemOpacityUniforms {#function-itemopacityuniforms}

_Returns the uniforms that fade the items of_ _fade_ _and__trail_ _._
```C++
ItemOpacityUniforms QAccelPlot::Internal::itemOpacityUniforms (
    const ItemFade & fade,
    const ItemTrail & trail
) 
```




<hr>




### function itemTiming {#function-itemtiming}

_Returns the timing of a group of_ _itemCount_ _items under__step_ _._
```C++
ItemTiming QAccelPlot::Internal::itemTiming (
    const TransitionStep & step,
    int itemCount
) 
```




<hr>




### function keptItemsChanged {#function-keptitemschanged}

_Returns_ `true` _when an item below both counts of__from_ _and__to_ _,__stride_ _values each, changes._
```C++
bool QAccelPlot::Internal::keptItemsChanged (
    const TransitionItems & from,
    const TransitionItems & to,
    int stride
) 
```



Invalid values that stay invalid do not count as a change. 


        

<hr>




### function largestMagnitudePrefix {#function-largestmagnitudeprefix}

_Returns the prefix of_ _prefixes_ _with the largest exponent at which__magnitude_ _is still at least 1._
```C++
UnitPrefix QAccelPlot::Internal::largestMagnitudePrefix (
    qreal magnitude,
    const QList< UnitPrefix > & prefixes
) 
```



_prefixes_ must be sorted by ascending exponent. Falls back to the smallest prefix when none qualifies, and to the bare unit when _prefixes_ is empty. A zero or non-finite _magnitude_ is treated as 1. 


        

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




### function logTickLabel {#function-logticklabel}

_Returns a decade_ _value_ _as a superscript power of ten, such as "10²" or "10⁻³"._
```C++
QString QAccelPlot::Internal::logTickLabel (
    qreal value
) 
```



A value that is not a positive power of ten falls back to `numericLabel(value, value)`. 


        

<hr>




### function maxTextureSize {#function-maxtexturesize}

_Returns the largest texture width and height, in pixels, that_ _window's_ _GPU supports._
```C++
int QAccelPlot::Internal::maxTextureSize (
    QQuickWindow * window
) 
```



Falls back to 8192 when the RHI cannot be queried (Qt older than 6.6, or a build without the private Qt API). `QACCELPLOT_MAX_TEXTURE_SIZE` lowers the result, for example to exercise capacity limits in tests. 


        

<hr>




### function numericLabel {#function-numericlabel}

_Returns_ _value_ _with the decimal places needed to write__tickStep_ _exactly._
```C++
QString QAccelPlot::Internal::numericLabel (
    qreal value,
    qreal tickStep
) 
```



Steps 1, 2, 5, and 10 get no decimals, 0.5 and 0.1 get one, 0.25 and 0.05 get two. A non-finite or non-positive step gets one. A value that rounds to zero is written without a minus sign. 


        

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



With _logarithmic_ set, the shift is applied in `log10` space, so both bounds are scaled by the same ratio. A range that is not positive is shifted linearly instead. Returns _range_ unchanged when a bound would leave the range of a double or both bounds would round to the same value. 


        

<hr>




### function placeGrowBaseline {#function-placegrowbaseline}

_Returns the value that items drawn from_ _baseline_ _grow from in__viewport_ _._
```C++
double QAccelPlot::Internal::placeGrowBaseline (
    double baseline,
    const GrowViewport & viewport
) 
```



It is _baseline_, or the viewport edge the items are drawn from when the axis cannot place it: an infinite baseline, or one at or below 0 on a log axis. 


        

<hr>




### function plannedSlots {#function-plannedslots}

_Returns the slots that animate a change from_ _fromCount_ _to__toCount_ _items._
```C++
TransitionSlots QAccelPlot::Internal::plannedSlots (
    int fromCount,
    int toCount,
    bool keptChanged,
    std::optional< TransitionKind > enter,
    std::optional< TransitionKind > update,
    std::optional< TransitionKind > exit
) 
```



Each kind is that of the slot's enabled transition, or `std::nullopt` without one. The update slot runs only when _keptChanged:_ a change that only adds or removes items leaves the kept ones alone. A `DrawTransition` or `GrowTransition` in the update slot redraws all items, so only the update slot runs. 


        

<hr>




### function prefixWithExponent {#function-prefixwithexponent}

_Returns the prefix of_ _prefixes_ _whose power of ten is__exponent_ _, or the bare unit when none matches._
```C++
UnitPrefix QAccelPlot::Internal::prefixWithExponent (
    int exponent,
    const QList< UnitPrefix > & prefixes
) 
```




<hr>




### function readEffectStops {#function-readeffectstops}

_Returns the color stops of a gradient effect: the_ _colormap_ _ramp when set, otherwise the stops of the QML_`Gradient` ___gradient_ _._
```C++
std::vector< GradientStopData > QAccelPlot::Internal::readEffectStops (
    const Colormap * colormap,
    QObject * gradient
) 
```



_gradient_ stops are read from its `stops` list property, falling back to child objects, and normalized like `readGradientStopList()`. Returns an empty vector when both are null.




**See also:** [**GradientFill**](classQAccelPlot_1_1GradientFill.md), [**GradientStroke**](classQAccelPlot_1_1GradientStroke.md) 



        

<hr>




### function readGradientStopList {#function-readgradientstoplist}

_Reads a list of stop objects, each exposing_ `position` _and_`color` _, into position order._
```C++
std::vector< GradientStopData > QAccelPlot::Internal::readGradientStopList (
    const QVariantList & stopObjects
) 
```



A single stop is duplicated, and the first and last colors are extended to positions 0 and 1. Entries that are not stop-like or have a NaN position are skipped.




**See also:** [**Colormap**](classQAccelPlot_1_1Colormap.md) 



        

<hr>




### function readOpacityAnimations {#function-readopacityanimations}

_Reads the Qt Quick animations of_ `"opacity"` _among__animations_ _, which run together, as one timeline._
```C++
OpacityTimeline QAccelPlot::Internal::readOpacityAnimations (
    const QList< QObject * > & animations,
    QStringList * problems=nullptr
) 
```



The animations are read, never run. `PropertyAnimation` and its numeric subtypes, `PropertyAction`, `PauseAnimation`, `SequentialAnimation`, and `ParallelAnimation` are supported, with `loops`. A missing `from` continues from the opacity reached so far, and a missing `to` is 1. Other animations, and animations of other properties, add only their duration.




**Parameters:**


* `problems` When not null, receives one message for each part of _animations_ that is ignored. 




        

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




### function recordFrameDrawn {#function-recordframedrawn}

_Returns whether_ _frame_ _is drawn; warns once through__softwareWarned_ _when the backend cannot run custom shaders._
```C++
bool QAccelPlot::Internal::recordFrameDrawn (
    const RecordFrame & frame,
    bool & softwareWarned
) 
```




<hr>




### function reservedRecordCount {#function-reservedrecordcount}

_Returns how many data points the vertex buffer of a record node holds room for, given_ _drawnCount_ _drawn ones._
```C++
int QAccelPlot::Internal::reservedRecordCount (
    int drawnCount
) 
```



It is the next power of two, at least 256, so appending one data point at a time reallocates the buffer only when the count crosses one. 


        

<hr>




### function resetDataTextureUploadTotals {#function-resetdatatextureuploadtotals}

_Sets the totals back to zero. Thread-safe._ 
```C++
void QAccelPlot::Internal::resetDataTextureUploadTotals () 
```




<hr>




### function resolveGradientValueRange {#function-resolvegradientvaluerange}

_Fills each unset gradient value bound of_ _payload_ _from [__dataMin_ _,__dataMax_ _]._
```C++
template<typename Payload>
void QAccelPlot::Internal::resolveGradientValueRange (
    Payload & payload,
    const qreal dataMin,
    const qreal dataMax
) 
```



Bounds resolve independently, so a set bound is kept when the other bound is unset. 


        

<hr>




### function rotatedBoundingSize {#function-rotatedboundingsize}

_Returns the size of the box that holds_ _size_ _rotated by__degrees_ _._
```C++
QSizeF QAccelPlot::Internal::rotatedBoundingSize (
    const QSizeF & size,
    qreal degrees
) 
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




### function shortestLength {#function-shortestlength}

_Returns the size of the shortest of_ _lists_ _._
```C++
template<typename... Lists>
std::size_t QAccelPlot::Internal::shortestLength (
    const Lists &... lists
) 
```




<hr>




### function siUnitPrefixes {#function-siunitprefixes}

_Returns every SI prefix from quecto to quetta, including the bare unit, by ascending exponent._ 
```C++
const QList< UnitPrefix > & QAccelPlot::Internal::siUnitPrefixes () 
```




<hr>




### function snappedUnitPrefix {#function-snappedunitprefix}

_Returns the prefix of_ _prefixes_ _with the largest exponent not above__exponent_ _._
```C++
UnitPrefix QAccelPlot::Internal::snappedUnitPrefix (
    int exponent,
    const QList< UnitPrefix > & prefixes
) 
```



_prefixes_ must be sorted by ascending exponent. Falls back to the smallest prefix when none qualifies, and to the bare unit when _prefixes_ is empty. 


        

<hr>




### function sortedUnitPrefixes {#function-sortedunitprefixes}

_Returns_ _prefixes_ _sorted by ascending exponent, without duplicates._
```C++
QList< UnitPrefix > QAccelPlot::Internal::sortedUnitPrefixes (
    QList< UnitPrefix > prefixes
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




### function toPrefix {#function-toprefix}

_Returns_ _value_ _, given in the base unit, expressed in__prefix_ _._
```C++
qreal QAccelPlot::Internal::toPrefix (
    qreal value,
    const UnitPrefix & prefix
) 
```




<hr>




### function unboundedGradientCoordinate {#function-unboundedgradientcoordinate}

_Returns an unbounded palette coordinate for a data-space_ _value_ _._
```C++
inline float QAccelPlot::Internal::unboundedGradientCoordinate (
    const Qt::Orientation direction,
    const qreal value,
    const qreal minimum,
    const qreal maximum
) 
```



Coordinates outside [0, 1] are intentionally preserved for interpolation. Renderers must clamp only after interpolation, immediately before sampling the gradient, so values beyond the configured range retain endpoint colors. 


        

<hr>




### function unitPrefixFromSymbol {#function-unitprefixfromsymbol}

_Returns the SI prefix written as_ _symbol_ _, or_`std::nullopt` _when__symbol_ _is not an SI prefix._
```C++
std::optional< UnitPrefix > QAccelPlot::Internal::unitPrefixFromSymbol (
    const QString & symbol
) 
```



An empty _symbol_ is the bare unit. "µ" (U+00B5), "μ" (U+03BC), and "u" all give micro with the symbol "µ" (U+00B5). 


        

<hr>




### function updateRecordNode {#function-updaterecordnode}

_Returns the node that draws_ _frame_ _with a__Material_ _, reusing__oldNode_ _, or_`nullptr` _when__frame_ _is not drawn._
```C++
template<typename Material, typename UpdateMaterial>
QSGGeometryNode * QAccelPlot::Internal::updateRecordNode (
    QSGNode * oldNode,
    const RecordFrame & frame,
    const DataPointStore & store,
    std::uint64_t & uploadedRevision,
    UpdateMaterial && updateMaterial
) 
```



Deletes _oldNode_ when returning `nullptr`. Otherwise sizes the vertex buffer, uploads the render data of _store_ when it changed since _uploadedRevision_, and passes the material to _updateMaterial_ to set its uniforms. 


        

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




### function uploadRecordData {#function-uploadrecorddata}

_Uploads the render data of_ _store_ _to__material_ _unless__uploadedRevision_ _is its render revision._
```C++
void QAccelPlot::Internal::uploadRecordData (
    DataTextureMaterial & material,
    const RecordFrame & frame,
    const DataPointStore & store,
    std::uint64_t & uploadedRevision,
    bool newMaterial
) 
```



Set _newMaterial_ for a material that has never been uploaded to. 


        

<hr>




### function valueIfSet {#function-valueifset}

_Returns_ _value_ _, or_`std::nullopt` _when it is NaN, the unset state of an effect property._
```C++
inline std::optional< qreal > QAccelPlot::Internal::valueIfSet (
    const qreal value
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



Matches `widenedSpan()` in axis\_pixel.glsl, so hit tests use the drawn size. 


        

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




### function writeTransitionFrame {#function-writetransitionframe}

_Writes the frame of_ _input_ _into__out_ _and the items it fades into__fade_ _._
```C++
int QAccelPlot::Internal::writeTransitionFrame (
    const TransitionFrameInput & input,
    std::vector< double > & out,
    ItemFade & fade
) 
```



The frame holds the kept items, then the shown added or removed items. The first group of _fade_ describes the update slot, the second the enter or exit slot. 

**Returns:**

The number of items in the frame. 





        

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



_anchorRatio_ is a position inside the range: 0 at `min` and 1 at `max`. With _logarithmic_ set, the extent is measured in `log10` space. A range that is not positive is zoomed linearly instead. Returns _range_ unchanged when a bound would leave the range of a double: it would not be finite, or in `log10` space it would reach zero. Also returns _range_ unchanged when a zoom-in would shrink the extent below 1e-12 of the larger bound, where double precision would soon collapse the bounds and keep a zoom-out from moving them. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/axis/internal/AxisLabelLayout.hpp`

