//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/inspection/InspectionResult.hpp"

#include <QObject>
#include <QPointF>

#include <limits>
#include <memory>
#include <optional>

namespace QAccelPlot {

class PlotSeries;
struct InspectionMetric;
struct InspectionSource;

/// \brief Data queries for one series: nearest samples, brackets, region statistics, and index pages.
///
/// Available as \c PlotSeries::inspection. Call it on the series' thread. Pixel arguments are
/// series-local logical pixels; regions are inclusive data-space limits, where an infinite limit
/// leaves that side unbounded and reversed limits are swapped.
///
/// \c LineCurve and \c PointCloud answer every query. Series whose X values are finite and
/// non-decreasing are searched in place, need no preparation, and stay queryable while records
/// are appended. Unordered series above a small size are indexed on a worker thread first; until
/// then queries return \c Inspection.Preparing. Other series types expose native records through
/// \c recordAt() and \c recordAtPosition() only.
///
/// \sa PlotInspector, SelectionTool
class SeriesInspection : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    /// \brief Readiness for sample queries: \c Ready, \c Idle, \c Preparing, \c Unsupported, or \c Unavailable.
    Q_PROPERTY(::QAccelPlot::InspectionNS::Status status READ status NOTIFY statusChanged)
    /// \brief Whether the series has XY records that sample queries can search.
    Q_PROPERTY(bool supported READ supported NOTIFY statusChanged)
    /// \brief Largest page returned by \c indices().
    Q_PROPERTY(int maximumPageSize READ maximumPageSize CONSTANT)

public:
    ~SeriesInspection() override;

    /// \brief Returns the readiness for sample queries.
    InspectionStatus status() const;
    /// \brief Returns whether the series has XY records that sample queries can search.
    bool supported() const;
    /// \brief Returns the largest page returned by \c indices().
    int maximumPageSize() const;

    /// \brief Returns the bytes held by query caches and indexes, excluding the series' own data.
    Q_INVOKABLE quint64 indexBytes() const;
    /// \brief Starts building a query index when the series needs one; otherwise does nothing.
    Q_INVOKABLE void prepare();

    /// \brief Returns the record at \a index; \c NoMatch with its raw coordinates when the record is invalid.
    Q_INVOKABLE ::QAccelPlot::InspectionSample sampleAt(int index) const;
    /// \brief Returns the valid sample nearest to \a position within \a radius pixels on screen.
    Q_INVOKABLE ::QAccelPlot::InspectionSample nearest(const QPointF& position, qreal radius = 10);
    /// \brief Returns the valid sample nearest to \a pixelX horizontally, within \a radius pixels.
    Q_INVOKABLE ::QAccelPlot::InspectionSample nearestByX(qreal pixelX, qreal radius = std::numeric_limits<qreal>::infinity());
    /// \brief Returns the valid samples on either side of \a pixelX and the point between them.
    Q_INVOKABLE ::QAccelPlot::InspectionBracket bracketByX(qreal pixelX);
    /// \brief Returns Y statistics of the valid samples inside the region.
    Q_INVOKABLE ::QAccelPlot::InspectionSummary summarize(qreal xMin, qreal xMax, qreal yMin, qreal yMax);
    /// \brief Returns Y statistics of the valid samples whose X lies in the interval.
    Q_INVOKABLE ::QAccelPlot::InspectionSummary summarizeRange(qreal xMin, qreal xMax);
    /// \brief Returns up to \a limit source indices inside the region, skipping the first \a offset matches.
    ///
    /// A nonzero \a expectedRevision that differs from the series' \c dataRevision returns
    /// \c Stale, so pages of one traversal never mix indices of different data.
    Q_INVOKABLE ::QAccelPlot::InspectionPage indices(
        qreal xMin, qreal xMax, qreal yMin, qreal yMax, int offset = 0, int limit = 4096, quint64 expectedRevision = 0);

    /// \brief Returns the native record at \a index of a bar, rectangle, or band series.
    Q_INVOKABLE ::QAccelPlot::InspectionRecord recordAt(int index) const;
    /// \brief Returns the native record drawn at the series-local \a position.
    Q_INVOKABLE ::QAccelPlot::InspectionRecord recordAtPosition(const QPointF& position) const;

signals:
    /// \brief Emitted when the readiness may have changed: after a data change, and when an index is ready.
    void statusChanged();

private:
    friend class PlotSeries;
    struct Private;

    explicit SeriesInspection(PlotSeries& series);
    void sourceChanged(bool appended);
    void sourceInvalidated();
    // Returns Ready when queries can run now. An unordered series requests its index and reports Preparing until it is built.
    InspectionStatus acquire(const InspectionSource& source);
    std::optional<InspectionMetric> metric() const;
    InspectionSample makeSample(const InspectionSource& source, int index, qreal distance) const;
    // Interpolates along the straight on-screen segment, which also follows logarithmic axes.
    InspectionSample interpolate(const InspectionMetric& metric, const InspectionSample& left, const InspectionSample& right, qreal pixelX) const;

    PlotSeries& series_;
    std::unique_ptr<Private> d_;
};

} // namespace QAccelPlot
