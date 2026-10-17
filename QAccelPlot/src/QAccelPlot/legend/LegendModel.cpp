//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/legend/LegendModel.hpp"

#include "QAccelPlot/legend/LegendEntry.hpp"

#include <algorithm>

namespace QAccelPlot {

namespace {

bool listsSeries(const QList<QPointer<PlotSeries>>& listed, const QList<PlotSeries*>& series)
{
    return std::equal(listed.cbegin(), listed.cend(), series.cbegin(), series.cend(),
        [](const QPointer<PlotSeries>& left, const PlotSeries* right) { return left.data() == right; });
}

} // namespace

LegendModel::LegendModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QList<PlotSeries*> LegendModel::series() const
{
    auto series = QList<PlotSeries*>{};
    for (const auto& listed : series_) {
        if (listed) {
            series.append(listed.data());
        }
    }
    return series;
}

void LegendModel::setSeries(const QList<PlotSeries*>& series)
{
    if (listsSeries(series_, series)) {
        return;
    }
    for (const auto& listed : std::as_const(series_)) {
        unwatchSeries(listed.data());
    }
    series_.clear();
    for (auto* added : series) {
        series_.append(added);
        watchSeries(added);
    }
    refreshRows();
    emit seriesChanged();
}

int LegendModel::count() const
{
    return static_cast<int>(rows_.size());
}

int LegendModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : count();
}

QVariant LegendModel::data(const QModelIndex& index, const int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= count()) {
        return {};
    }
    const auto* series = rows_[index.row()].data();
    if (!series) {
        return {};
    }
    switch (role) {
    case SeriesRole:
        return QVariant::fromValue(const_cast<PlotSeries*>(series));
    case NameRole:
        if (!series->name().isEmpty()) {
            return series->name();
        }
        if (!series->objectName().isEmpty()) {
            return series->objectName();
        }
        return QStringLiteral("Series %1").arg(index.row() + 1);
    case EntryRole: {
        const auto entry = series->legendEntry();
        return entry ? QVariant::fromValue(*entry) : QVariant{};
    }
    default:
        return {};
    }
}

QHash<int, QByteArray> LegendModel::roleNames() const
{
    return {{SeriesRole, "series"}, {NameRole, "name"}, {EntryRole, "entry"}};
}

void LegendModel::watchSeries(PlotSeries* series) const
{
    if (!series) {
        return;
    }
    connect(series, &PlotSeries::legendEntryChanged, this, &LegendModel::refreshRows);
    connect(series, &PlotSeries::nameChanged, this, &LegendModel::refreshRows);
    connect(series, &QObject::objectNameChanged, this, &LegendModel::refreshRows);
    connect(series, &QQuickItem::visibleChanged, this, &LegendModel::refreshRows);
    connect(series, &QObject::destroyed, this, &LegendModel::onSeriesDestroyed);
}

void LegendModel::unwatchSeries(PlotSeries* series)
{
    // The model receives nothing else from a series, so this removes exactly the watchSeries() connections.
    if (series) {
        disconnect(series, nullptr, this, nullptr);
    }
}

void LegendModel::onSeriesDestroyed(QObject* /*series*/)
{
    // The destroyed series' pointers are already null.
    series_.removeAll(QPointer<PlotSeries>{});
    refreshRows();
    emit seriesChanged();
}

void LegendModel::refreshRows()
{
    auto rows = QList<QPointer<PlotSeries>>{};
    for (const auto& series : std::as_const(series_)) {
        if (series && series->isVisible() && series->legendEntry()) {
            rows.append(series);
        }
    }
    if (rows == rows_) {
        if (!rows_.isEmpty()) {
            emit dataChanged(index(0), index(count() - 1));
        }
        return;
    }
    const auto previousCount = count();
    beginResetModel();
    rows_ = std::move(rows);
    endResetModel();
    if (count() != previousCount) {
        emit countChanged();
    }
}

} // namespace QAccelPlot
