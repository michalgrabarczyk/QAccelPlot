

# File InspectionRowModel.hpp

[**File List**](files.md) **>** [**inspection**](dir_7c3af00b227ed418fdf47d7cd69e8a77.md) **>** [**InspectionRowModel.hpp**](InspectionRowModel_8hpp.md)

[Go to the documentation of this file](InspectionRowModel_8hpp.md)


```C++
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

struct InspectionRow {
    QPointer<PlotSeries> series; 
    InspectionSample sample;     
    QPointF pixelPosition;       
    QString xText;               
    QString yText;               
    bool hasSummary{false};      
    InspectionSummary summary;   
    QString minimumText;         
    QString maximumText;         
    QString meanText;            
};

class InspectionRowModel : public QAbstractListModel {
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
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

    explicit InspectionRowModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const;
    Q_INVOKABLE QVariantMap get(int row) const;

    const QList<InspectionRow>& rows() const;
    void setRows(const QList<InspectionRow>& rows);
    void retainSeries(const QList<PlotSeries*>& series);

signals:
    void countChanged();

private:
    bool sameSeries(const QList<InspectionRow>& rows) const;

    QList<InspectionRow> rows_;
};

} // namespace QAccelPlot
```


