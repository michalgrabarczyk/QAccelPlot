

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

class SeriesInspection : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(InspectionNS::Status status READ status NOTIFY statusChanged)

public:
    ~SeriesInspection() override;

    InspectionStatus status() const;

    Q_INVOKABLE quint64 indexBytes() const;
    Q_INVOKABLE void prepare();

    Q_INVOKABLE InspectionDataPoint dataPointAt(int index) const;
    Q_INVOKABLE InspectionDataPoint nearest(const QPointF& position, qreal radius = 10);
    Q_INVOKABLE InspectionDataPoint nearestByX(qreal pixelX, qreal radius = std::numeric_limits<qreal>::infinity());
    Q_INVOKABLE InspectionDataPoint nearestByY(qreal pixelY, qreal radius = std::numeric_limits<qreal>::infinity());
    Q_INVOKABLE InspectionBracket bracketByX(qreal pixelX);
    Q_INVOKABLE InspectionBracket bracketByY(qreal pixelY);
    Q_INVOKABLE InspectionSummary summarize(qreal xMin, qreal xMax, qreal yMin, qreal yMax);
    Q_INVOKABLE InspectionSummary summarizeRange(qreal xMin, qreal xMax);
    Q_INVOKABLE InspectionPage indices(qreal xMin, qreal xMax, qreal yMin, qreal yMax, int offset = 0, int limit = 4096, quint64 expectedRevision = 0);

    Q_INVOKABLE InspectionRecord recordAt(int index) const;
    Q_INVOKABLE InspectionRecord recordAtPosition(const QPointF& position) const;
    Q_INVOKABLE InspectionDataPoint dataPointAtPosition(const QPointF& position) const;

signals:
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
```


