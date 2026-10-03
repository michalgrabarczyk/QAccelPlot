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
    QPointer<PlotSeries> series; ///< \brief Series the row describes.
    InspectionSample sample;     ///< \brief Sample at the cursor; invalid for selection rows.
    QPointF pixelPosition;       ///< \brief Sample position in plot-local logical pixels.
    QString xText;               ///< \brief Sample X formatted by the series' X axis.
    QString yText;               ///< \brief Sample Y formatted by the series' Y axis.
    bool hasSummary{false};      ///< \brief Whether \c summary was requested.
    InspectionSummary summary;   ///< \brief Region statistics.
    QString minimumText;         ///< \brief Summary minimum formatted by the series' Y axis.
    QString maximumText;         ///< \brief Summary maximum formatted by the series' Y axis.
    QString meanText;            ///< \brief Summary mean formatted by the series' Y axis.
};

/// \brief List model with one stable row per inspected series.
///
/// Provided by \c PlotInspector::model and \c SelectionTool::model. Rows are added and removed
/// only when the set of inspected series changes; cursor movement updates rows in place, so
/// delegates are reused.
///
/// Roles: \c series, \c seriesName, \c seriesColor, \c valid, \c sampleStatus, \c sampleIndex,
/// \c sampleX, \c sampleY, \c sampleValue, \c xText, \c yText, \c pixelPosition, \c distance,
/// \c interpolated, \c hasSummary, \c summaryStatus, \c summaryCount, \c minimum, \c maximum,
/// \c mean, \c standardDeviation, \c minimumIndex, \c maximumIndex, \c minimumText,
/// \c maximumText, and \c meanText.
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
        SampleStatusRole,
        SampleIndexRole,
        SampleXRole,
        SampleYRole,
        SampleValueRole,
        XTextRole,
        YTextRole,
        PixelPositionRole,
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
    bool sameSeries(const QList<InspectionRow>& rows) const;

    QList<InspectionRow> rows_;
};

} // namespace QAccelPlot
