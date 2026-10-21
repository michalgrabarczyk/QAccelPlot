//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/series/PlotSeries.hpp"
#include "QAccelPlot/series/SpatialGrid.hpp"

#include <QColor>
#include <QList>
#include <QVariantMap>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

class QSGGeometryNode;

namespace QAccelPlot {

class ErrorBarMaterial;

/// \brief A hardware-accelerated QML item that draws error bars: a line from a low to a high bound at each data point, capped at both ends.
///
/// Each data point is an <tt>(x, y, low, high)</tt> quadruple. \c orientation selects the axis that
/// \c low and \c high lie on: a vertical error bar is drawn at \c x from \c low to \c high along the
/// Y axis, and a horizontal one at \c y from \c low to \c high along the X axis. The other coordinate,
/// \c y for vertical and \c x for horizontal error bars, is the value the bounds belong to. The series
/// does not draw it: draw the values with a \c PointCloud or \c LineCurve declared after the error
/// bars, so that its markers are drawn and hovered on top. The same \c xs and \c ys then feed both
/// series.
///
/// \c setErrorData() and \c appendErrorData() take errors relative to the value instead of bounds,
/// one for both sides or one per side, and store the bounds they give. A NaN error gives a NaN bound.
///
/// Data points are uploaded to the GPU as a float data texture once per data change; panning,
/// zooming, and changing \c lineWidth, \c capLength, or \c offset only change shader uniforms. The
/// \c setData() overloads keep doubles and upload them relative to an origin near the viewport, so
/// large coordinates such as epoch timestamps stay precise. The \c setDataF() overloads store floats
/// and upload them without conversion.
///
/// \code
/// QAccelPlot.ErrorBarSeries {
///     xAxis: plot.xAxis
///     yAxis: plot.yAxis
///     Component.onCompleted: setErrorData([1, 2, 3], [4.2, 5.1, 4.7], [0.3, 0.5, 0.2])
/// }
/// \endcode
///
/// \par Invalid data points
/// A data point is not drawn or hovered when its position (\c x for vertical, \c y for horizontal
/// error bars) is NaN or ±Inf, or not strictly positive on a log-scale axis, or when its value is NaN.
/// A NaN bound leaves out that half of the error bar, which then runs from the value to the other
/// bound. An infinite bound, and a bound that is not strictly positive on a log-scale axis, extends
/// the error bar past the plot edge without a cap. \c low may be above \c high. Auto-ranging counts
/// every valid coordinate on its own.
///
/// \par Hover
/// \c hoveredIndex is the error bar within \c hoverRadius pixels of the cursor, the nearest one when
/// several are, and the one with the highest index on a tie. \c hoverColor draws it. Only the topmost
/// series under the cursor is hovered: set \c hoverEnabled to \c false to leave hover to the series
/// beneath.
///
/// \par Limits
/// At most 16,777,216 (2^24) error bars are drawn, fewer on GPUs whose maximum texture size is below
/// 8192. Error bars beyond the limit are not drawn or hovered, and a warning is logged once.
///
/// \sa PointCloud, LineCurve, BandSeries
class ErrorBarSeries : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(ErrorBarSeries)

    /// \brief Axis the bounds lie on: \c Qt.Vertical for the Y axis, \c Qt.Horizontal for the X axis. Default: \c Qt.Vertical.
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    /// \brief Line color. Default: \c Colors.dark.seriesPrimary.
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /// \brief Width in pixels of the error bars and their caps, clamped to at least 0. Default: 1.5.
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    /// \brief Length in pixels of the caps across the error bar, clamped to at least 0; 0 hides them. Default: 8.
    Q_PROPERTY(qreal capLength READ capLength WRITE setCapLength NOTIFY capLengthChanged)
    /// \brief Shift of every error bar across its bounds, in data units of the other axis. Default: 0.
    ///
    /// Bind it to \c BarSeries::barOffset to center error bars on grouped bars. Auto-ranging ignores it.
    Q_PROPERTY(qreal offset READ offset WRITE setOffset NOTIFY offsetChanged)
    /// \brief Color of the hovered error bar. Default: \c Colors.dark.hover; an invalid color draws it in \c color.
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    /// \brief Pick distance in pixels from an error bar or its caps, clamped to at least 0. Default: 6.
    Q_PROPERTY(qreal hoverRadius READ hoverRadius WRITE setHoverRadius NOTIFY hoverRadiusChanged)
    /// \brief Read-only: number of data points currently loaded.
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    /// \brief Read-only: index of the error bar under the cursor, or -1 when none.
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)

public:
    /// \brief Constructs an ErrorBarSeries with the given \a parent.
    explicit ErrorBarSeries(QQuickItem* parent = nullptr);
    ~ErrorBarSeries() override;

    /// \brief Returns the axis the bounds lie on.
    Qt::Orientation orientation() const;
    /// \brief Sets the axis the bounds lie on to \a orientation. The stored data points are kept.
    void setOrientation(Qt::Orientation orientation);

    /// \brief Returns the line color.
    QColor color() const;
    /// \brief Sets the line color to \a color.
    void setColor(const QColor& color);

    /// \brief Returns the line width in pixels.
    qreal lineWidth() const;
    /// \brief Sets the line width to \a width pixels. Negative values are clamped to 0.
    void setLineWidth(qreal width);

    /// \brief Returns the cap length in pixels.
    qreal capLength() const;
    /// \brief Sets the cap length to \a length pixels. Negative values are clamped to 0.
    void setCapLength(qreal length);

    /// \brief Returns the shift of every error bar across its bounds, in data units.
    qreal offset() const;
    /// \brief Sets the shift of every error bar across its bounds to \a offset data units.
    void setOffset(qreal offset);

    /// \brief Returns the color of the hovered error bar.
    QColor hoverColor() const;
    /// \brief Sets the color of the hovered error bar to \a color.
    void setHoverColor(const QColor& color);

    /// \brief Returns the hover pick distance in pixels.
    qreal hoverRadius() const;
    /// \brief Sets the hover pick distance to \a radius pixels. Negative values are clamped to 0.
    void setHoverRadius(qreal radius);

    /// \brief Returns the number of data points currently loaded.
    int count() const;
    /// \brief Returns the index of the hovered error bar, or -1 if none.
    int hoveredIndex() const;

    /// \brief Appends the data point (\a x, \a y, \a low, \a high). Triggers a redraw.
    Q_INVOKABLE void appendData(qreal x, qreal y, qreal low, qreal high);
    /// \brief Replaces the data points with \a xs, \a ys, \a lows, and \a highs. If the sizes differ, the shortest length is used.
    Q_INVOKABLE void setData(const QList<qreal>& xs, const QList<qreal>& ys, const QList<qreal>& lows, const QList<qreal>& highs);
    /// \brief Replaces the data points with \a xs, \a ys, \a lows, and \a highs. If the sizes differ, the shortest length is used.
    void setData(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& lows, const std::vector<double>& highs);
    // The PlotSeries overloads take interleaved (x, y, low, high) quadruples, as doubles or floats.
    using PlotSeries::setData;
    using PlotSeries::setDataF;

    /// \brief Appends a data point at (\a x, \a y) with \a error on both sides of the value.
    ///
    /// The value is \a y for vertical and \a x for horizontal error bars. The sign of \a error is ignored.
    Q_INVOKABLE void appendErrorData(qreal x, qreal y, qreal error);
    /// \brief Appends a data point at (\a x, \a y) with \a minus below and \a plus above the value. Signs are ignored.
    Q_INVOKABLE void appendErrorData(qreal x, qreal y, qreal minus, qreal plus);
    /// \brief Replaces the data points with \a xs and \a ys, each with an error from \a errors on both sides of the value.
    ///
    /// If the sizes differ, the shortest length is used. The signs of the errors are ignored.
    Q_INVOKABLE void setErrorData(const QList<qreal>& xs, const QList<qreal>& ys, const QList<qreal>& errors);
    /// \brief Replaces the data points with \a xs and \a ys, each with an error from \a minuses below and \a pluses above the value.
    ///
    /// If the sizes differ, the shortest length is used. The signs of the errors are ignored.
    Q_INVOKABLE void setErrorData(const QList<qreal>& xs, const QList<qreal>& ys, const QList<qreal>& minuses, const QList<qreal>& pluses);
    /// \brief Replaces the data points with \a xs and \a ys, each with an error from \a errors on both sides of the value.
    void setErrorData(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& errors);
    /// \brief Replaces the data points with \a xs and \a ys, each with an error from \a minuses below and \a pluses above the value.
    void setErrorData(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& minuses, const std::vector<double>& pluses);

    /// \brief Removes all data points.
    Q_INVOKABLE void clearData() override;

    /// \brief Returns data point \a index as an object with \c x, \c y, \c low, and \c high properties, or an empty object when out of range.
    Q_INVOKABLE QVariantMap errorBarAt(int index) const;
    /// \brief Returns the index of the error bar within \c hoverRadius of item position \a position, or -1.
    ///
    /// The nearest error bar wins; on a tie, the one with the highest index.
    Q_INVOKABLE int errorBarIndexAt(const QPointF& position) const;
    /// \brief Returns \c true when an error bar lies within \c hoverRadius of item position \a point.
    ///
    /// Hover delivery uses this test, so stacked series underneath still receive hover events away from
    /// the error bars.
    bool contains(const QPointF& point) const override;

signals:
    /// \brief Emitted when the orientation property changes.
    void orientationChanged();
    /// \brief Emitted when the color property changes.
    void colorChanged();
    /// \brief Emitted when the lineWidth property changes.
    void lineWidthChanged();
    /// \brief Emitted when the capLength property changes.
    void capLengthChanged();
    /// \brief Emitted when the offset property changes.
    void offsetChanged();
    /// \brief Emitted when the hoverColor property changes.
    void hoverColorChanged();
    /// \brief Emitted when the hoverRadius property changes.
    void hoverRadiusChanged();
    /// \brief Emitted when the data point count changes.
    void countChanged();
    /// \brief Emitted when the hovered error bar changes.
    void hoveredIndexChanged();

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
    /// \brief Refreshes the hover lookup and repaints after a bound axis changes between linear and log scale.
    void onAxisScaleChanged() override;
    /// \brief Refreshes the hover lookup and the count after the data points changed.
    void onDataPointsChanged(DataChange change, int previousCount) override;

private:
    // A drawn error bar in data coordinates: its position across the bounds, shifted by the offset,
    // and its ends along them. A NaN bound is replaced by the value and has no cap.
    struct DataBar {
        double position;
        double start;
        double end;
        bool startCapped;
        bool endCapped;
    };

    bool isHorizontal() const;
    double component(int index, int column) const;
    // Returns error bar \a index as drawn, or nullopt when it is not drawn.
    std::optional<DataBar> dataBar(int index) const;
    // Pixel distance from \a position to the line and caps of error bar \a index, or nullopt when it is not drawn.
    std::optional<qreal> pixelDistance(int index, const QPointF& position) const;
    void applyInterleavedData(std::vector<double>&& data);
    void setHoveredIndex(int index);
    void onGeometryChanged();
    void ensureSpatialGrid() const;
    void updateVertices(QSGGeometryNode& node, int drawnCount) const;
    void updateMaterial(ErrorBarMaterial& material, int drawnCount) const;

    Qt::Orientation orientation_{Qt::Vertical};
    QColor color_;
    qreal lineWidth_{1.5};
    qreal capLength_{8.0};
    qreal offset_{0.0};
    QColor hoverColor_;
    qreal hoverRadius_{6.0};
    int hoveredIndex_{-1};
    // Render revision of the data points last uploaded to the GPU.
    std::uint64_t uploadedRevision_{0};
    // Whether the last frame held more data points than the GPU buffers do; warns on each crossing.
    bool overCapacity_{false};
    // Error bars the GPU draws, as of the last frame; those beyond are not hovered either.
    int renderCapacity_{std::numeric_limits<int>::max()};
    // Data-space rectangles of the error bars that bound the hover candidates.
    mutable std::vector<double> hitRects_;
    mutable SpatialGrid spatialGrid_;
    mutable bool spatialGridValid_{false};
};

} // namespace QAccelPlot
