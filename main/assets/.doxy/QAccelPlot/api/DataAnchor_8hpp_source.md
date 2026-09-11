

# File DataAnchor.hpp

[**File List**](files.md) **>** [**annotations**](dir_5d1eadbc468a43ed1cfc69b314db38f6.md) **>** [**DataAnchor.hpp**](DataAnchor_8hpp.md)

[Go to the documentation of this file](DataAnchor_8hpp.md)


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

#include <QQuickItem>
#include <QRectF>
#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

namespace QAccelPlot {

class DataAnchor : public QQuickItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(DataAnchor)

    
    Q_PROPERTY(Axis* xAxis READ xAxis WRITE setXAxis NOTIFY xAxisChanged)

    
    Q_PROPERTY(Axis* yAxis READ yAxis WRITE setYAxis NOTIFY yAxisChanged)

    
    Q_PROPERTY(QRectF plotRect READ plotRect WRITE setPlotRect NOTIFY plotRectChanged)

    
    Q_PROPERTY(qreal dataX1 READ dataX1 WRITE setDataX1 NOTIFY dataX1Changed)

    
    Q_PROPERTY(qreal dataY1 READ dataY1 WRITE setDataY1 NOTIFY dataY1Changed)

    
    Q_PROPERTY(qreal dataX2 READ dataX2 WRITE setDataX2 NOTIFY dataX2Changed)

    
    Q_PROPERTY(qreal dataY2 READ dataY2 WRITE setDataY2 NOTIFY dataY2Changed)

    
    Q_PROPERTY(qreal hoverThreshold READ hoverThreshold WRITE setHoverThreshold NOTIFY hoverThresholdChanged)

public:
    explicit DataAnchor(QQuickItem* parent = nullptr);

    Axis* xAxis() const;
    void setXAxis(Axis* axis);

    Axis* yAxis() const;
    void setYAxis(Axis* axis);

    QRectF plotRect() const;
    void setPlotRect(const QRectF& rect);

    qreal dataX1() const;
    void setDataX1(qreal value);

    qreal dataY1() const;
    void setDataY1(qreal value);

    qreal dataX2() const;
    void setDataX2(qreal value);

    qreal dataY2() const;
    void setDataY2(qreal value);

    qreal hoverThreshold() const;
    void setHoverThreshold(qreal threshold);

    bool contains(const QPointF& point) const override;

signals:
    void xAxisChanged();
    void yAxisChanged();
    void plotRectChanged();
    void dataX1Changed();
    void dataY1Changed();
    void dataX2Changed();
    void dataY2Changed();
    void hoverThresholdChanged();

private:
    void updateGeometry();

    Axis* xAxis_{nullptr};
    Axis* yAxis_{nullptr};
    QRectF plotRect_;
    qreal dataX1_{0.0};
    qreal dataY1_{0.0};
    qreal dataX2_{0.0};
    qreal dataY2_{0.0};
    qreal hoverThreshold_{8.0};
};

} // namespace QAccelPlot
```


