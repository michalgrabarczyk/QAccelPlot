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
/// Samples are stored as interleaved <tt>(x, low, high)</tt> triples and uploaded to the GPU as a float
/// data texture once per data change; panning and zooming only change shader uniforms. Dashed edge lines
/// are the exception: zooming recomputes their dash positions on the CPU. The \c setData()
/// overloads keep doubles and upload them relative to an origin near the viewport, so large coordinates
/// such as epoch timestamps stay precise. The \c setDataF() overloads store floats and upload them without
/// conversion.
///
/// Draw a center line with a separate \c LineCurve. \c edges draws lines along the lower and upper edges of
/// the filled band.
///
/// \par Samples
/// At each sample the band spans from the smaller to the larger of \c low and \c high. X values are
/// expected in ascending order: \c hovered and \c valueAt() work only when they are.
///
/// \par Invalid samples
/// A sample is invalid when \c x, \c low, or \c high is NaN or ±Inf, or is not strictly positive on a
/// log-scale axis. The fill and the edge lines leave a gap at invalid samples. Auto-ranging skips invalid
/// values one by one.
///
/// \par Limits
/// At most 16,777,216 (2^24) samples are drawn, fewer on GPUs whose maximum texture size is below 6144.
/// Samples beyond the limit are not drawn, and a warning is logged once.
///
/// \sa LineCurve, BandEdges
class BandSeries : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(BandSeries)

    /// \brief Fill color. Default: \c Colors.dark.seriesPrimary with alpha 64.
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /// \brief Grouped edge line settings, e.g. <tt>edges.width</tt> and <tt>edges.lineStyle</tt>. No edge lines by default.
    Q_PROPERTY(BandEdges* edges READ edges CONSTANT)
    /// \brief Read-only: number of samples currently loaded.
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

    /// \brief Returns the number of samples currently loaded.
    int count() const;

    /// \brief Returns \c true while the cursor is inside the band.
    bool hovered() const;

    /// \brief Appends the sample (\a x, \a low, \a high). Triggers a redraw.
    Q_INVOKABLE void appendData(qreal x, qreal low, qreal high);
    /// \brief Replaces the samples with \a xs, \a lows, and \a highs. If the sizes differ, the shortest length is used.
    Q_INVOKABLE void setData(const QList<qreal>& xs, const QList<qreal>& lows, const QList<qreal>& highs);
    /// \brief Replaces the samples with \a xs, \a lows, and \a highs. If the sizes differ, the shortest length is used.
    void setData(const std::vector<double>& xs, const std::vector<double>& lows, const std::vector<double>& highs);
    /// \brief Copies \a sampleCount interleaved <tt>(x, low, high)</tt> double triples, retaining full precision.
    void setData(const double* data, int sampleCount) override;
    /// \brief Moves \a data (\a sampleCount × 3 doubles: x, low, high) into the series. No copy is made.
    void setData(std::vector<double>&& data, int sampleCount) override;
    /// \brief Like \c setDataNoRange(vector) but copies from a raw interleaved double array.
    void setDataNoRange(const double* data, int sampleCount) override;
    /// \brief Like \c setData(vector) but does not report X/Y data ranges to the axes.
    void setDataNoRange(std::vector<double>&& data, int sampleCount) override;
    /// \brief High-performance C++ overload: copies \a sampleCount × 3 floats (x, low, high) from \a data.
    void setDataF(const float* data, int sampleCount) override;
    /// \brief High-performance C++ overload: moves \a data (\a sampleCount × 3 floats) into the series.
    void setDataF(std::vector<float>&& data, int sampleCount) override;
    /// \brief Like \c setDataFNoRange(vector) but copies from a raw interleaved float array.
    void setDataFNoRange(const float* data, int sampleCount) override;
    /// \brief Like \c setDataF(vector) but does not report X/Y data ranges to the axes.
    void setDataFNoRange(std::vector<float>&& data, int sampleCount) override;
    /// \brief Thread-safe: queues \c setData(\a data, \a sampleCount) to the item's thread. No copy is made.
    void postData(std::vector<double>&& data, int sampleCount) override;
    /// \brief Thread-safe: queues \c setDataF(\a data, \a sampleCount) to the item's thread. No copy is made.
    void postData(std::vector<float>&& data, int sampleCount) override;
    /// \brief Removes all samples.
    Q_INVOKABLE void clearData() override;

    /// \brief Returns the band at data coordinate \a x as an object with \c x, \c low, and \c high properties.
    ///
    /// \c low and \c high are interpolated between the neighboring samples as drawn, with \c low ≤ \c high.
    /// Returns an empty object when \a x lies outside the samples, next to an invalid sample, or when
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
    /// \brief Emitted when the sample count changes.
    void countChanged();
    /// \brief Emitted when the hovered property changes.
    void hoveredChanged();

protected:
    /// \cond INTERNAL
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    /// \endcond
    /// \brief Refreshes ranges and uploaded coordinates when a bound axis changes between linear and log scale.
    void onAxisScaleChanged() override;
    /// \brief Moves the render origin to the new viewport when the float render data would lose precision there.
    void onAxisRangeChanged() override;

private:
    // The drawn band at one X: the smaller and larger interpolated value.
    struct Span {
        qreal low;
        qreal high;
    };

    // What the fill and the edge lines draw this frame: viewport uniforms relative to the render
    // origin, and the number of samples within the GPU capacity.
    struct RenderView {
        QVector2D domainMin;
        QVector2D domainMax;
        QVector2D viewportSize;
        int drawnSampleCount;
    };

    bool validateRawDataArguments(const void* data, int sampleCount) const;
    bool validateVectorArguments(std::size_t valueCount, int sampleCount) const;
    void applyData(std::vector<double>&& data, int sampleCount, bool reportRanges);
    void applyFloatData(std::vector<float>&& data, int sampleCount, bool reportRanges);
    void finishDataChange(int sampleCount, bool reportRanges);
    void promoteFloatDataToDouble();
    // True when the double setData() overloads supplied the data; false for the setDataF() overloads.
    bool hasPreciseData() const;
    // Returns value \a component (0 = x, 1 = low, 2 = high) of sample \a index.
    double value(int index, int component) const;
    bool sampleValid(int index) const;
    bool logScaleX() const;
    bool logScaleY() const;
    void updateDataRanges();
    void updateXAscending();
    std::optional<Span> spanAt(qreal x) const;
    bool edgesVisible() const;
    QColor edgeColor() const;
    void setHovered(bool hovered);
    // Follows edits of the current edge line style, such as a new dash pattern.
    void onEdgeLineStyleChanged();
    // Rebuilds renderData_ (origin-relative floats) from data_ for the current axes.
    void rebuildRenderData();
    RenderView renderView() const;
    void updateFillNode(QSGGeometryNode* node, const RenderView& view);
    QSGGeometryNode* paintEdge(
        QSGGeometryNode* oldNode, const BandEdgeRenderer& renderer, const RenderView& view, const std::shared_ptr<DataTexture>& dataTexture) const;

    // Which setter family supplied the data: setData() keeps doubles, setDataF() keeps floats.
    enum class DataType { Double, Float };

    QColor color_;
    BandEdges* edges_{new BandEdges{this}};
    bool hovered_{false};
    DataType dataType_{DataType::Double};
    // Data: 3 doubles per sample (x, low, high), full precision. Empty for DataType::Float.
    std::vector<double> data_;
    // Uploaded to the GPU: an origin-relative float mirror of data_, or the setDataF() data itself.
    std::vector<float> renderData_;
    qreal renderOriginX_{0.0};
    qreal renderOriginY_{0.0};
    // True when renderData_ matches data_, the origin, and the axis scales it was built for.
    bool renderDataValid_{false};
    bool renderLogScaleX_{false};
    bool renderLogScaleY_{false};
    // False while no finite value fixed the origin, so appended samples must rebuild renderData_.
    bool renderOriginSettled_{false};
    int sampleCount_{0};
    bool dataChanged_{true};
    bool xAscending_{true};
    // True when the most recent data update computed ranges; the NoRange APIs leave range
    // management to the caller, so scale changes must not overwrite it.
    bool autoDataRanges_{true};
    QMetaObject::Connection edgeStyleConnection_;
    BandEdgeRenderer lowerEdgeRenderer_{BandEdgeMaterial::Edge::Lower};
    BandEdgeRenderer upperEdgeRenderer_{BandEdgeMaterial::Edge::Upper};
};

} // namespace QAccelPlot
