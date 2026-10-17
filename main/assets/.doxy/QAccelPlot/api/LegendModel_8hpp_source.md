

# File LegendModel.hpp

[**File List**](files.md) **>** [**legend**](dir_5fa9a39c33e751e7d5879623bc62e73e.md) **>** [**LegendModel.hpp**](LegendModel_8hpp.md)

[Go to the documentation of this file](LegendModel_8hpp.md)


```C++
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

class LegendModel : public QAbstractListModel {
    Q_OBJECT
    QML_NAMED_ELEMENT(LegendModel)

    
    Q_PROPERTY(QList<::QAccelPlot::PlotSeries*> series READ series WRITE setSeries NOTIFY seriesChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        SeriesRole = Qt::UserRole + 1,
        NameRole,
        EntryRole,
    };

    explicit LegendModel(QObject* parent = nullptr);

    QList<PlotSeries*> series() const;
    void setSeries(const QList<PlotSeries*>& series);

    int count() const;

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void seriesChanged();
    void countChanged();

private:
    void watchSeries(PlotSeries* series) const;
    void unwatchSeries(PlotSeries* series);
    void onSeriesDestroyed(QObject* series);
    void refreshRows();

    QList<QPointer<PlotSeries>> series_;
    QList<QPointer<PlotSeries>> rows_;
};

} // namespace QAccelPlot
```


