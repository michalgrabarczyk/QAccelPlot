//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "axis/Axis.hpp"
#include "effects/GradientColorTypes.hpp"
#include "linestyles/LineStyle.hpp"
#include "renderers/CurveRendererParams.hpp"

#include <QColor>
#include <QPointF>
#include <QSGGeometry>
#include <QSGNode>
#include <QVector2D>

#include <vector>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot {

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
};

/// \brief Vertex layout for line geometry, shared with the main thread for pre-built vertex caches.
struct LineVertex {
    float id;        ///< \brief Point index in the data buffer.
    float side;      ///< \brief Side of the line (-0.5 or +0.5) for screen-space extrusion.
    unsigned char r; ///< \brief Red channel (used when \c useVertexColor is true).
    unsigned char g; ///< \brief Green channel (used when \c useVertexColor is true).
    unsigned char b; ///< \brief Blue channel (used when \c useVertexColor is true).
    unsigned char a; ///< \brief Alpha channel (used when \c useVertexColor is true).
    float arcLength; ///< \brief Cumulative screen-space arc length in pixels; 0 for solid lines.
};

/// \brief Internal renderer responsible for building and updating QSGNode line geometry for a LineCurve.
///
/// Supports solid and dashed lines, gradient stroke, gradient fill, and optional pre-built
/// vertex caches assembled on the main thread for reduced render-thread stalls.
class LineCurveLineRenderer {
public:
    /// \brief Pre-builds the vertex buffer into \a cache on the calling thread (main thread safe).
    ///
    /// Writes neutral RGBA (1,1,1,1); solid and gradient shaders obtain their colors from their materials.
    void buildVertexCache(const std::vector<float>& data, int pointCount, std::vector<char>& cache) const;

    /// \brief Returns \c true if \a point lies within \a params.hitThreshold pixels of the curve.
    bool contains(const QPointF& point, const CurveHitTestParams& params) const;

    /// \brief Updates the scene graph node tree for this curve.
    ///
    /// When \a params.vertexCache is non-null the vertex data is transferred via a fast \c memcpy;
    /// otherwise geometry is built in-place on the render thread.
    QSGNode* paint(QSGNode* oldNode, const LineCurveRenderParams& params) const;

private:
    void updateFillGeometry(QSGGeometryNode* fillNode, const LineCurveRenderParams& params) const;
    void updateLineMaterial(
        LineMaterial* material, const LineCurveRenderParams& params, const QColor& effectiveColor, bool useVertexColor, const DashParameters& dashParams) const;
    std::vector<float> computeArcLengths(const LineCurveRenderParams& params, const DashParameters& dashParams) const;
    void updateLineVertices(QSGGeometry* geometry, const LineCurveRenderParams& params, bool useVertexColor, const QColor& effectiveColor,
        const std::vector<float>& arcLengths) const;
};

} // namespace QAccelPlot
