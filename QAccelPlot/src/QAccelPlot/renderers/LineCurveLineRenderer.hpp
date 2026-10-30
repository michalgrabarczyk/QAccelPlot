//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/GradientColorTypes.hpp"
#include "QAccelPlot/linestyles/LineStyle.hpp"
#include "QAccelPlot/renderers/CurveRendererParams.hpp"
#include "QAccelPlot/renderers/LineStroke.hpp"
#include "QAccelPlot/series/LineCurveGapFilter.hpp"

#include <QColor>
#include <QPointF>
#include <QSGGeometry>
#include <QSGNode>
#include <QVector2D>

#include <cstddef>
#include <memory>
#include <vector>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot {

class DataTexture;
struct DashParameters;
class LineMaterial;

/// \brief Input parameters for LineCurveLineRenderer::paint(), assembled on the main thread.
struct LineCurveRenderParams {
    QQuickWindow* window;                           ///< \brief Window providing the texture upload context.
    const std::vector<float>& data;                 ///< \brief GPU-ready interleaved XY float data.
    CurveDataView sourceData;                       ///< \brief Original float or double data used by CPU-side operations.
    int pointCount;                                 ///< \brief Number of points in \c data.
    bool dataChanged;                               ///< \brief Whether the data buffer has changed since the last frame.
    QColor color;                                   ///< \brief Base line color.
    bool hovered;                                   ///< \brief Whether the curve is currently hovered.
    qreal lineWidth;                                ///< \brief Line width in pixels.
    QVector2D domainMin;                            ///< \brief Minimum data-space coordinate.
    QVector2D domainMax;                            ///< \brief Maximum data-space coordinate.
    QVector2D viewportSize;                         ///< \brief Viewport size in pixels.
    Axis* xAxis;                                    ///< \brief Horizontal axis.
    Axis* yAxis;                                    ///< \brief Vertical axis.
    bool logScaleX;                                 ///< \brief Whether the X axis uses log scale.
    bool logScaleY;                                 ///< \brief Whether the Y axis uses log scale.
    bool antialiasingEnabled;                       ///< \brief Whether GPU-side anti-aliasing is active.
    qreal antialiasingFeather;                      ///< \brief Anti-aliasing feather width in pixels.
    const GradientColorPayload& gradientPayload;    ///< \brief Gradient stroke parameters.
    const GradientFillPayload& gradientFillPayload; ///< \brief Gradient fill parameters.
    const std::vector<char>* vertexCache;           ///< \brief Pre-built vertex buffer, or \c nullptr to build on the render thread.
    const LineStyle* lineStyle;                     ///< \brief Active line style (dash pattern etc.).
    StepMode stepMode{StepMode::None};              ///< \brief How the line joins consecutive data points.
};

/// \brief Outline of a gradient fill in data space: one group of points per run of valid data points, broken at gaps.
///
/// Each point is a sampled data point or, with a step mode, a step corner.
struct FillSamples {
    std::vector<int> pointCounts; ///< \brief Number of points of each run; zero for runs too short to fill.
    std::vector<QPointF> points;  ///< \brief Points of all runs, run after run.
};

/// \brief Internal renderer responsible for building and updating QSGNode line geometry for a LineCurve.
///
/// Supports solid and dashed lines, gradient stroke, gradient fill, and optional pre-built
/// vertex caches assembled on the main thread for reduced render-thread stalls.
class LineCurveLineRenderer {
public:
    /// \brief Returns the vertex cache bytes per data point of a line drawn with \a stepMode.
    static std::size_t vertexCacheBytesPerPoint(StepMode stepMode);

    /// \brief Writes the vertices of data points [\a firstPoint, \a pointCount) of a line drawn with \a stepMode into \a cache.
    ///
    /// \a cache holds vertexCacheBytesPerPoint() bytes for each of \a pointCount data points. A vertex depends only on its
    /// data point index and step corner, never on data values, so the vertices before \a firstPoint stay valid.
    /// Writes neutral RGBA (1,1,1,1); solid and gradient shaders obtain their colors from their materials.
    static void writeVertexCache(std::vector<char>& cache, int firstPoint, int pointCount, StepMode stepMode);

    /// \brief Returns \c true if \a point lies within \a params.hitThreshold pixels of the curve.
    bool contains(const QPointF& point, const CurveHitTestParams& params) const;

    /// \brief Updates the scene graph node tree for this curve.
    ///
    /// When \a params.vertexCache is non-null the vertex data is transferred via a fast \c memcpy;
    /// otherwise geometry is built in-place on the render thread.
    QSGNode* paint(QSGNode* oldNode, const LineCurveRenderParams& params) const;

    /// \brief Returns the data texture of the line in \a node, a tree returned by paint(), or \c nullptr before an upload.
    static std::shared_ptr<DataTexture> dataTexture(QSGNode* node);

private:
    // Gradient fill data points, cached across frames because they depend only on the data.
    struct FillSampleCache {
        const void* data{nullptr};
        int pointCount{-1};
        bool logScaleX{false};
        bool logScaleY{false};
        StepMode stepMode{StepMode::None};
        FillSamples samples;
    };

    void updateFillGeometry(QSGGeometryNode* fillNode, const LineCurveRenderParams& params) const;
    const FillSamples& fillSamples(const LineCurveRenderParams& params) const;
    /// \brief Rebuilds \a samples: the fill outline of every run of valid data points, decimated to the fill vertex budget.
    static void rebuildFillSamples(const LineCurveRenderParams& params, FillSamples& samples);
    void updateLineMaterial(
        LineMaterial* material, const LineCurveRenderParams& params, const QColor& effectiveColor, bool useVertexColor, const DashParameters& dashParams) const;
    std::vector<float> computeArcLengths(const LineCurveRenderParams& params, const DashParameters& dashParams) const;
    void updateLineVertices(QSGGeometry* geometry, const LineCurveRenderParams& params, bool useVertexColor, const QColor& effectiveColor,
        const std::vector<float>& arcLengths) const;

    // Render-thread state touched only by paint(); workers that call buildVertexCache() never read it.
    mutable FillSampleCache fillSampleCache_;
};

} // namespace QAccelPlot
