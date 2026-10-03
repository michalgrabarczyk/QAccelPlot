

# File SeriesInspection.hpp

[**File List**](files.md) **>** [**inspection**](dir_7c3af00b227ed418fdf47d7cd69e8a77.md) **>** [**SeriesInspection.hpp**](SeriesInspection_8hpp.md)

[Go to the documentation of this file](SeriesInspection_8hpp.md)


```C++
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
enum class InspectionAxis;
struct InspectionMetric;
struct InspectionSource;

class SeriesInspection : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(::QAccelPlot::InspectionNS::Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool supported READ supported NOTIFY statusChanged)
    Q_PROPERTY(int maximumPageSize READ maximumPageSize CONSTANT)

public:
    ~SeriesInspection() override;

    InspectionStatus status() const;
    bool supported() const;
    int maximumPageSize() const;

    Q_INVOKABLE quint64 indexBytes() const;
    Q_INVOKABLE void prepare();

    Q_INVOKABLE ::QAccelPlot::InspectionSample sampleAt(int index) const;
    Q_INVOKABLE ::QAccelPlot::InspectionSample nearest(const QPointF& position, qreal radius = 10);
    Q_INVOKABLE ::QAccelPlot::InspectionSample nearestByX(qreal pixelX, qreal radius = std::numeric_limits<qreal>::infinity());
    Q_INVOKABLE ::QAccelPlot::InspectionSample nearestByY(qreal pixelY, qreal radius = std::numeric_limits<qreal>::infinity());
    Q_INVOKABLE ::QAccelPlot::InspectionBracket bracketByX(qreal pixelX);
    Q_INVOKABLE ::QAccelPlot::InspectionBracket bracketByY(qreal pixelY);
    Q_INVOKABLE ::QAccelPlot::InspectionSummary summarize(qreal xMin, qreal xMax, qreal yMin, qreal yMax);
    Q_INVOKABLE ::QAccelPlot::InspectionSummary summarizeRange(qreal xMin, qreal xMax);
    Q_INVOKABLE ::QAccelPlot::InspectionPage indices(
        qreal xMin, qreal xMax, qreal yMin, qreal yMax, int offset = 0, int limit = 4096, quint64 expectedRevision = 0);

    Q_INVOKABLE ::QAccelPlot::InspectionRecord recordAt(int index) const;
    Q_INVOKABLE ::QAccelPlot::InspectionRecord recordAtPosition(const QPointF& position) const;

signals:
    void statusChanged();

private:
    friend class PlotSeries;
    struct Private;

    explicit SeriesInspection(PlotSeries& series);
    void sourceChanged(bool appended);
    void sourceInvalidated();
    // Returns Ready when a query can run now: one along the given axis, or an on-screen or region query otherwise.
    // A series that is not ordered as the query needs requests its index and reports Preparing until it is built.
    InspectionStatus acquire(const InspectionSource& source, std::optional<InspectionAxis> along);
    std::optional<InspectionMetric> metric() const;
    InspectionSample nearestAlong(InspectionAxis axis, qreal pixel, qreal radius);
    InspectionBracket bracketAlong(InspectionAxis axis, qreal pixel);
    InspectionSample makeSample(const InspectionSource& source, int index, qreal distance) const;
    // Interpolates along the straight on-screen segment, which also follows logarithmic axes.
    InspectionSample interpolate(
        const InspectionMetric& metric, InspectionAxis axis, const InspectionSample& left, const InspectionSample& right, qreal pixel) const;

    PlotSeries& series_;
    std::unique_ptr<Private> d_;
};

} // namespace QAccelPlot
```


