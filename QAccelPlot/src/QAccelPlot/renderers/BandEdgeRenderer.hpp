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

#include <QSGGeometryNode>
#include <QVector2D>

#include <memory>
#include <optional>
#include <vector>

namespace QAccelPlot {

class Axis;
class DataTexture;

/// \brief Read-only view over interleaved <tt>(x, low, high)</tt> band samples in float or double precision.
struct BandSamples {
    const float* floatData{nullptr};   ///< \brief Float samples, or \c nullptr when \c doubleData is set.
    const double* doubleData{nullptr}; ///< \brief Double samples, or \c nullptr when \c floatData is set.
    int count{0};                      ///< \brief Number of samples.

    /// \brief Returns value \a component (0 = x, 1 = low, 2 = high) of sample \a index without narrowing double data.
    qreal value(const int index, const int component) const
    {
        const auto offset = static_cast<std::size_t>(index) * 3 + static_cast<std::size_t>(component);
        return doubleData ? static_cast<qreal>(doubleData[offset]) : static_cast<qreal>(floatData[offset]);
    }
};

/// \brief Inputs for BandEdgeRenderer::paint(), assembled while the GUI thread is blocked.
struct BandEdgeRenderParams {
    std::shared_ptr<DataTexture> dataTexture; ///< \brief The band's data texture; the edge line never uploads data itself.
    BandSamples samples;                      ///< \brief CPU copy of the band samples, used for dash lengths.
    LineStroke::Uniforms uniforms;            ///< \brief Line uniforms; \c uniforms.pointCount equals \c samples.count.
    Axis* xAxis;                              ///< \brief Horizontal axis.
    Axis* yAxis;                              ///< \brief Vertical axis.
    bool dataChanged;                         ///< \brief Whether the samples changed since the last paint.
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
