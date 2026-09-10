# Changelog

All notable changes to QAccelPlot are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- Per-axis `layoutSize` control for explicitly reserving horizontal-axis height
  or vertical-axis width.

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
