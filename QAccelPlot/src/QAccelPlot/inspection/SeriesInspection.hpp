//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/inspection/InspectionResult.hpp"
#include "QAccelPlot/series/StepMode.hpp"

#include <QObject>
#include <QPointF>

#include <limits>
#include <memory>
#include <optional>

namespace QAccelPlot {

class PlotSeries;

namespace Internal {
enum class InspectionAxis;
enum class InspectionMatching;
struct InspectionMetric;
struct InspectionSource;
} // namespace Internal

/// \brief Data queries for one series: nearest data points, brackets, region statistics, and index pages.
///
/// Available as \c PlotSeries::inspection. Call it on the series' thread. Pixel arguments are
/// series-local logical pixels; regions are inclusive data-space limits, where an infinite limit
/// leaves that side unbounded and reversed limits are swapped.
///
/// Queries cover every stored data point, also outside the viewport, and match data points rather than
/// the line segments between them. Invalid data points never match, and equal distances resolve to
/// the highest index. Statistics are data-point-weighted and use the original Y values.
///
/// \c LineCurve and \c PointCloud answer every query. Series whose X or Y values are finite and
/// non-decreasing are searched in place, need no preparation, and stay queryable while records
/// are appended; queries along one axis, such as \c nearestByX(), need that order along their own
/// axis. Other series of up to 20,000 records are scanned per query. Larger ones are indexed on a
/// worker thread first; until then queries return \c Inspection.Preparing. Requesting an index
/// copies the series' buffer once on the calling thread, the index takes about 40 MB per million
/// valid data points, and any data change discards it. Other series types expose native records
/// through \c recordAt() and \c recordAtPosition() only; \c HeatmapSeries also answers
/// \c dataPointAtPosition().
///
/// \sa PlotInspector, SelectionTool
class SeriesInspection : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    /// \brief Readiness for data point queries: \c Ready, \c Idle, \c Preparing, \c Unsupported, or \c Unavailable.
    ///
    /// Bar, rectangle, band, and heatmap series are \c Ready while available; the queries that do not
    /// apply to them return \c Unsupported. \c Unsupported only for a series that answers no query.
    Q_PROPERTY(InspectionNS::Status status READ status NOTIFY statusChanged)

public:
    ~SeriesInspection() override;

    /// \brief Returns the readiness for data point queries.
    InspectionStatus status() const;

    /// \brief Returns the bytes held by query caches and indexes, excluding the series' own data.
    Q_INVOKABLE quint64 indexBytes() const;
    /// \brief Starts building a query index when the series needs one; otherwise does nothing.
    Q_INVOKABLE void prepare();

    /// \brief Returns the record at \a index; \c NoMatch with its raw coordinates when the record is invalid.
    Q_INVOKABLE InspectionDataPoint dataPointAt(int index) const;
    /// \brief Returns the valid data point nearest to \a position within \a radius pixels on screen.
    Q_INVOKABLE InspectionDataPoint nearest(const QPointF& position, qreal radius = 10);
    /// \brief Returns the valid data point nearest to \a pixelX horizontally, within \a radius pixels.
    Q_INVOKABLE InspectionDataPoint nearestByX(qreal pixelX, qreal radius = std::numeric_limits<qreal>::infinity());
    /// \brief Returns the valid data point nearest to \a pixelY vertically, within \a radius pixels.
    Q_INVOKABLE InspectionDataPoint nearestByY(qreal pixelY, qreal radius = std::numeric_limits<qreal>::infinity());
    /// \brief Returns the valid data points on either side of \a pixelX and the point between them.
    Q_INVOKABLE InspectionBracket bracketByX(qreal pixelX);
    /// \brief Returns the valid data points whose Y is on either side of \a pixelY and the point between them.
    Q_INVOKABLE InspectionBracket bracketByY(qreal pixelY);
    /// \brief Returns Y statistics of the valid data points inside the region.
    Q_INVOKABLE InspectionSummary summarize(qreal xMin, qreal xMax, qreal yMin, qreal yMax);
    /// \brief Returns Y statistics of the valid data points whose X lies in the interval.
    Q_INVOKABLE InspectionSummary summarizeRange(qreal xMin, qreal xMax);
    /// \brief Returns up to \a limit source indices inside the region, skipping the first \a offset matches.
    ///
    /// \a limit must be positive and is capped at 4096, the default; the page's \c limit reports the
    /// size used. Request the next page with \a offset advanced by the length of \c indices while
    /// \c hasMore is true.
    ///
    /// A nonzero \a expectedRevision that differs from the series' \c dataRevision returns
    /// \c Stale, so pages of one traversal never mix indices of different data.
    Q_INVOKABLE InspectionPage indices(qreal xMin, qreal xMax, qreal yMin, qreal yMax, int offset = 0, int limit = 4096, quint64 expectedRevision = 0);

    /// \brief Returns the native record at \a index of a bar, rectangle, or band series.
    Q_INVOKABLE InspectionRecord recordAt(int index) const;
    /// \brief Returns the native record drawn at the series-local \a position.
    Q_INVOKABLE InspectionRecord recordAtPosition(const QPointF& position) const;
    /// \brief Returns the record drawn at the series-local \a position as a data point, such as the heatmap cell under it.
    ///
    /// \c Unsupported for series that do not draw data points as areas. The data and pixel coordinates
    /// of a match are the record's center, and \c distance is measured from \a position to it.
    Q_INVOKABLE InspectionDataPoint dataPointAtPosition(const QPointF& position) const;

signals:
    /// \brief Emitted when the readiness may have changed: after a data change, and when an index is ready.
    void statusChanged();

private:
    friend class PlotSeries;
    friend class PlotInspector;
    friend class SelectionTool;
    struct Private;

    explicit SeriesInspection(PlotSeries& series);
    Internal::InspectionMatching matching() const;
    void sourceChanged(bool appended);
    void sourceInvalidated();
    // Returns Ready when a query can run now: one along the given axis, or an on-screen or region query otherwise.
    // A series that is not ordered as the query needs requests its index and reports Preparing until it is built.
    InspectionStatus acquire(const Internal::InspectionSource& source, std::optional<Internal::InspectionAxis> along);
    std::optional<Internal::InspectionMetric> metric() const;
    InspectionDataPoint nearestAlong(Internal::InspectionAxis axis, qreal pixel, qreal radius);
    InspectionBracket bracketAlong(Internal::InspectionAxis axis, qreal pixel);
    InspectionDataPoint makeDataPoint(const Internal::InspectionSource& source, int index, qreal distance) const;
    // Interpolates along the line drawn with stepMode between left and right: the straight on-screen segment, which
    // also follows logarithmic axes, or the steps. A step point belongs to the data point whose step or riser it is on.
    InspectionDataPoint interpolate(const Internal::InspectionMetric& metric, Internal::InspectionAxis axis, const InspectionDataPoint& left,
        const InspectionDataPoint& right, qreal pixel, StepMode stepMode) const;

    PlotSeries& series_;
    std::unique_ptr<Private> d_;
};

} // namespace QAccelPlot
