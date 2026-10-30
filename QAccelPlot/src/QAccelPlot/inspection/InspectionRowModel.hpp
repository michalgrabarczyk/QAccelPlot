//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/inspection/InspectionResult.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"

#include <QAbstractListModel>
#include <QPointer>

namespace QAccelPlot {

/// \brief Inspection result of one series, as shown by one row of an \c InspectionRowModel.
struct InspectionRow {
    QPointer<PlotSeries> series;   ///< \brief Series the row describes.
    InspectionDataPoint dataPoint; ///< \brief Data point at the cursor; invalid for selection rows.
    qreal pixelX{0.0};             ///< \brief Data point X in plot-local logical pixels.
    qreal pixelY{0.0};             ///< \brief Data point Y in plot-local logical pixels.
    QString xText;                 ///< \brief Data point X formatted by the series' X axis.
    QString yText;                 ///< \brief Data point Y formatted by the series' Y axis.
    QString valueText;             ///< \brief Data point value with up to six significant digits; empty when it is NaN.
    bool hasSummary{false};        ///< \brief Whether \c summary was requested.
    InspectionSummary summary;     ///< \brief Region statistics.
    QString minimumText;           ///< \brief Summary minimum formatted by the series' Y axis.
    QString maximumText;           ///< \brief Summary maximum formatted by the series' Y axis.
    QString meanText;              ///< \brief Summary mean formatted by the series' Y axis.
};

/// \brief List model with one stable row per inspected series.
///
/// Provided by \c PlotInspector::model and \c SelectionTool::model. Rows are added and removed
/// only when the set of inspected series changes; cursor movement updates rows in place, so
/// delegates are reused.
///
/// Roles: \c series, \c seriesName, \c seriesColor, \c valid, \c dataPointStatus, \c dataPointIndex,
/// \c dataPointX, \c dataPointY, \c dataPointValue, \c xText, \c yText, \c valueText, \c pixelX, \c pixelY,
/// \c distance, \c interpolated, \c hasSummary, \c summaryStatus, \c summaryCount, \c minimum, \c maximum,
/// \c mean, \c standardDeviation, \c minimumIndex, \c maximumIndex, \c minimumText,
/// \c maximumText, and \c meanText.
///
/// \c pixelX and \c pixelY are in plot-local logical pixels, unlike the series-local
/// \c InspectionDataPoint::pixelX and \c InspectionDataPoint::pixelY.
class InspectionRowModel : public QAbstractListModel {
    Q_OBJECT
    QML_ANONYMOUS
    /// \brief Number of rows.
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    /// \brief Data roles; the QML role names are listed in the class description.
    enum Role {
        SeriesRole = Qt::UserRole + 1,
        SeriesNameRole,
        SeriesColorRole,
        ValidRole,
        DataPointStatusRole,
        DataPointIndexRole,
        DataPointXRole,
        DataPointYRole,
        DataPointValueRole,
        XTextRole,
        YTextRole,
        ValueTextRole,
        PixelXRole,
        PixelYRole,
        DistanceRole,
        InterpolatedRole,
        HasSummaryRole,
        SummaryStatusRole,
        SummaryCountRole,
        MinimumRole,
        MaximumRole,
        MeanRole,
        StandardDeviationRole,
        MinimumIndexRole,
        MaximumIndexRole,
        MinimumTextRole,
        MaximumTextRole,
        MeanTextRole,
    };

    /// \brief Constructs an empty model.
    explicit InspectionRowModel(QObject* parent = nullptr);

    /// \brief Returns the number of rows.
    int rowCount(const QModelIndex& parent = {}) const override;
    /// \brief Returns the value of \a role for the row at \a index.
    QVariant data(const QModelIndex& index, int role) const override;
    /// \brief Returns the QML role names.
    QHash<int, QByteArray> roleNames() const override;

    /// \brief Returns the number of rows.
    int count() const;
    /// \brief Returns every role of \a row keyed by role name, or an empty map when out of range.
    Q_INVOKABLE QVariantMap get(int row) const;

    /// \brief Returns the current rows.
    const QList<InspectionRow>& rows() const;
    /// \brief Replaces the rows, updating them in place when the series are unchanged.
    void setRows(const QList<InspectionRow>& rows);
    /// \brief Removes rows whose series is destroyed or not in \a series.
    void retainSeries(const QList<PlotSeries*>& series);

signals:
    /// \brief Emitted when the count property changes.
    void countChanged();

private:
    Q_SLOT void seriesAppearanceChanged();
    void watchSeries(PlotSeries* series) const;
    void unwatchSeries(PlotSeries* series);
    bool sameSeries(const QList<InspectionRow>& rows) const;

    QList<InspectionRow> rows_;
};

} // namespace QAccelPlot
