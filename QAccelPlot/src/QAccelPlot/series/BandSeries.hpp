//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/renderers/BandEdgeRenderer.hpp"
#include "QAccelPlot/series/BandEdges.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"

#include <QList>
#include <QVariantMap>
#include <QVector2D>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <memory>
#include <optional>
#include <vector>

namespace QAccelPlot {

/// \brief A hardware-accelerated QML item that fills the area between a low and a high value at each X.
///
/// Data points are stored as interleaved <tt>(x, low, high)</tt> triples and uploaded to the GPU as a float
/// data texture once per data change; panning and zooming only change shader uniforms. Dashed edge lines
/// are the exception: zooming recomputes their dash positions on the CPU. The \c setData()
/// overloads keep doubles and upload them relative to an origin near the viewport, so large coordinates
/// such as epoch timestamps stay precise. The \c setDataF() overloads store floats and upload them without
/// conversion.
///
/// Draw a center line with a separate \c LineCurve. \c edges draws lines along the lower and upper edges of
/// the filled band.
///
/// \par Data points
/// At each data point the band spans from the smaller to the larger of \c low and \c high. X values are
/// expected in ascending order: \c hovered and \c valueAt() work only when they are.
///
/// \par Invalid data points
/// A data point is invalid when \c x, \c low, or \c high is NaN or ±Inf, or is not strictly positive on a
/// log-scale axis. The fill and the edge lines leave a gap at invalid data points. Auto-ranging skips invalid
/// values one by one.
///
/// \par Limits
/// At most 16,777,216 (2^24) data points are drawn, fewer on GPUs whose maximum texture size is below 6144.
/// Data points beyond the limit are not drawn, and a warning is logged once.
///
/// \sa LineCurve, BandEdges
class BandSeries : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(BandSeries)

    /// \brief Fill color. Default: \c Colors.dark.seriesPrimary with alpha 64.
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /// \brief Grouped edge line settings, e.g. <tt>edges.width</tt> and <tt>edges.lineStyle</tt>. No edge lines by default.
    Q_PROPERTY(BandEdges* edges READ edges CONSTANT)
    /// \brief Read-only: number of data points currently loaded.
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    /// \brief Read-only: \c true while the mouse cursor is inside the band.
    Q_PROPERTY(bool hovered READ hovered NOTIFY hoveredChanged)

public:
    /// \brief Constructs a BandSeries with the given \a parent.
    explicit BandSeries(QQuickItem* parent = nullptr);

    /// \brief Returns the fill color.
    QColor color() const;
    /// \brief Sets the fill color to \a color.
    void setColor(const QColor& color);

    /// \brief Returns the grouped edge line settings. The object is owned by the series.
    BandEdges* edges() const;

    /// \brief Returns the number of data points currently loaded.
    int count() const;

    /// \brief Returns \c true while the cursor is inside the band.
    bool hovered() const;

    /// \brief Appends the data point (\a x, \a low, \a high). Triggers a redraw.
    Q_INVOKABLE void appendData(qreal x, qreal low, qreal high);
    /// \brief Replaces the data points with \a xs, \a lows, and \a highs. If the sizes differ, the shortest length is used.
    Q_INVOKABLE void setData(const QList<qreal>& xs, const QList<qreal>& lows, const QList<qreal>& highs);
    /// \brief Replaces the data points with \a xs, \a lows, and \a highs. If the sizes differ, the shortest length is used.
    void setData(const std::vector<double>& xs, const std::vector<double>& lows, const std::vector<double>& highs);
    // The PlotSeries overloads take interleaved (x, low, high) triples, as doubles or floats.
    using PlotSeries::setData;
    using PlotSeries::setDataF;
    /// \brief Removes all data points.
    Q_INVOKABLE void clearData() override;
    /// \brief Returns the legend entry: the symbol and color of the band.
    std::optional<LegendEntry> legendEntry() const override;

    /// \brief Returns the band at data coordinate \a x as an object with \c x, \c low, and \c high properties.
    ///
    /// \c low and \c high are interpolated between the neighboring data points as drawn, with \c low ≤ \c high.
    /// At an X shared by several data points they cover all of them. Returns an empty object when \a x lies outside the data points, next to an invalid data
    /// point, or when
    /// the X values are not in ascending order.
    Q_INVOKABLE QVariantMap valueAt(qreal x) const;

    /// \brief Returns \c true when item position \a point lies inside the band or on an edge line.
    ///
    /// Hover delivery uses this test, so stacked series underneath still receive hover events
    /// outside the band.
    bool contains(const QPointF& point) const override;

signals:
    /// \brief Emitted when the color property changes.
    void colorChanged();
    /// \brief Emitted when the data point count changes.
    void countChanged();
    /// \brief Emitted when the hovered property changes.
    void hoveredChanged();

protected:
    /// \cond INTERNAL
    bool recordInspectionSupported() const override;
    InspectionRecord inspectionRecord(int index) const override;
    InspectionRecord inspectionRecordAt(const QPointF& position) const override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    /// \endcond
    /// \brief Re-tests the hover position when a bound axis changes between linear and log scale.
    void onAxisScaleChanged() override;
    /// \brief Re-tests the hover position when the viewport moves under the cursor.
    void onAxisRangeChanged() override;
    /// \brief Refreshes the X order, the hover state, and the count after the data points changed.
    void onDataPointsChanged(DataChange change, int previousCount) override;

private:
    // The drawn band at one X: the smaller and larger interpolated value.
    struct Span {
        qreal low;
        qreal high;
    };

    // What the fill and the edge lines draw this frame: viewport uniforms relative to the render
    // origin, and the number of data points within the GPU capacity.
    struct RenderView {
        QVector2D domainMin;
        QVector2D domainMax;
        QVector2D viewportSize;
        int drawnCount;
        // Vertex buffer room, grown in powers of two so that appends rarely rebuild the vertices.
        int reservedCount;
    };

    // Applies (x, low, high) triples built from separate lists.
    void applyInterleavedData(std::vector<double>&& data);
    // Returns value \a component (0 = x, 1 = low, 2 = high) of data point \a index.
    double value(int index, int component) const;
    bool dataPointValid(int index) const;
    void updateXAscending();
    // Returns the band at x as drawn from the first dataPointCount data points.
    std::optional<Span> spanAt(qreal x, int dataPointCount) const;
    bool edgesVisible() const;
    QColor edgeColor() const;
    void setHovered(bool hovered);
    // Re-tests the last hover position after the data or the axes moved under the cursor.
    void refreshHovered();
    // Follows edits of the current edge line style, such as a new dash pattern.
    void onEdgeLineStyleChanged();
    RenderView renderView();
    void updateFillNode(QSGGeometryNode* node, const RenderView& view, bool dataChanged);
    QSGGeometryNode* paintEdge(QSGGeometryNode* oldNode, const BandEdgeRenderer& renderer, const RenderView& view,
        const std::shared_ptr<DataTexture>& dataTexture, bool dataChanged) const;

    QColor color_;
    BandEdges* edges_{new BandEdges{this}};
    bool hovered_{false};
    // Cursor position while hover events arrive, in item coordinates.
    std::optional<QPointF> hoverPosition_;
    // Render revision of the data points last uploaded to the GPU.
    std::uint64_t uploadedRevision_{0};
    bool xAscending_{true};
    QMetaObject::Connection edgeStyleConnection_;
    BandEdgeRenderer lowerEdgeRenderer_{BandEdgeMaterial::Edge::Lower};
    BandEdgeRenderer upperEdgeRenderer_{BandEdgeMaterial::Edge::Upper};
};

} // namespace QAccelPlot
