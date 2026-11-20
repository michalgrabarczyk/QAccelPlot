

# File GradientStroke.hpp

[**File List**](files.md) **>** [**effects**](dir_2cb1ffa6338b0172fd78b92843e0e53d.md) **>** [**GradientStroke.hpp**](GradientStroke_8hpp.md)

[Go to the documentation of this file](GradientStroke_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/effects/GradientColorTypes.hpp"
#include "QAccelPlot/effects/SeriesEffect.hpp"

#include <QMetaObject>
#include <QObject>
#include <QVector>
#include <QtQml/qqmlregistration.h>

#include <limits>

namespace QAccelPlot {

class GradientStroke : public SeriesEffect {
    Q_OBJECT
    QML_NAMED_ELEMENT(GradientStroke)

    
    Q_PROPERTY(Qt::Orientation direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(QObject* gradient READ gradient WRITE setGradient NOTIFY gradientChanged)
    Q_PROPERTY(Colormap* colormap READ colormap WRITE setColormap NOTIFY colormapChanged)
    Q_PROPERTY(qreal valueMin READ valueMin WRITE setValueMin RESET resetValueMin NOTIFY valueMinChanged)
    Q_PROPERTY(qreal valueMax READ valueMax WRITE setValueMax RESET resetValueMax NOTIFY valueMaxChanged)

public:
    explicit GradientStroke(QObject* parent = nullptr);

    Qt::Orientation direction() const;
    void setDirection(Qt::Orientation direction);

    QObject* gradient() const;
    void setGradient(QObject* gradient);

    Colormap* colormap() const;
    void setColormap(Colormap* colormap);

    qreal valueMin() const;
    void setValueMin(qreal value);
    void resetValueMin();

    qreal valueMax() const;
    void setValueMax(qreal value);
    void resetValueMax();

    const GradientColorPayload& payload() const;

    void refresh() override;

signals:
    void directionChanged();
    void gradientChanged();
    void colormapChanged();
    void valueMinChanged();
    void valueMaxChanged();

private:
    GradientColorPayload readPayload() const;
    void updatePayload();
    void reconnectGradientSignals();
    void disconnectGradientSignals();
    Q_SLOT void onGradientObjectChanged();
    void reconnectColormapSignals();
    void disconnectColormapSignals();

    Qt::Orientation direction_{Qt::Horizontal};
    QObject* gradient_{nullptr};
    QVector<QMetaObject::Connection> gradientConnections_;
    Colormap* colormap_{nullptr};
    QVector<QMetaObject::Connection> colormapConnections_;
    qreal valueMin_{std::numeric_limits<qreal>::quiet_NaN()};
    qreal valueMax_{std::numeric_limits<qreal>::quiet_NaN()};
    GradientColorPayload payload_;
};

} // namespace QAccelPlot
```


