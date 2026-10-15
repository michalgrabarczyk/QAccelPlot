//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/materials/BandEdgeMaterial.hpp"
#include "QAccelPlot/renderers/LineStroke.hpp"
#include "QAccelPlot/series/DataPointStore.hpp"

#include <QSGGeometryNode>
#include <QVector2D>

#include <memory>
#include <optional>
#include <vector>

namespace QAccelPlot {

class Axis;
class DataTexture;

/// \brief Inputs for BandEdgeRenderer::paint(), assembled while the GUI thread is blocked.
struct BandEdgeRenderParams {
    std::shared_ptr<DataTexture> dataTexture; ///< \brief The band's data texture; the edge line never uploads data itself.
    DataPointStore::View dataPoints;          ///< \brief The band's <tt>(x, low, high)</tt> data points, used for dash lengths.
    LineStroke::Uniforms uniforms;            ///< \brief Line uniforms; \c uniforms.pointCount equals \c dataPoints.count.
    Axis* xAxis;                              ///< \brief Horizontal axis.
    Axis* yAxis;                              ///< \brief Vertical axis.
    bool dataChanged;                         ///< \brief Whether the data points changed since the last paint.
    /// \brief Data points the vertex buffer holds room for, at least \c dataPoints.count, so appends rarely resize it.
    int reservedCount;
};

/// \brief Internal renderer for the lower or upper edge line of a \c BandSeries.
///
/// Draws one line ribbon node that samples the band's data texture through a \c BandEdgeMaterial.
class BandEdgeRenderer {
public:
    /// \brief Constructs a renderer for the \a edge line.
    explicit BandEdgeRenderer(BandEdgeMaterial::Edge edge);

    /// \brief Updates \a oldNode, or creates the edge line node when it is \c nullptr, and returns it.
    QSGGeometryNode* paint(QSGGeometryNode* oldNode, const BandEdgeRenderParams& params) const;

private:
    // The zoom state dash arc lengths depend on. Panning leaves it unchanged; spans are in log
    // space on log-scale axes.
    struct ArcLengthScale {
        qreal xSpan;
        qreal ySpan;
        QVector2D viewportSize;
        bool logScaleX;
        bool logScaleY;

        bool matches(const ArcLengthScale& other) const;
    };

    static ArcLengthScale arcLengthScale(const BandEdgeRenderParams& params);
    std::vector<float> arcLengths(const BandEdgeRenderParams& params) const;

    BandEdgeMaterial::Edge edge_;
    // Render-thread state touched only by paint(): the scale of the arc lengths in the vertex
    // buffer, or nothing when it holds none.
    mutable std::optional<ArcLengthScale> arcLengthScale_;
};

} // namespace QAccelPlot
