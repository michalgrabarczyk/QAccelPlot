








# Class QAccelPlot::RectangleSeries



[**ClassList**](annotated.md) **>** [**QAccelPlot**](namespaceQAccelPlot.md) **>** [**RectangleSeries**](classQAccelPlot_1_1RectangleSeries.md)



_A hardware-accelerated QML item that renders a large list of axis-aligned rectangles._ [More...](#detailed-description)

* `#include <RectangleSeries.hpp>`



Inherits the following classes: [QAccelPlot::PlotSeries](classQAccelPlot_1_1PlotSeries.md)




## Inheritance diagram

```mermaid
flowchart TB
  classQAccelPlot_1_1RectangleSeries["QAccelPlot::RectangleSeries"]

  classQAccelPlot_1_1PlotSeries["QAccelPlot::PlotSeries"]
  classQAccelPlot_1_1PlotSeries --> classQAccelPlot_1_1RectangleSeries
  click classQAccelPlot_1_1PlotSeries "../classQAccelPlot_1_1PlotSeries/" "Open QAccelPlot::PlotSeries"

  external_base_classQAccelPlot_1_1PlotSeries_1["QQuickItem"]
  external_base_classQAccelPlot_1_1PlotSeries_1 --> classQAccelPlot_1_1PlotSeries

```














## Public Types inherited from QAccelPlot::PlotSeries

See [QAccelPlot::PlotSeries](classQAccelPlot_1_1PlotSeries.md)

| Type | Name |
| ---: | :--- |
| enum  | [**LegendSymbol**](classQAccelPlot_1_1PlotSeries.md#enum-legendsymbol)  <br>_Supported default legend symbols._  |






















## Public Properties

| Type | Name |
| ---: | :--- |
| property [**Border**](classQAccelPlot_1_1Border.md) \* | [**border**](classQAccelPlot_1_1RectangleSeries.md#property-border-12)  <br>_Outline inside each rectangle's edges, e.g._ `border.width: 1` _. Default: black, width 0._ |
| property QList&lt; QColor &gt; | [**categoryColors**](classQAccelPlot_1_1RectangleSeries.md#property-categorycolors-12)  <br>_Fill colors indexed by each rectangle's_ `category` _. Default: empty._ |
| property QColor | [**color**](classQAccelPlot_1_1RectangleSeries.md#property-color-12)  <br>_Fill color of rectangles without a category color. Default:_ `Colors.dark.seriesPrimary` _with alpha 50._ |
| property QColor | [**hoverColor**](classQAccelPlot_1_1RectangleSeries.md#property-hovercolor-12)  <br>_Fill color of the rectangle under the cursor. Default: an invalid color, no highlight._  |
| property int | [**hoveredIndex**](classQAccelPlot_1_1RectangleSeries.md#property-hoveredindex-12)  <br>_Read-only: index of the rectangle under the cursor, or -1 when none._  |
| property qreal | [**minimumHeight**](classQAccelPlot_1_1RectangleSeries.md#property-minimumheight-12)  <br>_Minimum drawn height in pixels, like_ `minimumWidth` _. Default: 1._ |
| property qreal | [**minimumWidth**](classQAccelPlot_1_1RectangleSeries.md#property-minimumwidth-12)  <br>_Minimum drawn width in pixels, so narrow rectangles stay visible when zoomed out. Default: 1._  |


## Public Properties inherited from QAccelPlot::PlotSeries

See [QAccelPlot::PlotSeries](classQAccelPlot_1_1PlotSeries.md)

| Type | Name |
| ---: | :--- |
| property int | [**count**](classQAccelPlot_1_1PlotSeries.md#property-count-12)  <br>_Read-only: number of records stored, including invalid ones._  |
| property quint64 | [**dataRevision**](classQAccelPlot_1_1PlotSeries.md#property-datarevision-12)  <br>_Revision incremented by every accepted change of the series' records._  |
| property bool | [**hoverEnabled**](classQAccelPlot_1_1PlotSeries.md#property-hoverenabled-12)  <br>_Whether the series reacts to the mouse cursor. Default:_ `true` _._ |
| property bool | [**hovered**](classQAccelPlot_1_1PlotSeries.md#property-hovered-12)  <br>_Read-only:_ `true` _while the mouse cursor is over the series._ |
| property [**QAccelPlot::SeriesInspection**](classQAccelPlot_1_1SeriesInspection.md) \* | [**inspection**](classQAccelPlot_1_1PlotSeries.md#property-inspection-12)  <br>_Read-only constant: data queries for this series._  |
| property [**LegendSymbol**](classQAccelPlot_1_1PlotSeries.md#enum-legendsymbol) | [**legendSymbol**](classQAccelPlot_1_1PlotSeries.md#property-legendsymbol-12)  <br>_Symbol style requested from the default legend._  |
| property QString | [**name**](classQAccelPlot_1_1PlotSeries.md#property-name-12)  <br>_Identifying name used by the default legend._  |
| property QRectF | [**plotRect**](classQAccelPlot_1_1PlotSeries.md#property-plotrect-12)  <br>_Plot area in parent-item coordinates, assigned by_ `PlotView` _._ |
| property [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**xAxis**](classQAccelPlot_1_1PlotSeries.md#property-xaxis-12)  <br>_Horizontal axis used for data-to-pixel coordinate mapping._  |
| property [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**yAxis**](classQAccelPlot_1_1PlotSeries.md#property-yaxis-12)  <br>_Vertical axis used for data-to-pixel coordinate mapping._  |






## Public Signals

| Type | Name |
| ---: | :--- |
| signal void | [**categoryColorsChanged**](classQAccelPlot_1_1RectangleSeries.md#signal-categorycolorschanged)  <br>_Emitted when the categoryColors property changes._  |
| signal void | [**colorChanged**](classQAccelPlot_1_1RectangleSeries.md#signal-colorchanged)  <br>_Emitted when the color property changes._  |
| signal void | [**hoverColorChanged**](classQAccelPlot_1_1RectangleSeries.md#signal-hovercolorchanged)  <br>_Emitted when the hoverColor property changes._  |
| signal void | [**hoveredIndexChanged**](classQAccelPlot_1_1RectangleSeries.md#signal-hoveredindexchanged)  <br>_Emitted when the hovered rectangle index changes._  |
| signal void | [**minimumHeightChanged**](classQAccelPlot_1_1RectangleSeries.md#signal-minimumheightchanged)  <br>_Emitted when the minimumHeight property changes._  |
| signal void | [**minimumWidthChanged**](classQAccelPlot_1_1RectangleSeries.md#signal-minimumwidthchanged)  <br>_Emitted when the minimumWidth property changes._  |


## Public Signals inherited from QAccelPlot::PlotSeries

See [QAccelPlot::PlotSeries](classQAccelPlot_1_1PlotSeries.md)

| Type | Name |
| ---: | :--- |
| signal void | [**countChanged**](classQAccelPlot_1_1PlotSeries.md#signal-countchanged)  <br>_Emitted when the number of records changes._  |
| signal void | [**dataRevisionChanged**](classQAccelPlot_1_1PlotSeries.md#signal-datarevisionchanged)  <br>_Emitted when the dataRevision property changes._  |
| signal void | [**hoverEnabledChanged**](classQAccelPlot_1_1PlotSeries.md#signal-hoverenabledchanged)  <br>_Emitted when the hoverEnabled property changes._  |
| signal void | [**hoveredChanged**](classQAccelPlot_1_1PlotSeries.md#signal-hoveredchanged)  <br>_Emitted when the hovered property changes._  |
| signal void | [**legendEntryChanged**](classQAccelPlot_1_1PlotSeries.md#signal-legendentrychanged)  <br>_Emitted when the result of_ `legendEntry()` _changes._ |
| signal void | [**legendSymbolChanged**](classQAccelPlot_1_1PlotSeries.md#signal-legendsymbolchanged)  <br> |
| signal void | [**nameChanged**](classQAccelPlot_1_1PlotSeries.md#signal-namechanged)  <br> |
| signal void | [**plotRectChanged**](classQAccelPlot_1_1PlotSeries.md#signal-plotrectchanged)  <br> |
| signal void | [**xAxisChanged**](classQAccelPlot_1_1PlotSeries.md#signal-xaxischanged)  <br> |
| signal void | [**yAxisChanged**](classQAccelPlot_1_1PlotSeries.md#signal-yaxischanged)  <br> |






## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**RectangleSeries**](#function-rectangleseries) (QQuickItem \* parent=nullptr) <br>_Constructs a_ [_**RectangleSeries**_](classQAccelPlot_1_1RectangleSeries.md) _with the given__parent_ _._ |
|  [**Border**](classQAccelPlot_1_1Border.md) \* | [**border**](#function-border-22) () const<br>_Returns the grouped outline settings. The object is owned by the series._  |
|  QList&lt; QColor &gt; | [**categoryColors**](#function-categorycolors-22) () const<br>_Returns the fill colors indexed by category._  |
| virtual Q\_INVOKABLE void | [**clearData**](#function-cleardata) () override<br>_Removes all rectangles._  |
|  QColor | [**color**](#function-color-22) () const<br>_Returns the rectangle fill color._  |
|  bool | [**contains**](#function-contains) (const QPointF & point) override const<br>_Returns_ `true` _when a rectangle lies under item position__point_ _._ |
|  QColor | [**hoverColor**](#function-hovercolor-22) () const<br>_Returns the fill color of the hovered rectangle._  |
|  int | [**hoveredIndex**](#function-hoveredindex-22) () const<br>_Returns the index of the hovered rectangle, or -1 if none._  |
| virtual std::optional&lt; [**LegendEntry**](structQAccelPlot_1_1LegendEntry.md) &gt; | [**legendEntry**](#function-legendentry) () override const<br>_Returns the legend entry: the symbol and color of the rectangles._  |
|  qreal | [**minimumHeight**](#function-minimumheight-22) () const<br>_Returns the minimum drawn height in pixels._  |
|  qreal | [**minimumWidth**](#function-minimumwidth-22) () const<br>_Returns the minimum drawn width in pixels._  |
|  void | [**postData**](#function-postdata-14) (std::vector&lt; double &gt; && data, std::vector&lt; int &gt; && categories, int rectCount) <br>_Thread-safe: queues_ `setData` _(__data_ _,__categories_ _,__rectCount_ _) to the item's thread._ |
|  void | [**postData**](#function-postdata-24) (std::vector&lt; float &gt; && data, std::vector&lt; int &gt; && categories, int rectCount) <br>_Thread-safe: queues_ `setDataF` _(__data_ _,__categories_ _,__rectCount_ _) to the item's thread._ |
|  void | [**postData**](#function-postdata-34) (std::vector&lt; double &gt; && data, int count) <br>_Queues a moved double buffer for assignment on the series' thread._  |
|  void | [**postData**](#function-postdata-44) (std::vector&lt; float &gt; && data, int count) <br>_Queues a moved float buffer for assignment on the series' thread._  |
|  Q\_INVOKABLE QVariantMap | [**rectangleAt**](#function-rectangleat) (int index) const<br>_Returns rectangle_ _index_ _as an object with_`x1` _,_`y1` _,_`x2` _,_`y2` _properties._ |
|  int | [**rectangleIndexAt**](#function-rectangleindexat) (const QPointF & position) const<br>_Returns the index of the topmost rectangle under item position_ _position_ _, or -1._ |
|  Q\_INVOKABLE void | [**setCategories**](#function-setcategories) (const QList&lt; int &gt; & categories) <br>_Sets one category per rectangle. An empty list clears categories; any other size must equal_ `count` _._ |
|  void | [**setCategoryColors**](#function-setcategorycolors) (const QList&lt; QColor &gt; & colors) <br>_Sets the fill colors indexed by category to_ _colors_ _._ |
|  void | [**setColor**](#function-setcolor) (const QColor & color) <br>_Sets the fill color to_ _color_ _._ |
|  Q\_INVOKABLE void | [**setData**](#function-setdata-16) (const QVariantList & rects) <br>_Loads rectangles from_ _rects_ _, a QML list of objects with_`x1` _,_`y1` _,_`x2` _,_`y2` _properties._ |
|  void | [**setData**](#function-setdata-26) (std::vector&lt; double &gt; && data, std::vector&lt; int &gt; && categories, int rectCount) <br>_Moves_ _data_ _and per-rectangle__categories_ _(empty, or exactly__rectCount_ _) into the series._ |
|  void | [**setData**](#function-setdata-36) (const double \* data, int count) <br>_Replaces the series data with_ _count_ _data points copied from an interleaved double array._ |
|  void | [**setData**](#function-setdata-46) (std::vector&lt; double &gt; && data, int count) <br>_Replaces the series data by moving an interleaved double buffer._  |
|  void | [**setData**](#function-setdata-56) (const double \* data, int count, const [**DataBounds**](structQAccelPlot_1_1PlotSeries_1_1DataBounds.md) & bounds) <br>_Like_ `setData` _(__data_ _,__count_ _), with the data extents given as__bounds_ _instead of scanned for._ |
|  void | [**setData**](#function-setdata-66) (std::vector&lt; double &gt; && data, int count, const [**DataBounds**](structQAccelPlot_1_1PlotSeries_1_1DataBounds.md) & bounds) <br>_Like_ `setData` _(__data_ _,__count_ _), with the data extents given as__bounds_ _instead of scanned for._ |
|  void | [**setDataF**](#function-setdataf-15) (std::vector&lt; float &gt; && data, std::vector&lt; int &gt; && categories, int rectCount) <br>_Like_ `setDataF` _(__data_ _,__rectCount_ _) and also moves per-rectangle__categories_ _(empty, or exactly__rectCount_ _) into the series._ |
|  void | [**setDataF**](#function-setdataf-25) (const float \* data, int count) <br>_Replaces the series data with_ _count_ _data points copied from an interleaved float array._ |
|  void | [**setDataF**](#function-setdataf-35) (std::vector&lt; float &gt; && data, int count) <br>_Replaces the series data by moving an interleaved float buffer._  |
|  void | [**setDataF**](#function-setdataf-45) (const float \* data, int count, const [**DataBounds**](structQAccelPlot_1_1PlotSeries_1_1DataBounds.md) & bounds) <br>_Like_ `setDataF` _(__data_ _,__count_ _), with the data extents given as__bounds_ _instead of scanned for._ |
|  void | [**setDataF**](#function-setdataf-55) (std::vector&lt; float &gt; && data, int count, const [**DataBounds**](structQAccelPlot_1_1PlotSeries_1_1DataBounds.md) & bounds) <br>_Like_ `setDataF` _(__data_ _,__count_ _), with the data extents given as__bounds_ _instead of scanned for._ |
|  void | [**setHoverColor**](#function-sethovercolor) (const QColor & color) <br>_Sets the fill color of the hovered rectangle to_ _color_ _. An invalid color disables the highlight._ |
|  void | [**setMinimumHeight**](#function-setminimumheight) (qreal height) <br>_Sets the minimum drawn height to_ _height_ _pixels. Negative values are clamped to 0._ |
|  void | [**setMinimumWidth**](#function-setminimumwidth) (qreal width) <br>_Sets the minimum drawn width to_ _width_ _pixels. Negative values are clamped to 0._ |
|   | [**~RectangleSeries**](#function-rectangleseries) () override<br>_Destroys the rectangle series._  |


## Public Functions inherited from QAccelPlot::PlotSeries

See [QAccelPlot::PlotSeries](classQAccelPlot_1_1PlotSeries.md)

| Type | Name |
| ---: | :--- |
|   | [**PlotSeries**](classQAccelPlot_1_1PlotSeries.md#function-plotseries-12) (QQuickItem \* parent=nullptr) <br> |
| virtual void | [**clearData**](classQAccelPlot_1_1PlotSeries.md#function-cleardata) () <br>_Removes all data points from the series._  |
| virtual int | [**count**](classQAccelPlot_1_1PlotSeries.md#function-count-22) () const<br>_Returns the number of records stored. The default is the number of stored data points._  |
|  quint64 | [**dataRevision**](classQAccelPlot_1_1PlotSeries.md#function-datarevision-22) () const<br>_Returns the current data revision._  |
|  bool | [**hoverEnabled**](classQAccelPlot_1_1PlotSeries.md#function-hoverenabled-22) () const<br>_Returns whether the series reacts to the mouse cursor._  |
|  bool | [**hovered**](classQAccelPlot_1_1PlotSeries.md#function-hovered-22) () const<br>_Returns whether the mouse cursor is over the series._  |
|  [**SeriesInspection**](classQAccelPlot_1_1SeriesInspection.md) \* | [**inspection**](classQAccelPlot_1_1PlotSeries.md#function-inspection-22) () const<br>_Returns the data queries for this series; created on first use and owned by the series._  |
| virtual std::optional&lt; [**LegendEntry**](structQAccelPlot_1_1LegendEntry.md) &gt; | [**legendEntry**](classQAccelPlot_1_1PlotSeries.md#function-legendentry) () const<br>_Returns how a legend draws the series, or_ `std::nullopt` _for no legend entry. Default: no entry._ |
|  [**LegendSymbol**](classQAccelPlot_1_1PlotSeries.md#enum-legendsymbol) | [**legendSymbol**](classQAccelPlot_1_1PlotSeries.md#function-legendsymbol-22) () const<br> |
|  QString | [**name**](classQAccelPlot_1_1PlotSeries.md#function-name-22) () const<br> |
|  QRectF | [**plotRect**](classQAccelPlot_1_1PlotSeries.md#function-plotrect-22) () const<br> |
|  void | [**postData**](classQAccelPlot_1_1PlotSeries.md#function-postdata-12) (std::vector&lt; double &gt; && data, int count) <br>_Queues a moved double buffer for assignment on the series' thread._  |
|  void | [**postData**](classQAccelPlot_1_1PlotSeries.md#function-postdata-22) (std::vector&lt; float &gt; && data, int count) <br>_Queues a moved float buffer for assignment on the series' thread._  |
|  void | [**setData**](classQAccelPlot_1_1PlotSeries.md#function-setdata-14) (const double \* data, int count) <br>_Replaces the series data with_ _count_ _data points copied from an interleaved double array._ |
|  void | [**setData**](classQAccelPlot_1_1PlotSeries.md#function-setdata-24) (std::vector&lt; double &gt; && data, int count) <br>_Replaces the series data by moving an interleaved double buffer._  |
|  void | [**setData**](classQAccelPlot_1_1PlotSeries.md#function-setdata-34) (const double \* data, int count, const [**DataBounds**](structQAccelPlot_1_1PlotSeries_1_1DataBounds.md) & bounds) <br>_Like_ `setData` _(__data_ _,__count_ _), with the data extents given as__bounds_ _instead of scanned for._ |
|  void | [**setData**](classQAccelPlot_1_1PlotSeries.md#function-setdata-44) (std::vector&lt; double &gt; && data, int count, const [**DataBounds**](structQAccelPlot_1_1PlotSeries_1_1DataBounds.md) & bounds) <br>_Like_ `setData` _(__data_ _,__count_ _), with the data extents given as__bounds_ _instead of scanned for._ |
|  void | [**setDataF**](classQAccelPlot_1_1PlotSeries.md#function-setdataf-14) (const float \* data, int count) <br>_Replaces the series data with_ _count_ _data points copied from an interleaved float array._ |
|  void | [**setDataF**](classQAccelPlot_1_1PlotSeries.md#function-setdataf-24) (std::vector&lt; float &gt; && data, int count) <br>_Replaces the series data by moving an interleaved float buffer._  |
|  void | [**setDataF**](classQAccelPlot_1_1PlotSeries.md#function-setdataf-34) (const float \* data, int count, const [**DataBounds**](structQAccelPlot_1_1PlotSeries_1_1DataBounds.md) & bounds) <br>_Like_ `setDataF` _(__data_ _,__count_ _), with the data extents given as__bounds_ _instead of scanned for._ |
|  void | [**setDataF**](classQAccelPlot_1_1PlotSeries.md#function-setdataf-44) (std::vector&lt; float &gt; && data, int count, const [**DataBounds**](structQAccelPlot_1_1PlotSeries_1_1DataBounds.md) & bounds) <br>_Like_ `setDataF` _(__data_ _,__count_ _), with the data extents given as__bounds_ _instead of scanned for._ |
|  void | [**setHoverEnabled**](classQAccelPlot_1_1PlotSeries.md#function-sethoverenabled) (bool enabled) <br>_Sets whether the series reacts to the mouse cursor to_ _enabled_ _._ |
|  void | [**setLegendSymbol**](classQAccelPlot_1_1PlotSeries.md#function-setlegendsymbol) ([**LegendSymbol**](classQAccelPlot_1_1PlotSeries.md#enum-legendsymbol) symbol) <br> |
|  void | [**setName**](classQAccelPlot_1_1PlotSeries.md#function-setname) (const QString & name) <br> |
|  void | [**setPlotRect**](classQAccelPlot_1_1PlotSeries.md#function-setplotrect) (const QRectF & rect) <br>_Updates the series geometry to exactly cover_ _rect_ _._ |
|  void | [**setXAxis**](classQAccelPlot_1_1PlotSeries.md#function-setxaxis) ([**Axis**](classQAccelPlot_1_1Axis.md) \* axis) <br> |
|  void | [**setYAxis**](classQAccelPlot_1_1PlotSeries.md#function-setyaxis) ([**Axis**](classQAccelPlot_1_1Axis.md) \* axis) <br> |
|  [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**xAxis**](classQAccelPlot_1_1PlotSeries.md#function-xaxis-22) () const<br> |
|  std::optional&lt; [**DataExtent**](structQAccelPlot_1_1PlotSeries_1_1DataExtent.md) &gt; | [**xDataRange**](classQAccelPlot_1_1PlotSeries.md#function-xdatarange) () const<br>_Returns the extent this series reports to its horizontal axis, or_ `std::nullopt` _when it has none._ |
|  [**Axis**](classQAccelPlot_1_1Axis.md) \* | [**yAxis**](classQAccelPlot_1_1PlotSeries.md#function-yaxis-22) () const<br> |
|  std::optional&lt; [**DataExtent**](structQAccelPlot_1_1PlotSeries_1_1DataExtent.md) &gt; | [**yDataRange**](classQAccelPlot_1_1PlotSeries.md#function-ydatarange) () const<br>_Returns the extent this series reports to its vertical axis, or_ `std::nullopt` _when it has none._ |
|   | [**~PlotSeries**](classQAccelPlot_1_1PlotSeries.md#function-plotseries) () override<br> |








## Protected Types inherited from QAccelPlot::PlotSeries

See [QAccelPlot::PlotSeries](classQAccelPlot_1_1PlotSeries.md)

| Type | Name |
| ---: | :--- |
| enum  | [**CapacityOverflow**](classQAccelPlot_1_1PlotSeries.md#enum-capacityoverflow)  <br>_What a series draws when its records outgrow its GPU capacity._  |
| enum  | [**DataChange**](classQAccelPlot_1_1PlotSeries.md#enum-datachange)  <br>_How the records changed in a data update._  |








## Protected Attributes inherited from QAccelPlot::PlotSeries

See [QAccelPlot::PlotSeries](classQAccelPlot_1_1PlotSeries.md)

| Type | Name |
| ---: | :--- |
|  bool | [**overRenderCapacity\_**](classQAccelPlot_1_1PlotSeries.md#variable-overrendercapacity_)   = `{false}`<br>_Whether the record count of the last_ `updateRenderCapacity()` _exceeded_`renderCapacity_` _._ |
|  int | [**renderCapacity\_**](classQAccelPlot_1_1PlotSeries.md#variable-rendercapacity_)   = `{std::numeric\_limits&lt;int&gt;::max()}`<br>_Records the GPU buffers hold, as of the last_ `updateRenderCapacity()` _; the largest_`int` _before the first._ |






























## Protected Functions

| Type | Name |
| ---: | :--- |
|  void | [**hoverEnterEvent**](#function-hoverenterevent) (QHoverEvent \* event) override<br> |
|  void | [**hoverLeaveEvent**](#function-hoverleaveevent) (QHoverEvent \* event) override<br> |
|  void | [**hoverMoveEvent**](#function-hovermoveevent) (QHoverEvent \* event) override<br> |
| virtual [**InspectionRecord**](structQAccelPlot_1_1InspectionRecord.md) | [**inspectionRecord**](#function-inspectionrecord) (int index) override const<br>_Returns the rectangle at_ _index_ _with its edges and category._ |
| virtual [**InspectionRecord**](structQAccelPlot_1_1InspectionRecord.md) | [**inspectionRecordAt**](#function-inspectionrecordat) (const QPointF & position) override const<br>_Returns the rectangle drawn at the series-local_ _position_ _._ |
| virtual void | [**onAxisScaleChanged**](#function-onaxisscalechanged) () override<br>_Repaints with the coordinates mapped for the new scale._  |
| virtual void | [**onDataPointsChanged**](#function-ondatapointschanged) ([**DataChange**](classQAccelPlot_1_1PlotSeries.md#enum-datachange) change, int previousCount) override<br>_Refreshes the vertices, the hover index, and the count after the rectangles changed._  |
| virtual bool | [**recordInspectionSupported**](#function-recordinspectionsupported) () override const<br>_Returns true: rectangles are inspected as records._  |
|  QSGNode \* | [**updatePaintNode**](#function-updatepaintnode) (QSGNode \* oldNode, UpdatePaintNodeData \*) override<br> |


## Protected Functions inherited from QAccelPlot::PlotSeries

See [QAccelPlot::PlotSeries](classQAccelPlot_1_1PlotSeries.md)

| Type | Name |
| ---: | :--- |
|   | [**PlotSeries**](classQAccelPlot_1_1PlotSeries.md#function-plotseries-22) ([**DataPointStore::Layout**](structQAccelPlot_1_1DataPointStore_1_1Layout.md) layout, [**DataPointWording**](structQAccelPlot_1_1PlotSeries_1_1DataPointWording.md) wording, QQuickItem \* parent) <br>_Creates a series whose data points have_ _layout_ _;__wording_ _names them in warnings._ |
|  void | [**appendDataPoint**](classQAccelPlot_1_1PlotSeries.md#function-appenddatapoint) (const double \* coordinates) <br>_Appends one double data point and widens the data ranges by its valid coordinates._  |
| virtual void | [**applyDataPoints**](classQAccelPlot_1_1PlotSeries.md#function-applydatapoints) ([**DataPointInput**](structQAccelPlot_1_1PlotSeries_1_1DataPointInput.md) && input) <br>_Applies data points passed to_ `setData()` _or_`setDataF()` _._ |
| virtual [**DataRanges**](structQAccelPlot_1_1PlotSeries_1_1DataRanges.md) | [**computeDataRanges**](classQAccelPlot_1_1PlotSeries.md#function-computedataranges) () const<br>_Scans the records for their extents. The default implementation scans the data points._  |
| virtual int | [**dataPointValueCount**](classQAccelPlot_1_1PlotSeries.md#function-datapointvaluecount) () const<br>_Returns the number of values per data point that_ `setData()` _and_`setDataF()` _take._ |
|  [**DataPointStore**](classQAccelPlot_1_1DataPointStore.md) & | [**dataPoints**](classQAccelPlot_1_1PlotSeries.md#function-datapoints-12) () <br>_Returns the data points of the series._  |
|  const [**DataPointStore**](classQAccelPlot_1_1DataPointStore.md) & | [**dataPoints**](classQAccelPlot_1_1PlotSeries.md#function-datapoints-22) () const<br>_Returns the data points of the series._  |
|  bool | [**event**](classQAccelPlot_1_1PlotSeries.md#function-event) (QEvent \* event) override<br>_Withholds hover events from a series beneath another series under the cursor._  |
|  void | [**extendXDataRange**](classQAccelPlot_1_1PlotSeries.md#function-extendxdatarange) (qreal x) <br>_Widens the X extent to include_ _x_ _and notifies the bound axes._ |
|  void | [**extendYDataRange**](classQAccelPlot_1_1PlotSeries.md#function-extendydatarange) (qreal y) <br>_Widens the Y extent to include_ _y_ _. A non-finite__y_ _leaves the extent unchanged._ |
| virtual bool | [**inspectionAvailable**](classQAccelPlot_1_1PlotSeries.md#function-inspectionavailable) () const<br>_Returns false while the records are ambiguous, such as during a data transition. Default: true._  |
|  void | [**inspectionDataChanged**](classQAccelPlot_1_1PlotSeries.md#function-inspectiondatachanged) ([**DataChange**](classQAccelPlot_1_1PlotSeries.md#enum-datachange) change=DataChange::Replaced) <br>_Advances the data revision and refreshes the data queries. Call after every accepted record change._  |
| virtual [**InspectionDataPoint**](structQAccelPlot_1_1InspectionDataPoint.md) | [**inspectionDataPointAt**](classQAccelPlot_1_1PlotSeries.md#function-inspectiondatapointat) (const QPointF & position) const<br>_Returns the record drawn at the series-local_ _position_ _as a data point. Default: unsupported._ |
| virtual [**InspectionRecord**](structQAccelPlot_1_1InspectionRecord.md) | [**inspectionRecord**](classQAccelPlot_1_1PlotSeries.md#function-inspectionrecord) (int index) const<br>_Returns the native record at_ _index_ _for series that are not plain XY series. Default: unsupported._ |
| virtual [**InspectionRecord**](structQAccelPlot_1_1InspectionRecord.md) | [**inspectionRecordAt**](classQAccelPlot_1_1PlotSeries.md#function-inspectionrecordat) (const QPointF & position) const<br>_Returns the native record drawn at the series-local_ _position_ _. Default: unsupported._ |
| virtual [**InspectionSource**](structQAccelPlot_1_1InspectionSource.md) | [**inspectionSource**](classQAccelPlot_1_1PlotSeries.md#function-inspectionsource) () const<br>_Returns a view of the XY records that data point queries search. The default has none._  |
|  void | [**invalidateDataRanges**](classQAccelPlot_1_1PlotSeries.md#function-invalidatedataranges) () <br>_Discards the cached extents and notifies the bound axes._  |
|  void | [**invalidateInspection**](classQAccelPlot_1_1PlotSeries.md#function-invalidateinspection) () <br>_Refreshes the data queries after record validity changed without a data change, such as an axis scale switch._  |
|  bool | [**logScaleX**](classQAccelPlot_1_1PlotSeries.md#function-logscalex) () const<br>_Returns whether the horizontal axis is logarithmic; false without an axis._  |
|  bool | [**logScaleY**](classQAccelPlot_1_1PlotSeries.md#function-logscaley) () const<br>_Returns whether the vertical axis is logarithmic; false without an axis._  |
|  void | [**notifyDataPointsChanged**](classQAccelPlot_1_1PlotSeries.md#function-notifydatapointschanged) ([**DataChange**](classQAccelPlot_1_1PlotSeries.md#enum-datachange) change, int previousCount) <br>_Finishes a change of the data points: data ranges,_ `onDataPointsChanged()` _,_`countChanged()` _, data queries, and a repaint._ |
| virtual void | [**onAxisRangeChanged**](classQAccelPlot_1_1PlotSeries.md#function-onaxisrangechanged) () <br>_Called when the viewport of a bound axis changes. The default implementation schedules a repaint._  |
| virtual void | [**onAxisScaleChanged**](classQAccelPlot_1_1PlotSeries.md#function-onaxisscalechanged) () <br>_Called when a bound axis switches between linear and logarithmic scale, or a different axis is bound._  |
| virtual void | [**onDataPointsChanged**](classQAccelPlot_1_1PlotSeries.md#function-ondatapointschanged) ([**DataChange**](classQAccelPlot_1_1PlotSeries.md#enum-datachange) change, int previousCount) <br>_Called after the data points changed, before the data queries are refreshed. The default does nothing._  |
| virtual bool | [**positionInspectionSupported**](classQAccelPlot_1_1PlotSeries.md#function-positioninspectionsupported) () const<br>_Returns whether_ `inspectionDataPointAt()` _matches records drawn as areas, such as heatmap cells. Default: false._ |
| virtual bool | [**recordInspectionSupported**](classQAccelPlot_1_1PlotSeries.md#function-recordinspectionsupported) () const<br>_Returns whether_ `inspectionRecord()` _and_`inspectionRecordAt()` _answer for this series. Default: false._ |
|  QRectF | [**resolvePlotRect**](classQAccelPlot_1_1PlotSeries.md#function-resolveplotrect) () const<br>_Returns the plot area to render into:_ `plotRect` _when set, otherwise the item's current size._ |
|  void | [**setHovered**](classQAccelPlot_1_1PlotSeries.md#function-sethovered) (bool hovered) <br>_Sets the hovered property to_ _hovered_ _. Called by the hover handling of a series._ |
|  void | [**storeDataPoints**](classQAccelPlot_1_1PlotSeries.md#function-storedatapoints) ([**DataPointInput**](structQAccelPlot_1_1PlotSeries_1_1DataPointInput.md) && input) <br>_Stores_ _input_ _as the data points and finishes the change._ |
|  void | [**supportHover**](classQAccelPlot_1_1PlotSeries.md#function-supporthover) () <br>_Makes the series accept hover events while_ `hoverEnabled` _is_`true` _._ |
|  void | [**updateRenderCapacity**](classQAccelPlot_1_1PlotSeries.md#function-updaterendercapacity) (int capacity, int recordCount, [**CapacityOverflow**](classQAccelPlot_1_1PlotSeries.md#enum-capacityoverflow) overflow=CapacityOverflow::DropsExcess) <br>_Stores_ _capacity_ _, the number of records the GPU buffers hold, and warns when__recordCount_ _newly exceeds it._ |
|  bool | [**validateDataSize**](classQAccelPlot_1_1PlotSeries.md#function-validatedatasize) (std::size\_t valueCount, int count, bool floatData, int valuesPerDataPoint=0) const<br>_Returns whether_ _valueCount_ _values hold__count_ _data points; warns otherwise._ |
|  bool | [**validateRawDataArguments**](classQAccelPlot_1_1PlotSeries.md#function-validaterawdataarguments) (const void \* data, int count) const<br>_Returns whether_ _data_ _and__count_ _are valid arguments for copying data points; warns otherwise._ |






## Detailed Description


Rectangles are stored as interleaved (x1, y1, x2, y2) values and uploaded to the GPU as a float data texture, making it suitable for millions of rectangles. The `setData()` overloads keep doubles and upload them relative to an origin near the data, so large coordinates such as epoch timestamps stay precise. The `setDataF()` overloads store floats and upload them without conversion. Hover detection uses an internal `SpatialGrid`; rectangles spanning more than 64 grid cells are tested linearly.


An infinite edge extends the rectangle to the plot edge, e.g. `y1` = -Infinity and `y2` = +Infinity for a full-height span. Infinite edges don't affect the axes' data ranges. Rectangles with a NaN edge are not drawn or hovered.


Each rectangle can carry a `category`, an index into `categoryColors`. Rectangles without a category, or with one outside `categoryColors`, use `color`.


Set `antialiasing` to smooth edges that fall between pixels. It is off by default because rectangles that touch then show faint seams.




**
**

Up to 16,777,216 (2^24) rectangles are drawn correctly. The shader indexes rectangles in single precision, and at that count the data texture reaches 8192 rows, the size every GPU supports.




**See also:** [**LineCurve**](classQAccelPlot_1_1LineCurve.md), [**Axis**](classQAccelPlot_1_1Axis.md) 



    
## Public Properties Documentation





### property border {#property-border-12}

_Outline inside each rectangle's edges, e.g._ `border.width: 1` _. Default: black, width 0._
```C++
Border* QAccelPlot::RectangleSeries::border;
```



On rectangles too small for two outlines, the outline narrows so at least 1 px of fill stays visible. 


        

<hr>




### property categoryColors {#property-categorycolors-12}

_Fill colors indexed by each rectangle's_ `category` _. Default: empty._
```C++
QList<QColor> QAccelPlot::RectangleSeries::categoryColors;
```




<hr>




### property color {#property-color-12}

_Fill color of rectangles without a category color. Default:_ `Colors.dark.seriesPrimary` _with alpha 50._
```C++
QColor QAccelPlot::RectangleSeries::color;
```




<hr>




### property hoverColor {#property-hovercolor-12}

_Fill color of the rectangle under the cursor. Default: an invalid color, no highlight._ 
```C++
QColor QAccelPlot::RectangleSeries::hoverColor;
```




<hr>




### property hoveredIndex {#property-hoveredindex-12}

_Read-only: index of the rectangle under the cursor, or -1 when none._ 
```C++
int QAccelPlot::RectangleSeries::hoveredIndex;
```




<hr>




### property minimumHeight {#property-minimumheight-12}

_Minimum drawn height in pixels, like_ `minimumWidth` _. Default: 1._
```C++
qreal QAccelPlot::RectangleSeries::minimumHeight;
```




<hr>




### property minimumWidth {#property-minimumwidth-12}

_Minimum drawn width in pixels, so narrow rectangles stay visible when zoomed out. Default: 1._ 
```C++
qreal QAccelPlot::RectangleSeries::minimumWidth;
```



Narrower rectangles are widened around their center. Hover uses the widened size. Clamped to at least 0. 


        

<hr>
## Public Signals Documentation





### signal categoryColorsChanged {#signal-categorycolorschanged}

_Emitted when the categoryColors property changes._ 
```C++
void QAccelPlot::RectangleSeries::categoryColorsChanged;
```




<hr>




### signal colorChanged {#signal-colorchanged}

_Emitted when the color property changes._ 
```C++
void QAccelPlot::RectangleSeries::colorChanged;
```




<hr>




### signal hoverColorChanged {#signal-hovercolorchanged}

_Emitted when the hoverColor property changes._ 
```C++
void QAccelPlot::RectangleSeries::hoverColorChanged;
```




<hr>




### signal hoveredIndexChanged {#signal-hoveredindexchanged}

_Emitted when the hovered rectangle index changes._ 
```C++
void QAccelPlot::RectangleSeries::hoveredIndexChanged;
```




<hr>




### signal minimumHeightChanged {#signal-minimumheightchanged}

_Emitted when the minimumHeight property changes._ 
```C++
void QAccelPlot::RectangleSeries::minimumHeightChanged;
```




<hr>




### signal minimumWidthChanged {#signal-minimumwidthchanged}

_Emitted when the minimumWidth property changes._ 
```C++
void QAccelPlot::RectangleSeries::minimumWidthChanged;
```




<hr>
## Public Functions Documentation





### function RectangleSeries {#function-rectangleseries}

_Constructs a_ [_**RectangleSeries**_](classQAccelPlot_1_1RectangleSeries.md) _with the given__parent_ _._
```C++
explicit QAccelPlot::RectangleSeries::RectangleSeries (
    QQuickItem * parent=nullptr
) 
```




<hr>




### function border {#function-border-22}

_Returns the grouped outline settings. The object is owned by the series._ 
```C++
Border * QAccelPlot::RectangleSeries::border () const
```




<hr>




### function categoryColors {#function-categorycolors-22}

_Returns the fill colors indexed by category._ 
```C++
QList< QColor > QAccelPlot::RectangleSeries::categoryColors () const
```




<hr>




### function clearData {#function-cleardata}

_Removes all rectangles._ 
```C++
virtual Q_INVOKABLE void QAccelPlot::RectangleSeries::clearData () override
```



Implements [*QAccelPlot::PlotSeries::clearData*](classQAccelPlot_1_1PlotSeries.md#function-cleardata)


<hr>




### function color {#function-color-22}

_Returns the rectangle fill color._ 
```C++
QColor QAccelPlot::RectangleSeries::color () const
```




<hr>




### function contains {#function-contains}

_Returns_ `true` _when a rectangle lies under item position__point_ _._
```C++
bool QAccelPlot::RectangleSeries::contains (
    const QPointF & point
) override const
```



Hover delivery uses this test, so stacked series underneath still receive hover events outside this series rectangles. 


        

<hr>




### function hoverColor {#function-hovercolor-22}

_Returns the fill color of the hovered rectangle._ 
```C++
QColor QAccelPlot::RectangleSeries::hoverColor () const
```




<hr>




### function hoveredIndex {#function-hoveredindex-22}

_Returns the index of the hovered rectangle, or -1 if none._ 
```C++
int QAccelPlot::RectangleSeries::hoveredIndex () const
```




<hr>




### function legendEntry {#function-legendentry}

_Returns the legend entry: the symbol and color of the rectangles._ 
```C++
virtual std::optional< LegendEntry > QAccelPlot::RectangleSeries::legendEntry () override const
```



Implements [*QAccelPlot::PlotSeries::legendEntry*](classQAccelPlot_1_1PlotSeries.md#function-legendentry)


<hr>




### function minimumHeight {#function-minimumheight-22}

_Returns the minimum drawn height in pixels._ 
```C++
qreal QAccelPlot::RectangleSeries::minimumHeight () const
```




<hr>




### function minimumWidth {#function-minimumwidth-22}

_Returns the minimum drawn width in pixels._ 
```C++
qreal QAccelPlot::RectangleSeries::minimumWidth () const
```




<hr>




### function postData {#function-postdata-14}

_Thread-safe: queues_ `setData` _(__data_ _,__categories_ _,__rectCount_ _) to the item's thread._
```C++
void QAccelPlot::RectangleSeries::postData (
    std::vector< double > && data,
    std::vector< int > && categories,
    int rectCount
) 
```




<hr>




### function postData {#function-postdata-24}

_Thread-safe: queues_ `setDataF` _(__data_ _,__categories_ _,__rectCount_ _) to the item's thread._
```C++
void QAccelPlot::RectangleSeries::postData (
    std::vector< float > && data,
    std::vector< int > && categories,
    int rectCount
) 
```




<hr>




### function postData {#function-postdata-34}

_Queues a moved double buffer for assignment on the series' thread._ 
```C++
void QAccelPlot::RectangleSeries::postData (
    std::vector< double > && data,
    int count
) 
```




<hr>




### function postData {#function-postdata-44}

_Queues a moved float buffer for assignment on the series' thread._ 
```C++
void QAccelPlot::RectangleSeries::postData (
    std::vector< float > && data,
    int count
) 
```




<hr>




### function rectangleAt {#function-rectangleat}

_Returns rectangle_ _index_ _as an object with_`x1` _,_`y1` _,_`x2` _,_`y2` _properties._
```C++
Q_INVOKABLE QVariantMap QAccelPlot::RectangleSeries::rectangleAt (
    int index
) const
```



Includes `category` when categories are set. Returns an empty object when _index_ is out of range. 


        

<hr>




### function rectangleIndexAt {#function-rectangleindexat}

_Returns the index of the topmost rectangle under item position_ _position_ _, or -1._
```C++
int QAccelPlot::RectangleSeries::rectangleIndexAt (
    const QPointF & position
) const
```




<hr>




### function setCategories {#function-setcategories}

_Sets one category per rectangle. An empty list clears categories; any other size must equal_ `count` _._
```C++
Q_INVOKABLE void QAccelPlot::RectangleSeries::setCategories (
    const QList< int > & categories
) 
```




<hr>




### function setCategoryColors {#function-setcategorycolors}

_Sets the fill colors indexed by category to_ _colors_ _._
```C++
void QAccelPlot::RectangleSeries::setCategoryColors (
    const QList< QColor > & colors
) 
```




<hr>




### function setColor {#function-setcolor}

_Sets the fill color to_ _color_ _._
```C++
void QAccelPlot::RectangleSeries::setColor (
    const QColor & color
) 
```




<hr>




### function setData {#function-setdata-16}

_Loads rectangles from_ _rects_ _, a QML list of objects with_`x1` _,_`y1` _,_`x2` _,_`y2` _properties._
```C++
Q_INVOKABLE void QAccelPlot::RectangleSeries::setData (
    const QVariantList & rects
) 
```



A missing or null `x1` / `y1` is -Infinity and a missing `x2` / `y2` is +Infinity, so `{ x1: 8, x2: 12 }` is a full-height span. An optional integer `category` selects the fill color from `categoryColors`. 


        

<hr>




### function setData {#function-setdata-26}

_Moves_ _data_ _and per-rectangle__categories_ _(empty, or exactly__rectCount_ _) into the series._
```C++
void QAccelPlot::RectangleSeries::setData (
    std::vector< double > && data,
    std::vector< int > && categories,
    int rectCount
) 
```




<hr>




### function setData {#function-setdata-36}

_Replaces the series data with_ _count_ _data points copied from an interleaved double array._
```C++
void QAccelPlot::RectangleSeries::setData (
    const double * data,
    int count
) 
```



See the class description for the data point layout of each series. 


        

<hr>




### function setData {#function-setdata-46}

_Replaces the series data by moving an interleaved double buffer._ 
```C++
void QAccelPlot::RectangleSeries::setData (
    std::vector< double > && data,
    int count
) 
```




<hr>




### function setData {#function-setdata-56}

_Like_ `setData` _(__data_ _,__count_ _), with the data extents given as__bounds_ _instead of scanned for._
```C++
void QAccelPlot::RectangleSeries::setData (
    const double * data,
    int count,
    const DataBounds & bounds
) 
```




<hr>




### function setData {#function-setdata-66}

_Like_ `setData` _(__data_ _,__count_ _), with the data extents given as__bounds_ _instead of scanned for._
```C++
void QAccelPlot::RectangleSeries::setData (
    std::vector< double > && data,
    int count,
    const DataBounds & bounds
) 
```




<hr>




### function setDataF {#function-setdataf-15}

_Like_ `setDataF` _(__data_ _,__rectCount_ _) and also moves per-rectangle__categories_ _(empty, or exactly__rectCount_ _) into the series._
```C++
void QAccelPlot::RectangleSeries::setDataF (
    std::vector< float > && data,
    std::vector< int > && categories,
    int rectCount
) 
```




<hr>




### function setDataF {#function-setdataf-25}

_Replaces the series data with_ _count_ _data points copied from an interleaved float array._
```C++
void QAccelPlot::RectangleSeries::setDataF (
    const float * data,
    int count
) 
```




<hr>




### function setDataF {#function-setdataf-35}

_Replaces the series data by moving an interleaved float buffer._ 
```C++
void QAccelPlot::RectangleSeries::setDataF (
    std::vector< float > && data,
    int count
) 
```




<hr>




### function setDataF {#function-setdataf-45}

_Like_ `setDataF` _(__data_ _,__count_ _), with the data extents given as__bounds_ _instead of scanned for._
```C++
void QAccelPlot::RectangleSeries::setDataF (
    const float * data,
    int count,
    const DataBounds & bounds
) 
```




<hr>




### function setDataF {#function-setdataf-55}

_Like_ `setDataF` _(__data_ _,__count_ _), with the data extents given as__bounds_ _instead of scanned for._
```C++
void QAccelPlot::RectangleSeries::setDataF (
    std::vector< float > && data,
    int count,
    const DataBounds & bounds
) 
```




<hr>




### function setHoverColor {#function-sethovercolor}

_Sets the fill color of the hovered rectangle to_ _color_ _. An invalid color disables the highlight._
```C++
void QAccelPlot::RectangleSeries::setHoverColor (
    const QColor & color
) 
```




<hr>




### function setMinimumHeight {#function-setminimumheight}

_Sets the minimum drawn height to_ _height_ _pixels. Negative values are clamped to 0._
```C++
void QAccelPlot::RectangleSeries::setMinimumHeight (
    qreal height
) 
```




<hr>




### function setMinimumWidth {#function-setminimumwidth}

_Sets the minimum drawn width to_ _width_ _pixels. Negative values are clamped to 0._
```C++
void QAccelPlot::RectangleSeries::setMinimumWidth (
    qreal width
) 
```




<hr>




### function ~RectangleSeries {#function-rectangleseries}

_Destroys the rectangle series._ 
```C++
QAccelPlot::RectangleSeries::~RectangleSeries () override
```




<hr>
## Protected Functions Documentation





### function hoverEnterEvent {#function-hoverenterevent}

```C++
void QAccelPlot::RectangleSeries::hoverEnterEvent (
    QHoverEvent * event
) override
```




<hr>




### function hoverLeaveEvent {#function-hoverleaveevent}

```C++
void QAccelPlot::RectangleSeries::hoverLeaveEvent (
    QHoverEvent * event
) override
```




<hr>




### function hoverMoveEvent {#function-hovermoveevent}

```C++
void QAccelPlot::RectangleSeries::hoverMoveEvent (
    QHoverEvent * event
) override
```




<hr>




### function inspectionRecord {#function-inspectionrecord}

_Returns the rectangle at_ _index_ _with its edges and category._
```C++
virtual InspectionRecord QAccelPlot::RectangleSeries::inspectionRecord (
    int index
) override const
```



Implements [*QAccelPlot::PlotSeries::inspectionRecord*](classQAccelPlot_1_1PlotSeries.md#function-inspectionrecord)


<hr>




### function inspectionRecordAt {#function-inspectionrecordat}

_Returns the rectangle drawn at the series-local_ _position_ _._
```C++
virtual InspectionRecord QAccelPlot::RectangleSeries::inspectionRecordAt (
    const QPointF & position
) override const
```



Implements [*QAccelPlot::PlotSeries::inspectionRecordAt*](classQAccelPlot_1_1PlotSeries.md#function-inspectionrecordat)


<hr>




### function onAxisScaleChanged {#function-onaxisscalechanged}

_Repaints with the coordinates mapped for the new scale._ 
```C++
virtual void QAccelPlot::RectangleSeries::onAxisScaleChanged () override
```



Implements [*QAccelPlot::PlotSeries::onAxisScaleChanged*](classQAccelPlot_1_1PlotSeries.md#function-onaxisscalechanged)


<hr>




### function onDataPointsChanged {#function-ondatapointschanged}

_Refreshes the vertices, the hover index, and the count after the rectangles changed._ 
```C++
virtual void QAccelPlot::RectangleSeries::onDataPointsChanged (
    DataChange change,
    int previousCount
) override
```



Implements [*QAccelPlot::PlotSeries::onDataPointsChanged*](classQAccelPlot_1_1PlotSeries.md#function-ondatapointschanged)


<hr>




### function recordInspectionSupported {#function-recordinspectionsupported}

_Returns true: rectangles are inspected as records._ 
```C++
virtual bool QAccelPlot::RectangleSeries::recordInspectionSupported () override const
```



Implements [*QAccelPlot::PlotSeries::recordInspectionSupported*](classQAccelPlot_1_1PlotSeries.md#function-recordinspectionsupported)


<hr>




### function updatePaintNode {#function-updatepaintnode}

```C++
QSGNode * QAccelPlot::RectangleSeries::updatePaintNode (
    QSGNode * oldNode,
    UpdatePaintNodeData *
) override
```




<hr>

------------------------------
The documentation for this class was generated from the following file `QAccelPlot/src/QAccelPlot/series/RectangleSeries.hpp`

