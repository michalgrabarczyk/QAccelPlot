

# File PlotSeries.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**series**](dir_d1bb17d10be635dda10fdf13c9e6bbc5.md) **>** [**PlotSeries.hpp**](PlotSeries_8hpp.md)

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

#include "axis/Axis.hpp"

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

public:
    enum class LegendSymbol { Line, Fill };
    Q_ENUM(LegendSymbol)

    explicit PlotSeries(QQuickItem* parent = nullptr);

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

signals:
    void nameChanged();
    void xAxisChanged();
    void yAxisChanged();
    void plotRectChanged();
    void legendSymbolChanged();
    void xDataRangeChanged(qreal min, qreal max);
    void yDataRangeChanged(qreal min, qreal max);

protected:
    void setDataRanges(qreal xMin, qreal xMax, qreal yMin, qreal yMax);
    void clearDataRanges();

private:
    void onAxisRangeChanged();
    void reportXDataRangeToAxis() const;
    void reportYDataRangeToAxis() const;

    QString name_;
    QPointer<Axis> xAxis_;
    QPointer<Axis> yAxis_;
    QRectF plotRect_;
    LegendSymbol legendSymbol_{LegendSymbol::Line};
    qreal lastXMin_{std::numeric_limits<qreal>::max()};
    qreal lastXMax_{std::numeric_limits<qreal>::lowest()};
    qreal lastYMin_{std::numeric_limits<qreal>::max()};
    qreal lastYMax_{std::numeric_limits<qreal>::lowest()};
};

} // namespace QAccelPlot
```


