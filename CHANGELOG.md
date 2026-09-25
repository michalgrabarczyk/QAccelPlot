# Changelog

All notable changes to QAccelPlot are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Fixed

- `LineCurve` no longer reads the QML `Gradient` of a `GradientStroke` or
  `GradientFill` from the scene graph render thread on every frame. It now reads
  the gradient on the GUI thread when the effect changes.

## [0.3.0] — 2026-09-25

### Added

- Marker shapes `TriangleLeft`, `TriangleRight`, `XCross`, `HLine`, `VLine`,
  `Star`, `Asterisk`, `Hexagon`, and `Pentagon`, and a `Pixel` marker that draws
  each sample as a single pixel for very dense scatter plots.
- Hollow markers: set `marker.filled` to `false` to outline filled shapes with
  `marker.strokeWidth` pixels. Legend symbols follow the series.
- The `styling/markers` example shows every shape. Click a shape to draw a
  scatter plot with it, and adjust marker size, outline, and opacity.
- The `plot_types/parametric_curves` example draws curves whose x values do
  not increase: a spiral, a Lissajous figure, a spirograph, and a Van der Pol
  phase portrait.
- The `plot_types/point_cloud` example draws a value-colored cloud with a color bar.
- `LineCurve::setData(std::vector<double>&&, int)` moves an interleaved double
  buffer into the curve, keeping double precision for large coordinates such as
  Unix-epoch timestamps without re-interleaving.
- `LineCurve::postData(std::vector<double>&&, int)` hands off an interleaved
  double buffer from a worker thread, like the existing float `postData()`.
- `PointCloud` series for large unconnected scatter data. Its markers use the
  same `marker` grouped property as `LineCurve`.
- `Colormap`, mapping data values to colors through a built-in ramp (`Viridis`, `Plasma`,
  `Inferno`, `Magma`, `Grayscale`, `Rainbow`) or custom stops, with optional
  bounds, linear or logarithmic normalization, and a `reversed` flag. `GradientFill.colormap` and
  `GradientStroke.colormap` take a colormap as an alternative to `gradient`, which
  it overrides when both are set. `PointCloud.colormap` colors points by their
  per-point values.
- `ColorBar`, a continuous key for a `PointCloud` colormap. It draws the ramp
  with ticks for the series' resolved value range, follows `Linear` and `Log`
  normalization, and redraws when the colormap or range changes. Vertical and
  horizontal orientations; ticks are configured through `ticker`, as on `Axis`.

### Changed

- Public headers are now included with a `QAccelPlot/` prefix, for example
  `#include <QAccelPlot/series/LineCurve.hpp>`, both when QAccelPlot is
  embedded with `add_subdirectory` or `FetchContent` and when it is installed.
  The library sources moved to `QAccelPlot/src/QAccelPlot/`. Replace unprefixed
  includes such as `"series/LineCurve.hpp"`.
- `LineCurve.markerShape` and `markerSize` moved to `marker.shape` and `marker.size`.
- Examples are grouped by category under `examples/`: `plot_types/`, `styling/`,
  `axes/`, `data/`, `interaction/`, and `showcases/`, with `quickstart/` at the
  top. Renamed examples: `axis_formats` → `axes/tick_formats`, `custom_axis` →
  `axes/multiple_axes`, `interactive_tools` → `interaction/measurement_tools`,
  `performance_showcase` → `showcases/performance`, and `pulsar_showcase` →
  `showcases/pulsar`. Their executables are renamed to match, for example
  `QAccelPlotExampleTickFormats`.
- The styling and transitions example is split into single-topic examples:
  `styling/line_styles`, `styling/markers`, `styling/gradients`,
  `styling/transitions`, `styling/theming` (dark and light palettes), and
  `data/missing_data` (invalid samples and gaps).

### Removed

- `LogTickLabelFormatter`. The default `NumericTickLabelFormatter` now labels
  logarithmic axes with the same superscript powers of ten.

### Fixed

- The Qt Quick `opacity` property, set on a series or inherited from a parent
  item, now applies to `LineCurve` lines, markers, `GradientStroke`, and
  `GradientFill`, and to `PointCloud` and `RectangleList`. Previously it had no
  effect. Each fragment blends on its own, so overlapping translucent markers
  build up density.
- Numeric tick labels no longer show a spurious trailing zero. Labels now use
  exactly the decimal places needed to write the tick step, so steps of 1 and 2
  read `0, 1, 2` instead of `0.0, 1.0, 2.0`, and steps of 0.1 and 0.2 read
  `0.1, 0.2` instead of `0.10, 0.20`. All labels on a linear axis keep the
  same number of decimals, for example `0.0, 0.5, 1.0, 1.5, 2.0`.
- Logarithmic axes now label decades as superscript powers of ten
  (`10⁻², 10⁻¹, 10⁰, 10¹, 10²`) by default, instead of
  `0.010, 0.10, 1.0, 10, 100`. Custom formatters can override the new
  `TickLabelFormatter::doFormatLogTick()`; `tickLabel` callbacks are unchanged.
- `LineCurve` now renders its marker for a single-point dataset when both lines
  and markers are enabled.
- Rectangle hover detection now preserves double precision for large coordinates,
  including Unix-epoch timestamps, so nearby rectangles remain distinguishable.
- `RectangleList` now refreshes its render coordinates when either axis switches
  between linear and logarithmic scales, keeping rectangles positioned correctly
  without requiring new data.
- Overlapping rectangles no longer cause quadratic memory growth in the hover
  lookup index, preventing excessive memory use and crashes with large datasets.
- Removing, reparenting, or destroying a plot series no longer leaves it
  registered with its former plot, preventing later updates from accessing a
  dangling series pointer.
- `LineCurve` no longer keeps a dangling `transition` pointer when an
  externally owned data transition is destroyed; the property is cleared and
  its change is reported.
- Canceling, replacing, or destroying a `LineCurve` transition before its
  first rendered frame no longer leaves the point count inconsistent with the
  available data buffers, preventing out-of-bounds rendering access.
- Toggling a `LineCurve` gradient stroke no longer double-deletes its scene
  graph node and crashes; the renderer now preserves the node while replacing
  its material.
- Setting a linear axis ticker's `tickCount` to zero or below now disables its
  grid generation instead of potentially exhausting memory, and hiding both
  the main grid and subgrid clears existing geometry.
- Antialiased lines and markers no longer lose coverage inside their nominal
  size. The `antialiasingFeather` ramp is now centered on the edge, so 1 px lines
  stay continuous regardless of pixel alignment, the 1 px legend symbol is
  visible, and thicker lines render at their full width. Lines thinner than
  1 px draw 1 px wide with proportionally reduced alpha.
- `Diamond`, `TriangleUp`, and `TriangleDown` markers are no longer clipped flat
  at their tips, and their antialiased edges are equally soft on every side.
  They keep their previous width and height.
- `GradientStroke` and `GradientFill` now keep a `Fixed` gradient value bound
  when the other bound uses `DataRange`. Previously both bounds were taken from
  the data range.
- `GradientStroke` and `GradientFill` now redraw when a stop of their
  `gradient` changes color or position at runtime.

## [0.2.0] — 2026-09-16

### Added

- Per-axis `layoutSize` control for explicitly reserving horizontal-axis height
  or vertical-axis width.
- Automatic side assignment for the four named plot axes: `xAxis` uses the
  bottom, `x2Axis` the top, `yAxis` the left, and `y2Axis` the right.
- Explicit invalid-sample contract for `LineCurve`: `NaN`, `±Inf`, and
  non-positive values on logarithmic axes break the line, gradient fill, and
  hover hit testing consistently on every graphics backend, never draw markers,
  and are excluded from auto-ranging per dimension.
- `LineCurve.gaps` grouped property. `gaps.nanMode` selects
  `NanGapMode.Break` (default) or `NanGapMode.Connect`, which joins the valid
  samples on either side of a gap.
- A "Gaps" page in the styling and transitions example.

### Changed

- Values at or below zero on a logarithmic axis now render as gaps instead of
  being clamped to a steep drop toward the bottom of the plot.
- `MorphTransition` no longer interpolates from or to non-finite coordinates:
  new gaps appear immediately and samples leaving a gap jump to their target.
- Renamed `RectangleList::setRawData()` to `setData()`, matching the naming
  of the `LineCurve` data-setting APIs.
- Default plot, axis, grid, series, and legend colors now come from the
  built-in dark palette instead of generic Qt colors. The `Colors` palette is
  defined once in C++ and shared with QML, where its usage is unchanged.

### Fixed

- Legend line and marker samples now use the curve renderers, keeping marker
  shapes and dash patterns consistent across graphics backends and display scales.
- Preserved double precision in `LineCurve::setData()` for modern Unix-epoch
  timestamps and large coordinates while keeping the `F` ingestion APIs
  float-based.
- Reassigning a `LineCurve`'s `effects` list no longer deletes the effect
  objects, and effects destroyed elsewhere are removed from the list instead
  of leaving dangling entries.
- A running `LineCurve` transition no longer overwrites data pushed through
  `appendData()` or through a data-setting API called after disabling the
  transition mid-animation: any in-flight transition is now cancelled before
  the new data is applied directly.
- Major tick marks now honor fractional `tickWidth` values (e.g. `1.5`)
  instead of truncating to an integer pixel width.
- `LineCurve` no longer leaks the default `SolidLine` it creates for its
  `lineStyle`: the default is owned by the curve and released when the curve
  is destroyed. Styles assigned from QML are still left untouched, and the
  curve no longer keeps a dangling reference when an assigned style is
  destroyed before it; it emits `lineStyleChanged` and repaints instead.
- Appending the same axis to `extraAxes` twice no longer registers it twice
  and reserves its layout space twice, and reading an out-of-range
  `extraAxes` index returns `null` instead of tripping an assert.
- A very large series or rectangle count that would produce a data texture
  taller than a safe cross-GPU limit now logs a clear warning explaining why
  it may render nothing, instead of failing silently.
- `DashLine` now rejects a pattern containing a negative segment, and warns
  when a pattern is longer than the eight segments the renderer supports
  instead of silently dropping the rest.
- `mouseMoved` now fires for a plain cursor move over the plot, not only while
  a mouse button is held, matching its documented behavior.
- Axes are no longer given a negative width or height when the plot item is
  smaller than its padding plus the space its axes reserve.
- `LineCurve` no longer changes its own position or size from
  `updatePaintNode()`, which runs on the render thread with the threaded
  render loop and is not a safe place to mutate `QQuickItem` geometry. It
  now reads the plot rect that `QAccelPlot` already assigns on the GUI
  thread instead.
- `LineCurve::appendData()` updates the reported data range and the GPU render
  buffer incrementally instead of rescanning and rebuilding the whole curve on
  every call, so streaming N points costs O(N) work instead of O(N²).
- `RectangleList` re-uploads its data texture when the scene-graph node is
  recreated (e.g. after axes are temporarily unset), instead of leaving a
  freshly created node with an empty texture until new data arrives.
- `DataAnchor` no longer dereferences a dangling pointer after its `xAxis` or
  `yAxis` is destroyed elsewhere; the axis reference is cleared automatically
  instead.
- A horizontal-only wheel or touchpad scroll no longer zooms the plot out;
  it is now ignored by the zoom handler and passed through instead.
- Tick labels near zero no longer render as a misleading `"-0.00"` when a
  tiny negative value from accumulated tick-position arithmetic rounds to
  zero at the display precision.
- Vertical gradients on markers now match the orientation of line strokes and
  fills instead of rendering upside down.
- Changing an axis ticker's `tickCount` or `subtickCount` now refreshes the
  plot grid immediately instead of leaving it stale until the next pan or
  zoom.
- The plot area is now relaid out when an axis's `axisLinePadding`,
  `orientation`, or ticker `tickLengthIn`/`subtickLengthIn` changes at
  runtime, instead of only on `visible` or `layoutSize` changes.
- Extra axes now respect their `side`: a horizontal extra axis with
  `side: Top` stacks above the plot instead of always below it, and a
  vertical extra axis now reserves layout space and stacks on its declared
  `side` (`Left` or `Right`) instead of always overlapping the plot's left
  edge with no reserved space.
- Log-scale axis labels no longer collapse to `"0.0"` or `"100.0"` for
  small or large tick values; each major tick's own magnitude is now used
  to derive its label precision.
- `Axis::rescaleToData()` (double-click rescale) no longer does nothing for
  flat data — a constant series or a single point — and synthesizes a small
  viewport around the value instead, proportional to the value's own
  magnitude so a very small nonzero value isn't rendered indistinguishable
  from zero. A log-scale axis with only non-positive data now gets a valid,
  all-positive synthesized viewport instead of a mismatched
  positive-min/non-positive-max range.
- Enabling `logScale` (via the property, `toggleLogScale()`, or the `L` key)
  while the current viewport is non-positive now clamps it to a valid
  positive range instead of collapsing every curve to a single point.
- A non-finite X or Y data range on a `PlotSeries` no longer blocks the
  other axis's range update; each axis's range is now validated and
  reported independently.
- Tick labels are now formatted on the GUI thread before rendering, rather
  than inside `Axis::paint()`, which runs on the render thread with the
  threaded render loop. A `tickLabel` JavaScript callback is therefore never
  invoked from the render thread, and it now receives `tickStep` as its
  second argument (previously always `undefined`).
- Preserved double precision in `RectangleList` for modern Unix-epoch
  timestamps and large coordinates: rectangles are stored and their data
  range computed in double precision, and rendering uses an origin-relative
  float buffer instead of storing raw coordinates as float. A new
  `setData(const double*, int)` overload accepts double-precision data
  directly.
- A single `±Inf` coordinate is now skipped when computing a curve's axis
  data ranges instead of making that range non-finite, and all-`NaN` data no longer reports an inverted range.
- `NaN` samples no longer make adjacent valid line segments disappear or
  produce backend-dependent artifacts.
- On Qt 6.2, the legend now uses the dark palette's background, border, and
  text colors instead of falling back to a white box with dark text.

## [0.1.0] — 2026-09-08

### Added

- GPU-accelerated `LineCurve` rendering integrated with a QML-first `Plot`
  component and C++ `PlotView` API.
- Efficient data-ingestion paths for QML and C++, including thread-safe buffer
  handoff for real-time data produced by worker threads.
- Interactive linear and logarithmic axes with pan, zoom, configurable ticks,
  multiple label formatters, and support for primary, secondary, and extra
  axes.
- Curve styling with solid and dashed lines, point markers, anti-aliasing,
  gradient fills and strokes, and animated draw and morph transitions.
- Hover detection for curves and efficiently batched data-space rectangles.
- Data-anchored QML annotations for labels, regions, and custom overlays.
- Configurable plot grids, automatic legends, and light and dark color
  palettes.
