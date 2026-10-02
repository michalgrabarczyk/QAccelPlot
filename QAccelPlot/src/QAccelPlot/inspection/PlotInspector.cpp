//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/inspection/PlotInspector.hpp"

#include "QAccelPlot/axis/AxisTicker.hpp"

#include <QScopedValueRollback>

#include <algorithm>
#include <cmath>
#include <limits>

namespace QAccelPlot {
namespace {

constexpr auto kNaN = std::numeric_limits<qreal>::quiet_NaN();

QList<PlotSeries*> liveSeries(const QList<QPointer<PlotSeries>>& pointers)
{
    auto result = QList<PlotSeries*>{};
    for (const auto& pointer : pointers) {
        if (pointer) {
            result.push_back(pointer);
        }
    }
    return result;
}

QList<QPointer<PlotSeries>> guardedSeries(const QList<PlotSeries*>& pointers)
{
    auto result = QList<QPointer<PlotSeries>>{};
    for (auto* pointer : pointers) {
        if (pointer) {
            result.push_back(pointer);
        }
    }
    return result;
}

// Equality that treats two NaN values as equal, so an unset coordinate does not count as a change.
bool sameValue(const qreal a, const qreal b)
{
    return a == b || (std::isnan(a) && std::isnan(b));
}

bool samePoint(const QPointF& a, const QPointF& b)
{
    return sameValue(a.x(), b.x()) && sameValue(a.y(), b.y());
}

QString format(const QPointer<Axis>& axis, const qreal value, const qreal length)
{
    return axis ? axis->formatValue(value, length) : QString{};
}

} // namespace

struct PlotInspector::Query {
    InspectionRow row;
    QPointer<Axis> xAxis;
    QPointer<Axis> yAxis;
    const Axis* boundXAxis{nullptr};
    const Axis* boundYAxis{nullptr};
    QSizeF size;
};

struct PlotInspector::State {
    bool active{false};
    QPointF position{kNaN, kNaN};
    qreal cursorX{kNaN};
    qreal cursorY{kNaN};
    QString cursorXText;
    QString cursorYText;
    QList<InspectionRow> rows;
};

PlotInspector::PlotInspector(QObject* parent)
    : QObject(parent)
    , position_(kNaN, kNaN)
    , cursorX_(kNaN)
    , cursorY_(kNaN)
    , model_(new InspectionRowModel(this))
{
}

::QAccelPlot::QAccelPlot* PlotInspector::plot() const
{
    return plot_;
}

void PlotInspector::setPlot(::QAccelPlot::QAccelPlot* plot)
{
    if (plot_ == plot) {
        return;
    }
    plot_ = plot;
    reconnect();
    emit plotChanged();
}

bool PlotInspector::enabled() const
{
    return enabled_;
}

void PlotInspector::setEnabled(const bool enabled)
{
    if (enabled_ == enabled) {
        return;
    }
    enabled_ = enabled;
    reconnect();
    emit enabledChanged();
}

PlotInspector::Mode PlotInspector::mode() const
{
    return mode_;
}

void PlotInspector::setMode(const Mode mode)
{
    if (mode_ == mode) {
        return;
    }
    mode_ = mode;
    schedule();
    emit modeChanged();
}

qreal PlotInspector::radius() const
{
    return radius_;
}

void PlotInspector::setRadius(const qreal radius)
{
    if (!(radius >= 0) || radius_ == radius) {
        return;
    }
    radius_ = radius;
    schedule();
    emit radiusChanged();
}

bool PlotInspector::snapToSample() const
{
    return snapToSample_;
}

void PlotInspector::setSnapToSample(const bool enabled)
{
    if (snapToSample_ == enabled) {
        return;
    }
    snapToSample_ = enabled;
    schedule();
    emit snapToSampleChanged();
}

bool PlotInspector::interpolate() const
{
    return interpolate_;
}

void PlotInspector::setInterpolate(const bool enabled)
{
    if (interpolate_ == enabled) {
        return;
    }
    interpolate_ = enabled;
    schedule();
    emit interpolateChanged();
}

bool PlotInspector::summaries() const
{
    return summaries_;
}

void PlotInspector::setSummaries(const bool enabled)
{
    if (summaries_ == enabled) {
        return;
    }
    summaries_ = enabled;
    schedule();
    emit summariesChanged();
}

qreal PlotInspector::summaryRadius() const
{
    return summaryRadius_;
}

void PlotInspector::setSummaryRadius(const qreal radius)
{
    if (!std::isfinite(radius) || radius < 0 || summaryRadius_ == radius) {
        return;
    }
    summaryRadius_ = radius;
    schedule();
    emit summaryRadiusChanged();
}

QList<PlotSeries*> PlotInspector::includedSeries() const
{
    return liveSeries(included_);
}

void PlotInspector::setIncludedSeries(const QList<PlotSeries*>& series)
{
    auto guarded = guardedSeries(series);
    if (included_ == guarded) {
        return;
    }
    included_ = std::move(guarded);
    schedule();
    emit includedSeriesChanged();
}

QList<PlotSeries*> PlotInspector::excludedSeries() const
{
    return liveSeries(excluded_);
}

void PlotInspector::setExcludedSeries(const QList<PlotSeries*>& series)
{
    auto guarded = guardedSeries(series);
    if (excluded_ == guarded) {
        return;
    }
    excluded_ = std::move(guarded);
    schedule();
    emit excludedSeriesChanged();
}

qreal PlotInspector::cursorX() const
{
    return cursorX_;
}

qreal PlotInspector::cursorY() const
{
    return cursorY_;
}

QString PlotInspector::cursorXText() const
{
    return cursorXText_;
}

QString PlotInspector::cursorYText() const
{
    return cursorYText_;
}

bool PlotInspector::active() const
{
    return active_;
}

QPointF PlotInspector::position() const
{
    return position_;
}

InspectionRowModel* PlotInspector::model() const
{
    return model_;
}

int PlotInspector::validCount() const
{
    return validCount_;
}

void PlotInspector::refresh()
{
    pending_ = false;
    if (refreshing_) {
        schedule();
        return;
    }
    const auto guard = QScopedValueRollback<bool>{refreshing_, true};
    auto state = State{};
    if (const auto cursor = cursorPosition()) {
        if (!inspect(*cursor, state)) {
            // A label formatter changed what was queried; the results are stale.
            schedule();
            return;
        }
    } else if (enabled_ && plot_) {
        // Without a cursor the rows stay, so that delegates survive until it returns.
        for (auto* series : inspectedSeries()) {
            auto row = InspectionRow{};
            row.series = series;
            state.rows.push_back(row);
        }
    }
    publish(state);
}

void PlotInspector::schedule()
{
    if (pending_) {
        return;
    }
    pending_ = true;
    QMetaObject::invokeMethod(this, &PlotInspector::flush, Qt::QueuedConnection);
}

void PlotInspector::flush()
{
    if (pending_) {
        refresh();
    }
}

void PlotInspector::reconnect()
{
    for (const auto& connection : std::as_const(connections_)) {
        disconnect(connection);
    }
    connections_.clear();
    schedule();
    if (!plot_ || !enabled_) {
        return;
    }
    connections_.push_back(connect(plot_, &::QAccelPlot::QAccelPlot::pointerChanged, this, &PlotInspector::schedule));
    connections_.push_back(connect(plot_, &::QAccelPlot::QAccelPlot::plotRectChanged, this, &PlotInspector::schedule));
    connections_.push_back(connect(plot_, &::QAccelPlot::QAccelPlot::xAxisChanged, this, &PlotInspector::reconnect));
    connections_.push_back(connect(plot_, &::QAccelPlot::QAccelPlot::yAxisChanged, this, &PlotInspector::reconnect));
    connections_.push_back(connect(plot_, &QObject::destroyed, this, &PlotInspector::reconnect));
    connections_.push_back(connect(plot_, &::QAccelPlot::QAccelPlot::seriesChanged, this, [this] {
        // Rows of removed series go at once, so delegates never see a destroyed series.
        model_->retainSeries(plot_ ? plot_->series() : QList<PlotSeries*>{});
        reconnect();
    }));
    for (auto* axis : {plot_->xAxis(), plot_->yAxis()}) {
        if (axis) {
            connections_.push_back(connect(axis, &Axis::rangeChanged, this, &PlotInspector::schedule));
        }
    }
    const auto series = plot_->series();
    for (auto* item : series) {
        connectSeries(item);
    }
}

void PlotInspector::connectSeries(PlotSeries* series)
{
    connections_.push_back(connect(series, &PlotSeries::dataRevisionChanged, this, &PlotInspector::schedule));
    connections_.push_back(connect(series->inspection(), &SeriesInspection::statusChanged, this, &PlotInspector::schedule));
    connections_.push_back(connect(series, &PlotSeries::xAxisChanged, this, &PlotInspector::reconnect));
    connections_.push_back(connect(series, &PlotSeries::yAxisChanged, this, &PlotInspector::reconnect));
    connections_.push_back(connect(series, &PlotSeries::nameChanged, this, &PlotInspector::schedule));
    connections_.push_back(connect(series, &QQuickItem::visibleChanged, this, &PlotInspector::schedule));
    connections_.push_back(connect(series, &QQuickItem::opacityChanged, this, &PlotInspector::schedule));
    for (auto* axis : {series->xAxis(), series->yAxis()}) {
        if (!axis) {
            continue;
        }
        connections_.push_back(connect(axis, &Axis::rangeChanged, this, &PlotInspector::schedule, Qt::UniqueConnection));
        connections_.push_back(connect(axis->ticker(), &AxisTicker::tickLabelFormatterChanged, this, &PlotInspector::reconnect, Qt::UniqueConnection));
        connections_.push_back(connect(axis->ticker(), &AxisTicker::tickLabelFormatChanged, this, &PlotInspector::schedule, Qt::UniqueConnection));
    }
}

bool PlotInspector::inspects(PlotSeries* series) const
{
    return series->isVisible() && series->opacity() > 0 && (included_.isEmpty() || included_.contains(series)) && !excluded_.contains(series)
        && series->inspection()->supported();
}

QList<PlotSeries*> PlotInspector::inspectedSeries() const
{
    auto result = QList<PlotSeries*>{};
    const auto series = plot_->series();
    std::copy_if(series.cbegin(), series.cend(), std::back_inserter(result), [this](PlotSeries* item) { return inspects(item); });
    return result;
}

std::optional<QPointF> PlotInspector::cursorPosition() const
{
    if (!enabled_ || !plot_ || !plot_->pointerInside()) {
        return std::nullopt;
    }
    return plot_->pointerPosition();
}

bool PlotInspector::inspect(const QPointF& cursor, State& state) const
{
    const auto plot = plot_;
    auto queries = queryRows(cursor);
    formatRows(queries);
    if (plot_ != plot || !plot || !current(queries)) {
        return false;
    }
    state.active = true;
    state.position = snappedPosition(queries, cursor);
    const auto area = plot->plotRect();
    state.cursorX = plot->xAxis() ? plot->pixelToDataX(state.position.x()) : kNaN;
    state.cursorY = plot->yAxis() ? plot->pixelToDataY(state.position.y()) : kNaN;
    state.cursorXText = format(plot->xAxis(), state.cursorX, area.width());
    state.cursorYText = plot ? format(plot->yAxis(), state.cursorY, area.height()) : QString{};
    for (const auto& query : std::as_const(queries)) {
        state.rows.push_back(query.row);
    }
    return plot_ == plot && plot && current(queries);
}

QList<PlotInspector::Query> PlotInspector::queryRows(const QPointF& cursor) const
{
    const auto byX = mode_ == NearestX;
    auto queries = QList<Query>{};
    for (auto* series : inspectedSeries()) {
        auto query = Query{};
        query.row.series = series;
        query.xAxis = series->xAxis();
        query.yAxis = series->yAxis();
        query.boundXAxis = series->xAxis();
        query.boundYAxis = series->yAxis();
        query.size = series->size();
        const auto local = series->mapFromItem(plot_, cursor);
        query.row.sample = byX ? sampleByX(*series->inspection(), local.x()) : series->inspection()->nearest(local, radius_);
        if (query.row.sample.valid()) {
            query.row.pixelPosition = series->mapToItem(plot_, query.row.sample.pixelPosition);
        }
        if (summaries_) {
            query.row.hasSummary = true;
            query.row.summary = summarize(*series, local, byX);
        }
        queries.push_back(query);
    }
    return queries;
}

InspectionSample PlotInspector::sampleByX(SeriesInspection& inspection, const qreal pixelX) const
{
    const auto bracket = inspection.bracketByX(pixelX);
    if (!bracket.valid()) {
        auto result = InspectionSample{};
        result.status = bracket.status;
        result.dataRevision = bracket.dataRevision;
        return result;
    }
    if (interpolate_ && bracket.interpolated.valid()) {
        return bracket.interpolated;
    }
    const auto inside = bracket.left.valid() && bracket.right.valid();
    return inspection.nearestByX(pixelX, inside ? std::numeric_limits<qreal>::infinity() : radius_);
}

InspectionSummary PlotInspector::summarize(PlotSeries& series, const QPointF& local, const bool byX) const
{
    auto result = InspectionSummary{};
    result.dataRevision = series.dataRevision();
    if (!series.xAxis() || !series.yAxis() || !(series.width() > 0) || !(series.height() > 0)) {
        result.status = InspectionStatus::Unavailable;
        return result;
    }
    const auto x0 = series.xAxis()->pixelToCoord(local.x() - summaryRadius_, series.width());
    const auto x1 = series.xAxis()->pixelToCoord(local.x() + summaryRadius_, series.width());
    if (byX) {
        return series.inspection()->summarizeRange(x0, x1);
    }
    const auto y0 = series.yAxis()->pixelToCoord(local.y() - summaryRadius_, series.height());
    const auto y1 = series.yAxis()->pixelToCoord(local.y() + summaryRadius_, series.height());
    return series.inspection()->summarize(x0, x1, y0, y1);
}

void PlotInspector::formatRows(QList<Query>& queries) const
{
    for (auto& query : queries) {
        auto& row = query.row;
        if (row.sample.valid()) {
            row.xText = format(query.xAxis, row.sample.x(), query.size.width());
            row.yText = format(query.yAxis, row.sample.y(), query.size.height());
        }
        if (row.summary.valid()) {
            row.minimumText = format(query.yAxis, row.summary.minimum, query.size.height());
            row.maximumText = format(query.yAxis, row.summary.maximum, query.size.height());
            row.meanText = format(query.yAxis, row.summary.mean, query.size.height());
        }
    }
}

bool PlotInspector::current(const QList<Query>& queries) const
{
    return std::all_of(queries.cbegin(), queries.cend(), [](const Query& query) {
        const auto& series = query.row.series;
        return series && series->dataRevision() == query.row.sample.dataRevision && series->xAxis() == query.boundXAxis && series->yAxis() == query.boundYAxis;
    });
}

QPointF PlotInspector::snappedPosition(const QList<Query>& queries, const QPointF& cursor) const
{
    if (!snapToSample_) {
        return cursor;
    }
    const auto* closest = static_cast<const InspectionRow*>(nullptr);
    for (const auto& query : queries) {
        if (query.row.sample.valid() && (!closest || query.row.sample.distance < closest->sample.distance)) {
            closest = &query.row;
        }
    }
    if (!closest) {
        return cursor;
    }
    return {closest->pixelPosition.x(), mode_ == NearestXY ? closest->pixelPosition.y() : cursor.y()};
}

void PlotInspector::publish(const State& state)
{
    const auto validCount
        = static_cast<int>(std::count_if(state.rows.cbegin(), state.rows.cend(), [](const InspectionRow& row) { return row.sample.valid(); }));
    const auto activeDiffers = active_ != state.active;
    const auto positionDiffers = !samePoint(position_, state.position);
    const auto cursorDiffers
        = !sameValue(cursorX_, state.cursorX) || !sameValue(cursorY_, state.cursorY) || cursorXText_ != state.cursorXText || cursorYText_ != state.cursorYText;
    const auto validCountDiffers = validCount_ != validCount;
    active_ = state.active;
    position_ = state.position;
    cursorX_ = state.cursorX;
    cursorY_ = state.cursorY;
    cursorXText_ = state.cursorXText;
    cursorYText_ = state.cursorYText;
    validCount_ = validCount;
    model_->setRows(state.rows);
    if (positionDiffers) {
        emit positionChanged();
    }
    if (cursorDiffers) {
        emit cursorChanged();
    }
    if (validCountDiffers) {
        emit validCountChanged();
    }
    if (activeDiffers) {
        emit activeChanged();
    }
    emit refreshed();
}

} // namespace QAccelPlot
