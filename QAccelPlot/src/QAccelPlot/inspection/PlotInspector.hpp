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

#include <optional>

namespace QAccelPlot {

/// \brief Inspects every visible XY series of a plot at a cursor and publishes one model row per series.
///
/// The cursor follows the plot's pointer by default; set \c followPointer to \c false and write
/// \c cursorX to drive it from code, for example to link the crosshairs of several plots.
/// Results are refreshed at most once per event-loop pass.
///
/// \sa SeriesInspection, InspectionRowModel
class PlotInspector : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(PlotInspector)
    /// \brief Plot whose series are inspected.
    Q_PROPERTY(::QAccelPlot::QAccelPlot* plot READ plot WRITE setPlot NOTIFY plotChanged)
    /// \brief Enables queries. A disabled inspector is inactive and does no work. Default: true.
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    /// \brief \c NearestX reports each series at the cursor's X; \c NearestXY picks by on-screen distance. Default: NearestX.
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    /// \brief Pick distance in logical pixels. In \c NearestX mode it only limits matches beyond a series' first or last sample. Default: 16.
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged)
    /// \brief Moves the crosshair to the closest matching sample. Default: false.
    Q_PROPERTY(bool snapToSample READ snapToSample WRITE setSnapToSample NOTIFY snapToSampleChanged)
    /// \brief In \c NearestX mode, reports the point on the line between two consecutive samples instead of the nearer one. Default: false.
    Q_PROPERTY(bool interpolate READ interpolate WRITE setInterpolate NOTIFY interpolateChanged)
    /// \brief Adds Y statistics of the samples around the cursor to every row. Default: false.
    Q_PROPERTY(bool summaries READ summaries WRITE setSummaries NOTIFY summariesChanged)
    /// \brief Half-width of the summary neighborhood in logical pixels. Default: 0.5.
    Q_PROPERTY(qreal summaryRadius READ summaryRadius WRITE setSummaryRadius NOTIFY summaryRadiusChanged)
    /// \brief Series to inspect; an empty list inspects every series of the plot.
    Q_PROPERTY(QList<PlotSeries*> includedSeries READ includedSeries WRITE setIncludedSeries NOTIFY includedSeriesChanged)
    /// \brief Series never inspected, even when listed in \c includedSeries.
    Q_PROPERTY(QList<PlotSeries*> excludedSeries READ excludedSeries WRITE setExcludedSeries NOTIFY excludedSeriesChanged)
    /// \brief Whether the cursor follows the plot's pointer. When false it stays at \c cursorX and \c cursorY. Default: true.
    Q_PROPERTY(bool followPointer READ followPointer WRITE setFollowPointer NOTIFY followPointerChanged)
    /// \brief Cursor X on the plot's primary X axis: the pointer's while \c followPointer is true, NaN when it is outside.
    ///
    /// A written value is used while \c followPointer is false.
    Q_PROPERTY(qreal cursorX READ cursorX WRITE setCursorX NOTIFY cursorChanged)
    /// \brief Cursor Y on the plot's primary Y axis; see \c cursorX. NaN leaves the cursor without a Y.
    Q_PROPERTY(qreal cursorY READ cursorY WRITE setCursorY NOTIFY cursorChanged)
    /// \brief Crosshair X formatted by the plot's primary X axis.
    Q_PROPERTY(QString cursorXText READ cursorXText NOTIFY cursorChanged)
    /// \brief Crosshair Y formatted by the plot's primary Y axis; empty without a cursor Y.
    Q_PROPERTY(QString cursorYText READ cursorYText NOTIFY cursorChanged)
    /// \brief True while enabled and the cursor is inside the plot area.
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    /// \brief Crosshair position in plot-local logical pixels; Y is NaN without a cursor Y.
    Q_PROPERTY(QPointF position READ position NOTIFY positionChanged)
    /// \brief Read-only constant: one row per inspected series.
    Q_PROPERTY(::QAccelPlot::InspectionRowModel* model READ model CONSTANT)
    /// \brief Number of rows with a matching sample.
    Q_PROPERTY(int validCount READ validCount NOTIFY validCountChanged)

public:
    /// \brief How each series is matched to the cursor.
    enum Mode {
        NearestX, ///< \brief The sample at the cursor's X, for comparing series at one position.
        NearestXY ///< \brief The sample closest to the cursor on screen.
    };
    Q_ENUM(Mode)

    /// \brief Constructs an inspector without a plot.
    explicit PlotInspector(QObject* parent = nullptr);

    /// \brief Returns the inspected plot.
    ::QAccelPlot::QAccelPlot* plot() const;
    /// \brief Sets the inspected plot.
    void setPlot(::QAccelPlot::QAccelPlot* plot);
    /// \brief Returns whether queries are enabled.
    bool enabled() const;
    /// \brief Enables or disables queries.
    void setEnabled(bool enabled);
    /// \brief Returns the matching mode.
    Mode mode() const;
    /// \brief Sets the matching mode.
    void setMode(Mode mode);
    /// \brief Returns the pick distance in logical pixels.
    qreal radius() const;
    /// \brief Sets the pick distance; negative and NaN values are ignored.
    void setRadius(qreal radius);
    /// \brief Returns whether the crosshair snaps to the closest matching sample.
    bool snapToSample() const;
    /// \brief Enables or disables crosshair snapping.
    void setSnapToSample(bool enabled);
    /// \brief Returns whether \c NearestX rows are interpolated between samples.
    bool interpolate() const;
    /// \brief Enables or disables interpolation between consecutive samples.
    void setInterpolate(bool enabled);
    /// \brief Returns whether rows include neighborhood statistics.
    bool summaries() const;
    /// \brief Enables or disables neighborhood statistics.
    void setSummaries(bool enabled);
    /// \brief Returns the summary half-width in logical pixels.
    qreal summaryRadius() const;
    /// \brief Sets the summary half-width; negative and nonfinite values are ignored.
    void setSummaryRadius(qreal radius);
    /// \brief Returns the series to inspect, without destroyed entries.
    QList<PlotSeries*> includedSeries() const;
    /// \brief Sets the series to inspect.
    void setIncludedSeries(const QList<PlotSeries*>& series);
    /// \brief Returns the series never inspected, without destroyed entries.
    QList<PlotSeries*> excludedSeries() const;
    /// \brief Sets the series never inspected.
    void setExcludedSeries(const QList<PlotSeries*>& series);
    /// \brief Returns whether the cursor follows the plot's pointer.
    bool followPointer() const;
    /// \brief Switches between following the pointer and a cursor set from code.
    void setFollowPointer(bool follow);
    /// \brief Returns the cursor X on the plot's primary X axis.
    qreal cursorX() const;
    /// \brief Sets the cursor X used while \c followPointer is false.
    void setCursorX(qreal x);
    /// \brief Returns the cursor Y on the plot's primary Y axis.
    qreal cursorY() const;
    /// \brief Sets the cursor Y used while \c followPointer is false.
    void setCursorY(qreal y);
    /// \brief Returns the formatted crosshair X.
    QString cursorXText() const;
    /// \brief Returns the formatted crosshair Y.
    QString cursorYText() const;
    /// \brief Returns whether the cursor is inside the plot area.
    bool active() const;
    /// \brief Returns the crosshair position in plot-local logical pixels.
    QPointF position() const;
    /// \brief Returns the row model.
    InspectionRowModel* model() const;
    /// \brief Returns the number of rows with a matching sample.
    int validCount() const;

    /// \brief Runs the queries now instead of on the next event-loop pass.
    Q_INVOKABLE void refresh();
    /// \brief Moves the cursor \a steps source records along the first matching series and stops following the pointer.
    Q_INVOKABLE void stepCursor(int steps);

signals:
    /// \brief Emitted when the plot property changes.
    void plotChanged();
    /// \brief Emitted when the enabled property changes.
    void enabledChanged();
    /// \brief Emitted when the mode property changes.
    void modeChanged();
    /// \brief Emitted when the radius property changes.
    void radiusChanged();
    /// \brief Emitted when the snapToSample property changes.
    void snapToSampleChanged();
    /// \brief Emitted when the interpolate property changes.
    void interpolateChanged();
    /// \brief Emitted when the summaries property changes.
    void summariesChanged();
    /// \brief Emitted when the summaryRadius property changes.
    void summaryRadiusChanged();
    /// \brief Emitted when the includedSeries property changes.
    void includedSeriesChanged();
    /// \brief Emitted when the excludedSeries property changes.
    void excludedSeriesChanged();
    /// \brief Emitted when the followPointer property changes.
    void followPointerChanged();
    /// \brief Emitted when the cursor coordinates or their texts change.
    void cursorChanged();
    /// \brief Emitted when the active property changes.
    void activeChanged();
    /// \brief Emitted when the position property changes.
    void positionChanged();
    /// \brief Emitted when the validCount property changes.
    void validCountChanged();
    /// \brief Emitted after every refresh that published results.
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
    // Returns the cursor in plot-local pixels, or nothing while the inspector is inactive. Y is NaN for a cursor set from code without a Y.
    std::optional<QPointF> cursorPosition() const;
    // Fills state with the results at cursor. Returns false when user code run by a label formatter invalidated them.
    bool inspect(const QPointF& cursor, State& state) const;
    QList<Query> queryRows(const QPointF& cursor) const;
    // Between two samples a series always matches; the radius only limits how far past its ends it still does.
    InspectionSample sampleByX(SeriesInspection& inspection, qreal pixelX) const;
    InspectionSummary summarize(PlotSeries& series, const QPointF& local, bool byX) const;
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
};

} // namespace QAccelPlot
