//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/series/PlotSeries.hpp"

#include <QAbstractListModel>
#include <QList>
#include <QPointer>

namespace QAccelPlot {

/// \brief List model of the legend entries of a list of series.
///
/// Has one row per \c PlotSeries::legendEntries() entry of each visible series in \c series: one
/// row for most series, and one per category for a series colored by category. Series without an
/// entry, such as \c HeatmapSeries, and hidden series have no row. Rows follow changes of the
/// series' entries, names, and visibility.
///
/// Roles: \c series, \c name (the entry's \c name, else the series' \c name, its \c objectName, or
/// "Series N"), and \c entry (a \c LegendEntry). \c Plot provides one over all its series as
/// \c legendModel:
///
/// \code
/// QAccelPlot.Plot {
///     id: plot
///     legend: QAccelPlot.Legend { model: QAccelPlot.LegendModel { series: [signal, fit] } }
/// }
/// \endcode
///
/// \sa LegendEntry, PlotSeries::legendEntries()
class LegendModel : public QAbstractListModel {
    Q_OBJECT
    QML_NAMED_ELEMENT(LegendModel)

    /// \brief Series to list, in legend order. Default: empty.
    Q_PROPERTY(QList<PlotSeries*> series READ series WRITE setSeries NOTIFY seriesChanged)
    /// \brief Read-only: number of rows.
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    /// \brief Data roles; the QML role names are listed in the class description.
    enum Role {
        SeriesRole = Qt::UserRole + 1,
        NameRole,
        EntryRole,
    };

    /// \brief Constructs an empty model with the given \a parent.
    explicit LegendModel(QObject* parent = nullptr);

    /// \brief Returns the listed series.
    QList<PlotSeries*> series() const;
    /// \brief Sets the listed series to \a series.
    void setSeries(const QList<PlotSeries*>& series);

    /// \brief Returns the number of rows.
    int count() const;

    /// \brief Returns the number of rows; \a parent must be invalid.
    int rowCount(const QModelIndex& parent = {}) const override;
    /// \brief Returns the value of \a role for the row at \a index.
    QVariant data(const QModelIndex& index, int role) const override;
    /// \brief Returns the QML role names.
    QHash<int, QByteArray> roleNames() const override;

signals:
    /// \brief Emitted when the series property changes.
    void seriesChanged();
    /// \brief Emitted when the number of rows changes.
    void countChanged();

private:
    void watchSeries(PlotSeries* series) const;
    void unwatchSeries(PlotSeries* series);
    void onSeriesDestroyed();
    /// \brief Recomputes the rows: resets the model when the listed rows change, and reports changed data otherwise.
    void refreshRows();

    // One legend row: entry \c entry of \c series' legendEntries().
    struct Row {
        QPointer<PlotSeries> series;
        int entry{0};

        bool operator==(const Row& other) const;
    };

    QList<QPointer<PlotSeries>> series_;
    QList<Row> rows_;
};

} // namespace QAccelPlot
