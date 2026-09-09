//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "effects/GradientColorTypes.hpp"
#include "renderers/CurveRendererParams.hpp"

#include <QColor>
#include <QPointF>
#include <QSGNode>
#include <QVector2D>

#include <vector>

namespace QAccelPlot {

/// \brief Input parameters for LineCurvePointRenderer::paint(), assembled on the main thread.
struct PointCurveRenderParams {
    const std::vector<float>& data;              ///< \brief Interleaved XY float data.
    int pointCount;                              ///< \brief Number of points in \c data.
    bool dataChanged;                            ///< \brief Whether data has changed since the last frame.
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
    const GradientColorPayload& gradientPayload; ///< \brief Gradient color parameters.
    const std::vector<char>* vertexCache;        ///< \brief Pre-built vertex buffer, or \c nullptr.
    int shapeType;                               ///< \brief Marker shape index (matches \c LineCurve::PointShape).
};

/// \brief Vertex layout for point (marker) geometry, shared with the main thread for vertex caches.
struct PointVertex {
    float x;       ///< \brief Data-space X coordinate.
    float y;       ///< \brief Data-space Y coordinate.
    float cornerX; ///< \brief Screen-space X corner offset for quad expansion.
    float cornerY; ///< \brief Screen-space Y corner offset for quad expansion.
    float r;       ///< \brief Red channel.
    float g;       ///< \brief Green channel.
    float b;       ///< \brief Blue channel.
    float a;       ///< \brief Alpha channel.
};

/// \brief Internal renderer responsible for building and updating QSGNode marker geometry for a LineCurve.
///
/// Renders per-point markers using a single GPU-side quad per point. Supports pre-built
/// vertex caches assembled on the main thread.
class LineCurvePointRenderer {
public:
    /// \brief Pre-builds the vertex buffer into \a cache on the calling thread (main thread safe).
    void buildVertexCache(const std::vector<float>& data, int pointCount, std::vector<char>& cache) const;

    /// \brief Returns \c true if \a point lies within \a params.hitThreshold pixels of any marker.
    bool contains(const QPointF& point, const CurveHitTestParams& params) const;

    /// \brief Updates the scene graph node tree for the point markers.
    ///
    /// When \a params.vertexCache is non-null the vertex data is transferred via a fast \c memcpy.
    QSGNode* paint(QSGNode* oldNode, const PointCurveRenderParams& params) const;
};

} // namespace QAccelPlot
