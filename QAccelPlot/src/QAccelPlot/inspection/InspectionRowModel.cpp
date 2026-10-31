//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/inspection/InspectionRowModel.hpp"

#include "QAccelPlot/legend/LegendEntry.hpp"

#include <QColor>

#include <algorithm>
#include <utility>

namespace QAccelPlot {
namespace {

QString seriesName(const PlotSeries* series)
{
    if (!series) {
        return {};
    }
    return series->name().isEmpty() ? series->objectName() : series->name();
}

// The color of the series' legend entry; invalid for a series without one.
QColor seriesColor(const PlotSeries* series)
{
    const auto entry = series ? series->legendEntry() : std::nullopt;
    return entry ? entry->color : QColor{};
}

QVariant dataPointData(const InspectionRow& row, const int role)
{
    switch (role) {
    case InspectionRowModel::ValidRole:
        return row.dataPoint.valid();
    case InspectionRowModel::DataPointStatusRole:
        return QVariant::fromValue(row.dataPoint.status);
    case InspectionRowModel::DataPointIndexRole:
        return row.dataPoint.index;
    case InspectionRowModel::DataPointXRole:
        return row.dataPoint.x;
    case InspectionRowModel::DataPointYRole:
        return row.dataPoint.y;
    case InspectionRowModel::DataPointValueRole:
        return row.dataPoint.value;
    case InspectionRowModel::XTextRole:
        return row.xText;
    case InspectionRowModel::YTextRole:
        return row.yText;
    case InspectionRowModel::ValueTextRole:
        return row.valueText;
    case InspectionRowModel::PixelXRole:
        return row.pixelX;
    case InspectionRowModel::PixelYRole:
        return row.pixelY;
    case InspectionRowModel::DistanceRole:
        return row.dataPoint.distance;
    case InspectionRowModel::InterpolatedRole:
        return row.dataPoint.interpolated;
    default:
        return {};
    }
}

QVariant summaryData(const InspectionRow& row, const int role)
{
    switch (role) {
    case InspectionRowModel::HasSummaryRole:
        return row.hasSummary;
    case InspectionRowModel::SummaryStatusRole:
        return QVariant::fromValue(row.summary.status);
    case InspectionRowModel::SummaryCountRole:
        return row.summary.count;
    case InspectionRowModel::MinimumRole:
        return row.summary.minimum;
    case InspectionRowModel::MaximumRole:
        return row.summary.maximum;
    case InspectionRowModel::MeanRole:
        return row.summary.mean;
    case InspectionRowModel::StandardDeviationRole:
        return row.summary.standardDeviation;
    case InspectionRowModel::MinimumIndexRole:
        return row.summary.minimumIndex;
    case InspectionRowModel::MaximumIndexRole:
        return row.summary.maximumIndex;
    case InspectionRowModel::MinimumTextRole:
        return row.minimumText;
    case InspectionRowModel::MaximumTextRole:
        return row.maximumText;
    case InspectionRowModel::MeanTextRole:
        return row.meanText;
    default:
        return {};
    }
}

} // namespace

InspectionRowModel::InspectionRowModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int InspectionRowModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(rows_.size());
}

QVariant InspectionRowModel::data(const QModelIndex& index, const int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rows_.size()) {
        return {};
    }
    const auto& row = rows_.at(index.row());
    switch (role) {
    case SeriesRole:
        return QVariant::fromValue(row.series.data());
    case SeriesNameRole:
        return seriesName(row.series);
    case SeriesColorRole:
        return seriesColor(row.series);
    default:
        return role < HasSummaryRole ? dataPointData(row, role) : summaryData(row, role);
    }
}

QHash<int, QByteArray> InspectionRowModel::roleNames() const
{
    return {{SeriesRole, "series"}, {SeriesNameRole, "seriesName"}, {SeriesColorRole, "seriesColor"}, {ValidRole, "valid"},
        {DataPointStatusRole, "dataPointStatus"}, {DataPointIndexRole, "dataPointIndex"}, {DataPointXRole, "dataPointX"}, {DataPointYRole, "dataPointY"},
        {DataPointValueRole, "dataPointValue"}, {XTextRole, "xText"}, {YTextRole, "yText"}, {ValueTextRole, "valueText"}, {PixelXRole, "pixelX"},
        {PixelYRole, "pixelY"}, {DistanceRole, "distance"}, {InterpolatedRole, "interpolated"}, {HasSummaryRole, "hasSummary"},
        {SummaryStatusRole, "summaryStatus"}, {SummaryCountRole, "summaryCount"}, {MinimumRole, "minimum"}, {MaximumRole, "maximum"}, {MeanRole, "mean"},
        {StandardDeviationRole, "standardDeviation"}, {MinimumIndexRole, "minimumIndex"}, {MaximumIndexRole, "maximumIndex"}, {MinimumTextRole, "minimumText"},
        {MaximumTextRole, "maximumText"}, {MeanTextRole, "meanText"}};
}

int InspectionRowModel::count() const
{
    return static_cast<int>(rows_.size());
}

QVariantMap InspectionRowModel::get(const int row) const
{
    auto result = QVariantMap{};
    if (row < 0 || row >= rows_.size()) {
        return result;
    }
    const auto names = roleNames();
    for (auto it = names.cbegin(); it != names.cend(); ++it) {
        result.insert(QString::fromLatin1(it.value()), data(index(row), it.key()));
    }
    return result;
}

const QList<InspectionRow>& InspectionRowModel::rows() const
{
    return rows_;
}

void InspectionRowModel::setRows(const QList<InspectionRow>& rows)
{
    if (sameSeries(rows)) {
        rows_ = rows;
        if (!rows_.isEmpty()) {
            emit dataChanged(index(0), index(static_cast<int>(rows_.size()) - 1));
        }
        return;
    }
    const auto countDiffers = rows_.size() != rows.size();
    beginResetModel();
    for (const auto& row : std::as_const(rows_)) {
        unwatchSeries(row.series);
    }
    rows_ = rows;
    for (const auto& row : std::as_const(rows_)) {
        watchSeries(row.series);
    }
    endResetModel();
    if (countDiffers) {
        emit countChanged();
    }
}

void InspectionRowModel::retainSeries(const QList<PlotSeries*>& series)
{
    auto removed = false;
    for (auto row = static_cast<int>(rows_.size()) - 1; row >= 0; --row) {
        const auto& current = rows_.at(row).series;
        if (current && series.contains(current.data())) {
            continue;
        }
        beginRemoveRows({}, row, row);
        unwatchSeries(current);
        rows_.removeAt(row);
        endRemoveRows();
        removed = true;
    }
    if (removed) {
        emit countChanged();
    }
}

void InspectionRowModel::seriesAppearanceChanged()
{
    const auto* series = sender();
    for (auto row = 0; row < rows_.size(); ++row) {
        if (rows_[row].series.data() == series) {
            emit dataChanged(index(row), index(row), {SeriesNameRole, SeriesColorRole});
        }
    }
}

void InspectionRowModel::watchSeries(PlotSeries* series) const
{
    if (!series) {
        return;
    }
    connect(series, &PlotSeries::nameChanged, this, &InspectionRowModel::seriesAppearanceChanged, Qt::UniqueConnection);
    connect(series, &QObject::objectNameChanged, this, &InspectionRowModel::seriesAppearanceChanged, Qt::UniqueConnection);
    connect(series, &PlotSeries::legendEntryChanged, this, &InspectionRowModel::seriesAppearanceChanged, Qt::UniqueConnection);
}

void InspectionRowModel::unwatchSeries(PlotSeries* series)
{
    // The model receives nothing else from a series, so this removes exactly the watchSeries() connections.
    if (series) {
        disconnect(series, nullptr, this, nullptr);
    }
}

bool InspectionRowModel::sameSeries(const QList<InspectionRow>& rows) const
{
    return rows.size() == rows_.size()
        && std::equal(rows.cbegin(), rows.cend(), rows_.cbegin(), [](const InspectionRow& a, const InspectionRow& b) { return a.series == b.series; });
}

} // namespace QAccelPlot
