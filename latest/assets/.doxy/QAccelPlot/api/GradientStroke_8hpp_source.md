

# File GradientStroke.hpp

[**File List**](files.md) **>** [**effects**](dir_2cb1ffa6338b0172fd78b92843e0e53d.md) **>** [**GradientStroke.hpp**](GradientStroke_8hpp.md)

[Go to the documentation of this file](GradientStroke_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/effects/GradientColorTypes.hpp"
#include "QAccelPlot/effects/LineCurveEffect.hpp"

#include <QMetaObject>
#include <QObject>
#include <QVector>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

class GradientStroke : public LineCurveEffect {
    Q_OBJECT
    QML_NAMED_ELEMENT(GradientStroke)

    
    Q_PROPERTY(Qt::Orientation direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(QObject* gradient READ gradient WRITE setGradient NOTIFY gradientChanged)
    Q_PROPERTY(Colormap* colormap READ colormap WRITE setColormap NOTIFY colormapChanged)
    Q_PROPERTY(GradientValueSource valueMinSource READ valueMinSource WRITE setValueMinSource NOTIFY valueMinSourceChanged)
    Q_PROPERTY(qreal valueMin READ valueMin WRITE setValueMin NOTIFY valueMinChanged)
    Q_PROPERTY(GradientValueSource valueMaxSource READ valueMaxSource WRITE setValueMaxSource NOTIFY valueMaxSourceChanged)
    Q_PROPERTY(qreal valueMax READ valueMax WRITE setValueMax NOTIFY valueMaxChanged)

public:
    explicit GradientStroke(QObject* parent = nullptr);

    Qt::Orientation direction() const;
    void setDirection(Qt::Orientation direction);

    QObject* gradient() const;
    void setGradient(QObject* gradient);

    Colormap* colormap() const;
    void setColormap(Colormap* colormap);

    GradientValueSource valueMinSource() const;
    void setValueMinSource(GradientValueSource source);

    qreal valueMin() const;
    void setValueMin(qreal value);

    GradientValueSource valueMaxSource() const;
    void setValueMaxSource(GradientValueSource source);

    qreal valueMax() const;
    void setValueMax(qreal value);

    const GradientColorPayload& payload() const;

    void refresh() override;

signals:
    void directionChanged();
    void gradientChanged();
    void colormapChanged();
    void valueMinSourceChanged();
    void valueMinChanged();
    void valueMaxSourceChanged();
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
    GradientValueSource valueMinSource_{GradientValueSource::DataRange};
    qreal valueMin_{0.0};
    GradientValueSource valueMaxSource_{GradientValueSource::DataRange};
    qreal valueMax_{0.0};
    GradientColorPayload payload_;
};

} // namespace QAccelPlot
```


