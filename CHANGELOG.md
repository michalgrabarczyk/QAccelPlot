# Changelog

All notable changes to QAccelPlot are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- Per-axis `layoutSize` control for explicitly reserving horizontal-axis height
  or vertical-axis width.
- Automatic side assignment for the four named plot axes: `xAxis` uses the
  bottom, `x2Axis` the top, `yAxis` the left, and `y2Axis` the right.

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
