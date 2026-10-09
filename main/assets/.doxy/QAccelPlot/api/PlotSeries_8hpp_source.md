

# File PlotSeries.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**PlotSeries.hpp**](PlotSeries_8hpp.md)

[Go to the documentation of this file](PlotSeries_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/inspection/SeriesInspection.hpp"

#include <QPointer>
#include <QQuickItem>
#include <QRectF>
#include <QString>
#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <limits>
#include <optional>
#include <vector>

namespace QAccelPlot {

class PlotSeries : public QQuickItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(PlotSeries)
    QML_UNCREATABLE("PlotSeries is a base class for concrete plot series.")

    
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(Axis* xAxis READ xAxis WRITE setXAxis NOTIFY xAxisChanged)
    Q_PROPERTY(Axis* yAxis READ yAxis WRITE setYAxis NOTIFY yAxisChanged)
    Q_PROPERTY(QRectF plotRect READ plotRect WRITE setPlotRect NOTIFY plotRectChanged)
    Q_PROPERTY(LegendSymbol legendSymbol READ legendSymbol WRITE setLegendSymbol NOTIFY legendSymbolChanged)

    
    Q_PROPERTY(quint64 dataRevision READ dataRevision NOTIFY dataRevisionChanged)
    Q_PROPERTY(::QAccelPlot::SeriesInspection* inspection READ inspection CONSTANT)

public:
    enum class LegendSymbol { Line, Fill, Marker };
    Q_ENUM(LegendSymbol)

    
    enum class MarkerShape {
        None,          
        Circle,        
        Square,        
        Diamond,       
        TriangleUp,    
        TriangleDown,  
        TriangleLeft,  
        TriangleRight, 
        Cross,         
        XCross,        
        HLine,         
        VLine,         
        Star,          
        Asterisk,      
        Pixel,         
        Hexagon,       
        Pentagon       
    };
    Q_ENUM(MarkerShape)

    
    struct DataExtent {
        qreal min; 
        qreal max; 
    };

    struct DataBounds {
        qreal xMin; 
        qreal xMax; 
        qreal yMin; 
        qreal yMax; 
    };

    explicit PlotSeries(QQuickItem* parent = nullptr);
    ~PlotSeries() override;

    quint64 dataRevision() const;
    SeriesInspection* inspection() const;

    QString name() const;
    void setName(const QString& name);

    Axis* xAxis() const;
    void setXAxis(Axis* axis);

    Axis* yAxis() const;
    void setYAxis(Axis* axis);

    QRectF plotRect() const;
    void setPlotRect(const QRectF& rect);

    LegendSymbol legendSymbol() const;
    void setLegendSymbol(LegendSymbol symbol);

    virtual void setData(const double* data, int count) = 0;
    virtual void setData(std::vector<double>&& data, int count) = 0;
    virtual void setDataF(const float* data, int count) = 0;
    virtual void setDataF(std::vector<float>&& data, int count) = 0;
    void setData(const double* data, int count, const DataBounds& bounds);
    void setData(std::vector<double>&& data, int count, const DataBounds& bounds);
    void setDataF(const float* data, int count, const DataBounds& bounds);
    void setDataF(std::vector<float>&& data, int count, const DataBounds& bounds);
    virtual void postData(std::vector<double>&& data, int count) = 0;
    virtual void postData(std::vector<float>&& data, int count) = 0;
    virtual void clearData() = 0;

    std::optional<DataExtent> xDataRange() const;
    std::optional<DataExtent> yDataRange() const;

signals:
    void dataRevisionChanged();
    void nameChanged();
    void xAxisChanged();
    void yAxisChanged();
    void plotRectChanged();
    void legendSymbolChanged();

protected:
    enum class DataChange {
        Replaced, 
        Appended, 
    };

    void inspectionDataChanged(DataChange change = DataChange::Replaced);
    void invalidateInspection();
    virtual InspectionSource inspectionSource() const;
    virtual bool inspectionAvailable() const;
    virtual InspectionRecord inspectionRecord(int index) const;
    virtual InspectionRecord inspectionRecordAt(const QPointF& position) const;

    struct DataRanges {
        std::optional<DataExtent> x; 
        std::optional<DataExtent> y; 
    };

    virtual DataRanges computeDataRanges() const;
    void invalidateDataRanges();
    void extendXDataRange(qreal x);
    void extendYDataRange(qreal y);
    virtual void onAxisScaleChanged();
    virtual void onAxisRangeChanged();
    QRectF resolvePlotRect() const;
    bool event(QEvent* event) override;

private:
    friend class SeriesInspection;

    void ensureDataRanges() const;
    void reportDataRangesChanged() const;
    void deliverTopmostHover(QHoverEvent* event);
    bool coveredBySeriesAbove(const QPointF& position) const;

    bool hoverDelivered_{false};
    quint64 dataRevision_{0};
    mutable SeriesInspection* inspection_{nullptr};
    QString name_;
    QPointer<Axis> xAxis_;
    QPointer<Axis> yAxis_;
    QMetaObject::Connection xAxisDestroyed_;
    QMetaObject::Connection yAxisDestroyed_;
    QRectF plotRect_;
    LegendSymbol legendSymbol_{LegendSymbol::Line};
    // The extents are a cache of a scan over the records, refreshed by const readers.
    mutable std::optional<DataExtent> xDataExtent_;
    mutable std::optional<DataExtent> yDataExtent_;
    mutable bool dataRangesStale_{false};
    // Bounds given with the data update in progress; they replace its scan.
    std::optional<DataBounds> updateBounds_;
};

} // namespace QAccelPlot
```


