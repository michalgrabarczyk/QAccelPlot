//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/inspection/InspectionRowModel.hpp"

#include <QColor>

#include <algorithm>

namespace QAccelPlot {
namespace {

QString seriesName(const PlotSeries* series)
{
    if (!series) {
        return {};
    }
    return series->name().isEmpty() ? series->objectName() : series->name();
}

QVariant sampleData(const InspectionRow& row, const int role)
{
    switch (role) {
    case InspectionRowModel::ValidRole:
        return row.sample.valid();
    case InspectionRowModel::SampleStatusRole:
        return QVariant::fromValue(row.sample.status);
    case InspectionRowModel::SampleIndexRole:
        return row.sample.index;
    case InspectionRowModel::SampleXRole:
        return row.sample.x();
    case InspectionRowModel::SampleYRole:
        return row.sample.y();
    case InspectionRowModel::SampleValueRole:
        return row.sample.value;
    case InspectionRowModel::XTextRole:
        return row.xText;
    case InspectionRowModel::YTextRole:
        return row.yText;
    case InspectionRowModel::PixelPositionRole:
        return row.pixelPosition;
    case InspectionRowModel::DistanceRole:
        return row.sample.distance;
    case InspectionRowModel::InterpolatedRole:
        return row.sample.interpolated;
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
        return row.series ? row.series->property("color") : QVariant{QColor{}};
    default:
        return role < HasSummaryRole ? sampleData(row, role) : summaryData(row, role);
    }
}

QHash<int, QByteArray> InspectionRowModel::roleNames() const
{
    return {{SeriesRole, "series"}, {SeriesNameRole, "seriesName"}, {SeriesColorRole, "seriesColor"}, {ValidRole, "valid"}, {SampleStatusRole, "sampleStatus"},
        {SampleIndexRole, "sampleIndex"}, {SampleXRole, "sampleX"}, {SampleYRole, "sampleY"}, {SampleValueRole, "sampleValue"}, {XTextRole, "xText"},
        {YTextRole, "yText"}, {PixelPositionRole, "pixelPosition"}, {DistanceRole, "distance"}, {InterpolatedRole, "interpolated"},
        {HasSummaryRole, "hasSummary"}, {SummaryStatusRole, "summaryStatus"}, {SummaryCountRole, "summaryCount"}, {MinimumRole, "minimum"},
        {MaximumRole, "maximum"}, {MeanRole, "mean"}, {StandardDeviationRole, "standardDeviation"}, {MinimumIndexRole, "minimumIndex"},
        {MaximumIndexRole, "maximumIndex"}, {MinimumTextRole, "minimumText"}, {MaximumTextRole, "maximumText"}, {MeanTextRole, "meanText"}};
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
    rows_ = rows;
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
        rows_.removeAt(row);
        endRemoveRows();
        removed = true;
    }
    if (removed) {
        emit countChanged();
    }
}

bool InspectionRowModel::sameSeries(const QList<InspectionRow>& rows) const
{
    return rows.size() == rows_.size()
        && std::equal(rows.cbegin(), rows.cend(), rows_.cbegin(), [](const InspectionRow& a, const InspectionRow& b) { return a.series == b.series; });
}

} // namespace QAccelPlot
