//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/GradientColorTypes.hpp"
#include "QAccelPlot/renderers/CurveRendererParams.hpp"

#include <QColor>
#include <QPointF>
#include <QSGNode>
#include <QVector2D>

#include <memory>
#include <vector>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot {

class DataTexture;

/// \brief Input parameters for LineCurvePointRenderer::paint(), assembled on the main thread.
struct PointCurveRenderParams {
    QQuickWindow* window;                        ///< \brief Window providing the texture upload context.
    const std::vector<float>& data;              ///< \brief GPU-ready interleaved XY float data.
    int pointCount;                              ///< \brief Number of points in \c data.
    bool dataChanged;                            ///< \brief Whether data has changed since the last frame.
    std::shared_ptr<DataTexture> dataTexture;    ///< \brief Texture that already holds \c data, or \c nullptr to upload \c data.
    QColor color;                                ///< \brief Base marker color.
    bool hovered;                                ///< \brief Whether the curve is currently hovered.
    qreal markerSize;                            ///< \brief Marker radius in pixels.
    QVector2D domainMin;                         ///< \brief Minimum data-space coordinate.
    QVector2D domainMax;                         ///< \brief Maximum data-space coordinate.
    QVector2D viewportSize;                      ///< \brief Viewport size in pixels.
    bool logScaleX;                              ///< \brief Whether the X axis uses log scale.
    bool logScaleY;                              ///< \brief Whether the Y axis uses log scale.
    bool antialiasingEnabled;                    ///< \brief Whether GPU-side anti-aliasing is active.
    qreal antialiasingFeather;                   ///< \brief Anti-aliasing feather width in pixels.
    const GradientColorPayload& gradientPayload; ///< \brief Gradient color parameters, with value bounds in the coordinates of \c data.
    const std::vector<char>* vertexCache;        ///< \brief Pre-built vertex buffer, or \c nullptr.
    int shapeType;                               ///< \brief Marker shape index (\c MarkerShape value minus one).
    qreal markerStrokeWidth;                     ///< \brief Outline width in pixels for hollow markers.
    bool markerFilled;                           ///< \brief Whether closed marker shapes are filled rather than outlined.
};

/// \brief Placeholder vertex of a marker quad, shared with the main thread for vertex caches.
///
/// The marker shader reads the point from the data texture and derives the point index and quad
/// corner from the vertex index, so it ignores the vertex content.
struct PointVertex {
    unsigned char padding[4]; ///< \brief Ignored; written as zero.
};

/// \brief Internal renderer responsible for building and updating QSGNode marker geometry for a LineCurve.
///
/// Renders per-point markers as two triangles per point. The vertex buffer depends only on the
/// point count, so data updates of the same size only upload the data texture.
class LineCurvePointRenderer {
public:
    /// \brief Number of vertices drawn per marker.
    static constexpr int kVerticesPerPoint{6};

    /// \brief Pre-builds the vertex buffer of \a pointCount markers into \a cache on the calling thread (main thread safe).
    ///
    /// The cache depends only on \a pointCount; \a data is not read.
    void buildVertexCache(const std::vector<float>& data, int pointCount, std::vector<char>& cache) const;

    /// \brief Returns \c true if \a point lies within \a params.hitThreshold pixels of any marker.
    bool contains(const QPointF& point, const CurveHitTestParams& params) const;

    /// \brief Updates the scene graph node tree for the point markers.
    ///
    /// The vertex buffer is written only when the point count changes, from \a params.vertexCache
    /// when it matches.
    QSGNode* paint(QSGNode* oldNode, const PointCurveRenderParams& params) const;
};

} // namespace QAccelPlot
