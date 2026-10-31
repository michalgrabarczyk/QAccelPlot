

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
#include "QAccelPlot/series/DataPointStore.hpp"

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
#include <string>
#include <vector>

namespace QAccelPlot {

struct LegendEntry;

class PlotSeries : public QQuickItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(PlotSeries)
    QML_UNCREATABLE("PlotSeries is a base class for concrete plot series.")

    
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(Axis* xAxis READ xAxis WRITE setXAxis NOTIFY xAxisChanged)
    Q_PROPERTY(Axis* yAxis READ yAxis WRITE setYAxis NOTIFY yAxisChanged)
    Q_PROPERTY(QRectF plotRect READ plotRect WRITE setPlotRect NOTIFY plotRectChanged)
    Q_PROPERTY(LegendSymbol legendSymbol READ legendSymbol WRITE setLegendSymbol NOTIFY legendSymbolChanged)
    Q_PROPERTY(bool hoverEnabled READ hoverEnabled WRITE setHoverEnabled NOTIFY hoverEnabledChanged)
    Q_PROPERTY(bool hovered READ hovered NOTIFY hoveredChanged)

    
    Q_PROPERTY(quint64 dataRevision READ dataRevision NOTIFY dataRevisionChanged)
    Q_PROPERTY(SeriesInspection* inspection READ inspection CONSTANT)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum class LegendSymbol { Line, Fill, Marker };
    Q_ENUM(LegendSymbol)

    
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
    virtual int count() const;

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

    bool hoverEnabled() const;
    void setHoverEnabled(bool enabled);
    bool hovered() const;

    virtual std::optional<LegendEntry> legendEntry() const;

    void setData(const double* data, int count);
    void setData(std::vector<double>&& data, int count);
    void setDataF(const float* data, int count);
    void setDataF(std::vector<float>&& data, int count);
    void setData(const double* data, int count, const DataBounds& bounds);
    void setData(std::vector<double>&& data, int count, const DataBounds& bounds);
    void setDataF(const float* data, int count, const DataBounds& bounds);
    void setDataF(std::vector<float>&& data, int count, const DataBounds& bounds);
    void postData(std::vector<double>&& data, int count);
    void postData(std::vector<float>&& data, int count);
    virtual void clearData();

    std::optional<DataExtent> xDataRange() const;
    std::optional<DataExtent> yDataRange() const;

signals:
    void dataRevisionChanged();
    void countChanged();
    void nameChanged();
    void xAxisChanged();
    void yAxisChanged();
    void plotRectChanged();
    void legendSymbolChanged();
    void hoverEnabledChanged();
    void hoveredChanged();
    void legendEntryChanged();

protected:
    struct DataPointWording {
        std::string countName;  
        std::string plural;     
        std::string doubleUnit; 
        std::string floatUnit;  
    };

    PlotSeries(DataPointStore::Layout layout, DataPointWording wording, QQuickItem* parent);

    struct DataPointInput {
        std::vector<double> doubles;        
        std::vector<float> floats;          
        const double* doubleArray{nullptr}; 
        const float* floatArray{nullptr};   
        int count{0};                       
        int stride{0};                      
        bool precise{true};                 

        std::vector<double> toDoubles() &&;
    };

    virtual void applyDataPoints(DataPointInput&& input);
    void storeDataPoints(DataPointInput&& input);
    virtual int dataPointValueCount() const;

    enum class DataChange {
        Replaced, 
        Appended, 
    };

    DataPointStore& dataPoints();
    const DataPointStore& dataPoints() const;
    bool logScaleX() const;
    bool logScaleY() const;
    bool validateRawDataArguments(const void* data, int count) const;
    bool validateDataSize(std::size_t valueCount, int count, bool floatData, int valuesPerDataPoint = 0) const;
    void appendDataPoint(const double* coordinates);
    void notifyDataPointsChanged(DataChange change, int previousCount);
    virtual void onDataPointsChanged(DataChange change, int previousCount);

    void inspectionDataChanged(DataChange change = DataChange::Replaced);
    void invalidateInspection();
    virtual Internal::InspectionSource inspectionSource() const;
    virtual bool inspectionAvailable() const;
    virtual bool recordInspectionSupported() const;
    virtual InspectionRecord inspectionRecord(int index) const;
    virtual InspectionRecord inspectionRecordAt(const QPointF& position) const;
    virtual bool positionInspectionSupported() const;
    virtual InspectionDataPoint inspectionDataPointAt(const QPointF& position) const;

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
    enum class CapacityOverflow {
        DropsExcess, 
        DropsAll,    
    };
    void updateRenderCapacity(int capacity, int recordCount, CapacityOverflow overflow = CapacityOverflow::DropsExcess);
    void supportHover();
    void setHovered(bool hovered);
    bool event(QEvent* event) override;

    int renderCapacity_{std::numeric_limits<int>::max()};
    bool overRenderCapacity_{false};

private:
    friend class SeriesInspection;

    void handleAxisScaleChanged();
    void handleAxisRangeChanged();
    // Returns whether the scale of a dimension changed.
    bool syncDataPointDimensions();
    void ensureDataRanges() const;
    void reportDataRangesChanged() const;
    void deliverTopmostHover(QHoverEvent* event);
    bool coveredBySeriesAbove(const QPointF& position) const;
    void updateAcceptHoverEvents();

    bool hoverSupported_{false};
    bool hoverEnabled_{true};
    bool hovered_{false};
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
    DataPointStore dataPoints_;
    DataPointWording wording_;
};

} // namespace QAccelPlot
```


