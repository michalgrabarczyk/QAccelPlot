# PointCloud series — implementation plan

Status: **implemented** (all six phases). The sections below are the original
plan; §0 records where the implementation deviates from it and what was measured.

## 0. As built

### Deviations from the plan

| Plan | Implementation | Reason |
| --- | --- | --- |
| Two data textures (positions, values) | One data texture with a stride of 2 `(x, y)` or 3 `(x, y, value)`, exposed as the `stride` uniform | `DataTextureMaterial` keeps one CPU image buffer per material; one texture means one upload and no second staging buffer |
| `setValues()` converts storage | Values are expanded/compacted **in place** in the interleaved buffer | Avoids a second allocation for large clouds |
| Phase 5: evaluate `gl_VertexIndex` / smaller vertex buffer | Kept 8-byte `(pointId, corner)` × 6 unindexed vertices. A 4-vertex indexed quad was implemented and measured **~11% slower** (44 vs 49 FPS panning 1M points, OpenGL, alternating runs against the same `static_1m` reference) and reverted. `gl_VertexIndex` was not adopted: `QSGGeometry` still needs one buffer entry per vertex, so the saving is 7 bytes per vertex at the risk of backend input-layout differences | Measured, see below |
| Texture capacity: chunk into multiple nodes | Clamp to capacity and warn once (the plan's allowed first-release option). Capacity uses `QRhi::TextureSizeMax` on Qt ≥ 6.6, 8192 before | Keeps one node per series |
| Hover index built after `setData*` | Built lazily on the first pointer query after a data change; `contains()` is overridden so hover is only delivered near points | Streaming clouds without a pointer over them pay nothing |
| — | `readGradientStops()` helper shared by `GradientStroke`, `GradientFill`, `PointCloud` | Removed two identical copies instead of adding a third |
| — | `PlotSeries::resolvePlotRect()` (moved from `LineCurve`) | A series constructed in C++ with the plot as parent never receives `plotRect`; found when the benchmark cloud rendered nothing |
| — | Shader NaN/Inf test decodes exponent bits (`isNonFiniteBits`) | `isnan()` is not reliable on every GLSL ES driver |
| Phase 4 legend unit test | Verified by the example screenshot; `tst_plot_appearance` does not load the QML module | No QML engine in the library unit tests |

Additional API beyond §2: `hasValues`, `pointIndexAt()`, public `contains()`,
`postData(xy, pointCount)`, and `setDataFNoRange(xy, values, pointCount)`.

### Verification

- Unit tests: `tst_point_spatial_index` (10 cases incl. brute-force comparison
  over 2000 queries), `tst_point_cloud_data` (15 cases), 3 new cases in
  `tst_material_comparison`. Full `ctest` (non-AI) passes.
- Example renders identically on D3D11, OpenGL, and Vulkan (no pixel differs by
  more than 8/255); `--animate --screenshot` exercises worker-thread `postData()`.
- `LineCurve` marker screenshots (quickstart, styling_and_transitions) are
  pixel-identical to the pre-change build after the `point_shapes.glsl` extraction.

### Benchmarks (Windows, Qt 6.7.3, OpenGL headless, 1920×1080)

GPU numbers on this machine vary by up to ~25% between runs; compare scenarios
from alternating runs only.

| Scenario | Avg FPS | P50 frame (ms) |
| --- | --- | --- |
| `static_1m` (LineCurve, reference) | 87–90 | 7.1–7.3 |
| `point_cloud_static_1m` (1.5 px markers) | 50 | 12.5 |
| `live_500k` (LineCurve, reference) | 820–875 | 1.0–1.1 |
| `point_cloud_live_250k` (with values) | 690–830 | 1.1–1.3 |
| `point_cloud_ingestion_1m` (CPU) | 740 | 1.3 |
| `point_cloud_hover_index_1m` (CPU) | 65 | 15.3 |

## 1. Goal

Add a `PointCloud` QML series for large, unconnected 2D scatter data
(10⁴–10⁶+ points) that:

- renders every point as a GPU billboard with the existing marker shapes,
- colors points uniformly or by a per-point scalar through a `Gradient` colormap,
- supports linear and log axes, pan/zoom, the default legend, and hover picking,
- accepts data from worker threads without blocking the UI thread,
- keeps steady-state updates to a single data-texture upload per frame.

`LineCurve` with `NoLine` + markers already draws scatter-like plots, but it
builds a 32-byte × 6-vertex buffer on the CPU for every data change, evaluates
gradients per vertex on the CPU, and hit-tests by linear scan. That does not
scale to a million points or to per-point values. `PointCloud` reuses the
data-texture architecture of `RectangleList` instead.

Non-goals for the first release: 3D clouds, per-point size, density
(heat-map) aggregation, transitions, and double-precision origin rebasing
(tracked as follow-ups in §9).

## 2. Public API

```cpp
class PointCloud : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(PointCloud)

    Q_PROPERTY(QColor color ...)                         // Uniform color; default Qt::blue
    Q_PROPERTY(MarkerShape markerShape ...)              // default Circle
    Q_PROPERTY(qreal markerSize ...)                     // radius in px; default 3
    Q_PROPERTY(ColorMode colorMode ...)                  // default UniformColor
    Q_PROPERTY(QQuickGradient* colorGradient ...)        // used by ValueColor
    Q_PROPERTY(GradientValueSource::Source valueMinSource ...)  // DataRange | Fixed
    Q_PROPERTY(qreal valueMin ...)
    Q_PROPERTY(GradientValueSource::Source valueMaxSource ...)
    Q_PROPERTY(qreal valueMax ...)
    Q_PROPERTY(bool antialiasingEnabled ...)             // default true
    Q_PROPERTY(qreal antialiasingFeather ...)            // default 1
    Q_PROPERTY(qreal hoverRadius ...)                    // px; default 6
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)
    Q_PROPERTY(qreal dataValueMin READ ... NOTIFY valueRangeChanged)  // resolved range
    Q_PROPERTY(qreal dataValueMax READ ... NOTIFY valueRangeChanged)

public:
    // MarkerShape is inherited from PlotSeries and shared with LineCurve markers.
    enum class ColorMode { UniformColor, ValueColor };

    Q_INVOKABLE void setData(const QList<QPointF>& points);
    Q_INVOKABLE void setValues(const QList<qreal>& values);   // size must equal count
    Q_INVOKABLE void clearData();
    Q_INVOKABLE QPointF pointAt(int index) const;
    Q_INVOKABLE qreal valueAt(int index) const;                // NaN when no values

    void setDataF(const float* xy, int pointCount);
    void setDataF(std::vector<float>&& xy, int pointCount);
    void setDataF(std::vector<float>&& xy, std::vector<float>&& values, int pointCount);
    void setDataFNoRange(std::vector<float>&& xy, std::vector<float>&& values, int pointCount);
    // Thread-safe; queued to the item's thread. Empty values = clear values.
    void postData(std::vector<float>&& xy, std::vector<float>&& values, int pointCount);
};
```

Data contract:

- `xy` is interleaved `[x0, y0, x1, y1, …]`, `values` is one float per point.
- Non-finite coordinates are kept in storage (indices stay stable) but excluded
  from ranges, hit testing, and drawing (the vertex shader collapses them).
- On log axes, non-positive coordinates are skipped the same way.
- `setValues` / non-empty `values` with a mismatched size → `qCWarning`, reject.

Marker shapes live on `PlotSeries` as `MarkerShape`, shared with `LineCurve`.
An earlier draft gave `PointCloud` its own enum so that QML could write
`QAccelPlot.PointCloud.Circle`, but QML resolves enums through the meta-object
chain, so a base-class enum is still reachable from the derived type. One
definition then removes any chance of the two lists drifting apart. `None` is
meaningless for a cloud, so `PointCloud` rejects it and keeps its current shape.

## 3. Rendering design

### 3.1 Buffers

| Buffer | Content | Rebuilt when |
| --- | --- | --- |
| Data texture A (RGBA8888, width 2048) | `x, y` floats (2 per point) | data changes |
| Data texture B (RGBA8888, width 2048) | `value` floats (1 per point) | values change, `ValueColor` only |
| Gradient texture (1D LUT, 256 texels) | resolved colormap | gradient stops change |
| Vertex buffer | `float id, float corner` per vertex, 6 per point (8 bytes) | point **count** changes |

The vertex buffer is deterministic from the count, exactly like
`RectangleList`, so streaming data at a fixed count is a pure texture upload
(`DataTextureUpload` in-place path when `QACCELPLOT_USE_QT_PRIVATE_API=ON`).

Memory at 1 M points: 8 MB positions + 4 MB values (CPU copies) and
48 MB vertex buffer. Phase 5 evaluates replacing the vertex attribute with
`gl_VertexIndex` (id = index / 6, corner = index % 6) to drop the vertex
buffer to a minimal placeholder.

Texture capacity: 2048 × `maxTextureSize` rows. Query
`QRhi::resourceLimit(TextureSizeMax)` once per window; above the limit, split
the cloud into multiple geometry nodes (one texture pair each, sharing the
same material settings). The first release may instead clamp and warn.

### 3.2 Shaders

- `shaders/point_shapes.glsl` — extract the SDF functions and the AA
  alpha computation from `point.frag` so `LineCurve` markers and `PointCloud`
  share one implementation (no visual change to `LineCurve`; covered by
  existing visual contracts).
- `shaders/point_cloud.vert` — fetch `x, y` via `data_texture.glsl`, apply log
  mapping (`math_utils.glsl`), map to item pixels, expand by `corner * markerSize`;
  in `ValueColor` fetch the value, normalize by `valueMin/valueMax`, pass `t` to
  the fragment shader. Collapse non-finite / invalid-log points to a degenerate
  position outside clip space.
- `shaders/point_cloud.frag` — `shapeSDF` + AA from the include, color from
  the uniform or `texture(gradientSampler, vec2(t, 0.5))`, premultiplied output.

A two-sampler vertex stage (positions + values) plus a fragment-stage LUT is
supported on every RHI backend QAccelPlot targets.

### 3.3 Material

`PointCloudMaterial : DataTextureMaterial` adds `valueTexture`,
`gradientTexture`, `markerSize`, `shapeType`, `colorMode`, `valueMin`,
`valueMax`, `antialiasingEnabled`, `antialiasingFeather`, `pointCount`.
Implement `compareExtra()`; extend `tst_material_comparison.cpp`. Reuse
`GradientTexture` for the LUT.

### 3.4 Blending and overdraw

Premultiplied alpha, painter's order = data order. Low-alpha colors give a
natural density impression; document that ordering is not depth-sorted.

## 4. Hover / hit testing

New `series/PointSpatialIndex.{hpp,cpp}`: uniform grid over data bounds,
cells store point indices in a CSR layout (`cellStart_`, `indices_`) to avoid
`std::vector<std::vector<int>>` allocation churn at 10⁶ points.

`nearest(dataX, dataY, radiusDataX, radiusDataY)` returns the closest valid
point within an axis-aligned ellipse. The pixel `hoverRadius` is converted to
data units with the current axis mapping (log axes: convert in log space),
so hit testing is correct at any zoom.

- Built on the UI thread after `setData*` — O(n). For `postData` at high rates,
  build lazily on the first hover event after a data change, and skip entirely
  when `QACCELPLOT_HOVER_ENABLED=0`.
- Optional follow-up: build the index on the posting thread and hand it over
  with the data (mirrors `setDataFNoRangeWithCache`).

## 5. Integration points

- `PlotSeries::LegendSymbol` gains `Marker`. `PointCloud` sets it in the
  constructor. `LegendSymbol.qml` shows only the marker sample when
  `isMarkerSymbol` (today it would draw a line because `lineStyle` is undefined).
- `QAccelPlot.cpp` discovers the item through the existing `PlotSeries` cast;
  no change expected — verify axis range aggregation with a mixed
  `LineCurve` + `PointCloud` plot.
- Value range: `DataRange` source uses the finite min/max of `values`,
  emitted through `valueRangeChanged` so a QML color bar can bind to it.

## 6. Work breakdown

| Phase | Deliverable | Tests |
| --- | --- | --- |
| 1 | `PointSpatialIndex` | `tst_point_spatial_index.cpp`: empty, single point, duplicates, nearest vs. brute force on 10⁴ random points, NaN skipping, anisotropic radius |
| 2 | `PointCloud` data model (no rendering): setters, validation, ranges, value range, `postData`, `pointAt`/`valueAt`, hover index | `tst_point_cloud_data.cpp`: signal spies for `countChanged`/range signals, rejected arguments, mismatched values, non-finite exclusion, log-axis exclusion, queued `postData` from a `std::thread`, hover env var |
| 3 | Shader include extraction; `PointCloudMaterial`; `point_cloud.vert/.frag`; `updatePaintNode` | `tst_material_comparison.cpp` extension; existing visual contracts must stay green (marker regression check) |
| 4 | `LegendSymbol.Marker` + `LegendSymbol.qml` | `tst_plot_appearance.cpp` legend case |
| 5 | Benchmarks: `benchmarks/common` scenario "point cloud 1 M static / 250 k streaming"; evaluate `gl_VertexIndex` vertex-buffer removal | benchmark regression workflow |
| 6 | Register the example (below), docs, changelog | smoke + AI visual contract |

### Phase 6 registration checklist

1. `examples/CMakeLists.txt`: `add_subdirectory(point_cloud)`
2. `examples/test/CMakeLists.txt`:
   `add_qaccelplot_example_visual_tests(QAccelPlotExamplePointCloud point_cloud)`
3. `examples/test/visual/ai_visual_matrix.py`: add
   `"QAccelPlotExamplePointCloud": "point_cloud"` to `TARGET_TO_CONTRACT`
4. `.github/scripts/ai_visual_impact.py`: add `"point_cloud"` to `EXAMPLES`
5. `README.md` examples table, `AGENTS.md` examples list, `llms.txt`
6. `docs/guide/cookbook/point-clouds.md` + `mkdocs.yml` nav, regenerate
   `llms-full.txt` (`scripts/generate-llms-full.py`)
7. `docs/guide/api.md` entry, `CHANGELOG.md` under `[Unreleased]`

## 7. Acceptance example (`examples/point_cloud`)

The example is written against the API above and doubles as the end-to-end
test. One 1200 × 800 screenshot exercises:

| Panel | Feature under test |
| --- | --- |
| **Clustered returns** (left, 250 k points default) | large count, `ValueColor` + `Gradient` LUT, fixed value range, translucency, legend `Marker` symbol, `count` property, hover tooltip via `hoveredIndex`/`pointAt`/`valueAt`, pan/zoom, `postData` streaming from a worker thread (Animate switch) |
| **Marker shapes** (top right) | all six `MarkerShape` values, uniform colors, legend entries per series, AA on small markers |
| **Power-law scatter** (bottom right) | log-log axes, non-positive and NaN points silently skipped, auto data range reporting to axes |

Manual checks while running it:

- Change the point count to 1 000 000 — UI stays responsive, count label updates.
- Toggle **Animate** — clusters rotate smoothly; frame rate does not collapse
  (the worker posts only after a presented frame, so queues cannot grow).
- Hover a point in the left panel — tooltip shows index, coordinates, value.
- Drag the marker size slider — rendering updates without re-uploading data.
- Toggle **Antialiasing** — edges visibly harden.
- Run with `QSG_RHI_BACKEND=d3d11`, `vulkan`, `opengl`.

Automated: `smoke_QAccelPlotExamplePointCloud` captures the screenshot, and
`examples/test/visual/contracts/point_cloud.json` defines the AI inspection.

## 8. Risks

| Risk | Mitigation |
| --- | --- |
| 6-vertex expansion memory at ≥ 1 M points | Phase 5 `gl_VertexIndex` evaluation; texture-capacity chunking |
| Float precision for timestamp-scale X | Document; follow-up origin rebasing like `LineCurve::renderOrigin` |
| Hover index rebuild cost during streaming | Lazy build, env var switch, worker-built index follow-up |
| Shared SDF extraction changes `LineCurve` markers | Existing visual contracts + side-by-side screenshot diff before merge |
| Vertex-stage texture fetch limits on old GLES 3.0 drivers | Same constraint already accepted by `LineCurve` / `RectangleList` |

## 9. Follow-ups

Per-point size channel, per-point RGBA mode, density/heat-map mode,
double-precision origin rebasing, lasso selection returning indices,
`DrawTransition` fade-in, level-of-detail decimation for > 10⁷ points.
