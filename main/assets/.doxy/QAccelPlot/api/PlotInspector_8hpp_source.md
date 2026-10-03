

# File PlotInspector.hpp

[**File List**](files.md) **>** [**inspection**](dir_7c3af00b227ed418fdf47d7cd69e8a77.md) **>** [**PlotInspector.hpp**](PlotInspector_8hpp.md)

[Go to the documentation of this file](PlotInspector_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/inspection/InspectionRowModel.hpp"

#include <QQmlListProperty>

#include <memory>
#include <optional>

namespace QAccelPlot {

class OverlayChildren;

class PlotInspector : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(PlotInspector)
    Q_CLASSINFO("DefaultProperty", "data")
    Q_PROPERTY(QQmlListProperty<QObject> data READ data)
    Q_PROPERTY(::QAccelPlot::QAccelPlot* plot READ plot WRITE setPlot NOTIFY plotChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(bool snapToSample READ snapToSample WRITE setSnapToSample NOTIFY snapToSampleChanged)
    Q_PROPERTY(bool interpolate READ interpolate WRITE setInterpolate NOTIFY interpolateChanged)
    Q_PROPERTY(bool summaries READ summaries WRITE setSummaries NOTIFY summariesChanged)
    Q_PROPERTY(qreal summaryRadius READ summaryRadius WRITE setSummaryRadius NOTIFY summaryRadiusChanged)
    Q_PROPERTY(QList<PlotSeries*> includedSeries READ includedSeries WRITE setIncludedSeries NOTIFY includedSeriesChanged)
    Q_PROPERTY(QList<PlotSeries*> excludedSeries READ excludedSeries WRITE setExcludedSeries NOTIFY excludedSeriesChanged)
    Q_PROPERTY(bool followPointer READ followPointer WRITE setFollowPointer NOTIFY followPointerChanged)
    Q_PROPERTY(qreal cursorX READ cursorX WRITE setCursorX NOTIFY cursorChanged)
    Q_PROPERTY(qreal cursorY READ cursorY WRITE setCursorY NOTIFY cursorChanged)
    Q_PROPERTY(QString cursorXText READ cursorXText NOTIFY cursorChanged)
    Q_PROPERTY(QString cursorYText READ cursorYText NOTIFY cursorChanged)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(QPointF position READ position NOTIFY positionChanged)
    Q_PROPERTY(::QAccelPlot::InspectionRowModel* model READ model CONSTANT)
    Q_PROPERTY(int validCount READ validCount NOTIFY validCountChanged)

public:
    enum Mode {
        NearestX, 
        NearestY, 
        NearestXY 
    };
    Q_ENUM(Mode)

    
    explicit PlotInspector(QObject* parent = nullptr);
    ~PlotInspector() override;

    QQmlListProperty<QObject> data();

    ::QAccelPlot::QAccelPlot* plot() const;
    void setPlot(::QAccelPlot::QAccelPlot* plot);
    bool enabled() const;
    void setEnabled(bool enabled);
    Mode mode() const;
    void setMode(Mode mode);
    qreal radius() const;
    void setRadius(qreal radius);
    bool snapToSample() const;
    void setSnapToSample(bool enabled);
    bool interpolate() const;
    void setInterpolate(bool enabled);
    bool summaries() const;
    void setSummaries(bool enabled);
    qreal summaryRadius() const;
    void setSummaryRadius(qreal radius);
    QList<PlotSeries*> includedSeries() const;
    void setIncludedSeries(const QList<PlotSeries*>& series);
    QList<PlotSeries*> excludedSeries() const;
    void setExcludedSeries(const QList<PlotSeries*>& series);
    bool followPointer() const;
    void setFollowPointer(bool follow);
    qreal cursorX() const;
    void setCursorX(qreal x);
    qreal cursorY() const;
    void setCursorY(qreal y);
    QString cursorXText() const;
    QString cursorYText() const;
    bool active() const;
    QPointF position() const;
    InspectionRowModel* model() const;
    int validCount() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void stepCursor(int steps);

signals:
    void plotChanged();
    void enabledChanged();
    void modeChanged();
    void radiusChanged();
    void snapToSampleChanged();
    void interpolateChanged();
    void summariesChanged();
    void summaryRadiusChanged();
    void includedSeriesChanged();
    void excludedSeriesChanged();
    void followPointerChanged();
    void cursorChanged();
    void activeChanged();
    void positionChanged();
    void validCountChanged();
    void refreshed();

private:
    // One series' query result together with what is needed to format it and to detect that it went stale.
    struct Query;
    struct State;

    void schedule();
    void flush();
    void reconnect();
    void connectSeries(PlotSeries* series);
    bool inspects(PlotSeries* series) const;
    QList<PlotSeries*> inspectedSeries() const;
    // Returns the cursor in plot-local pixels, or nothing while the inspector is inactive. A coordinate that a cursor set from code lacks is NaN.
    std::optional<QPointF> cursorPosition() const;
    // Returns the mode the cursor allows: without one of its coordinates, series are matched along the other axis.
    Mode effectiveMode(const QPointF& cursor) const;
    // Fills state with the results at cursor. Returns false when user code run by a label formatter invalidated them.
    bool inspect(const QPointF& cursor, State& state) const;
    QList<Query> queryRows(const QPointF& cursor) const;
    // Between two samples a series always matches; the radius only limits how far past its ends it still does.
    InspectionSample sampleAlong(SeriesInspection& inspection, Mode mode, const QPointF& local) const;
    InspectionSummary summarize(PlotSeries& series, const QPointF& local, Mode mode) const;
    void formatRows(QList<Query>& queries) const;
    bool current(const QList<Query>& queries) const;
    QPointF snappedPosition(const QList<Query>& queries, const QPointF& cursor) const;
    void publish(const State& state);

    QPointer<::QAccelPlot::QAccelPlot> plot_;
    bool enabled_{true};
    Mode mode_{NearestX};
    qreal radius_{16};
    bool snapToSample_{false};
    bool interpolate_{false};
    bool summaries_{false};
    qreal summaryRadius_{0.5};
    QList<QPointer<PlotSeries>> included_;
    QList<QPointer<PlotSeries>> excluded_;
    bool followPointer_{true};
    QList<QMetaObject::Connection> connections_;
    bool pending_{false};
    bool refreshing_{false};
    bool active_{false};
    QPointF position_;
    qreal cursorX_;
    qreal cursorY_;
    // Cursor set from code; used while followPointer_ is false.
    qreal pinnedX_;
    qreal pinnedY_;
    QString cursorXText_;
    QString cursorYText_;
    int validCount_{0};
    InspectionRowModel* model_;
    std::unique_ptr<OverlayChildren> children_;
};

} // namespace QAccelPlot
```


