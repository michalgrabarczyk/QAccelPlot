

# File GradientFill.hpp

[**File List**](files.md) **>** [**effects**](dir_a4c3deeda37ae6198148ecdf2e43fc36.md) **>** [**GradientFill.hpp**](GradientFill_8hpp.md)

[Go to the documentation of this file](GradientFill_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "effects/LineCurveEffect.hpp"
#include "effects/GradientColorTypes.hpp"

#include <QMetaObject>
#include <QObject>
#include <QVector>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

class GradientFill : public LineCurveEffect {
    Q_OBJECT
    QML_NAMED_ELEMENT(GradientFill)

    
    Q_PROPERTY(GradientDirection direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(QObject* gradient READ gradient WRITE setGradient NOTIFY gradientChanged)
    Q_PROPERTY(GradientValueSource gradientValueMinSource READ gradientValueMinSource WRITE setGradientValueMinSource NOTIFY gradientValueMinSourceChanged)
    Q_PROPERTY(qreal gradientValueMin READ gradientValueMin WRITE setGradientValueMin NOTIFY gradientValueMinChanged)
    Q_PROPERTY(GradientValueSource gradientValueMaxSource READ gradientValueMaxSource WRITE setGradientValueMaxSource NOTIFY gradientValueMaxSourceChanged)
    Q_PROPERTY(qreal gradientValueMax READ gradientValueMax WRITE setGradientValueMax NOTIFY gradientValueMaxChanged)
    Q_PROPERTY(GradientFillBaseline baseline READ baseline WRITE setBaseline NOTIFY baselineChanged)
    Q_PROPERTY(qreal baselineValue READ baselineValue WRITE setBaselineValue NOTIFY baselineValueChanged)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)

public:
    explicit GradientFill(QObject* parent = nullptr);

    GradientDirection direction() const;
    void setDirection(GradientDirection direction);

    QObject* gradient() const;
    void setGradient(QObject* gradient);

    GradientValueSource gradientValueMinSource() const;
    void setGradientValueMinSource(GradientValueSource source);

    qreal gradientValueMin() const;
    void setGradientValueMin(qreal value);

    GradientValueSource gradientValueMaxSource() const;
    void setGradientValueMaxSource(GradientValueSource source);

    qreal gradientValueMax() const;
    void setGradientValueMax(qreal value);

    GradientFillBaseline baseline() const;
    void setBaseline(GradientFillBaseline baseline);

    qreal baselineValue() const;
    void setBaselineValue(qreal value);

    qreal opacity() const;
    void setOpacity(qreal value);

    GradientFillPayload payload() const;

signals:
    void directionChanged();
    void gradientChanged();
    void gradientValueMinSourceChanged();
    void gradientValueMinChanged();
    void gradientValueMaxSourceChanged();
    void gradientValueMaxChanged();
    void baselineChanged();
    void baselineValueChanged();
    void opacityChanged();

private:
    void reconnectGradientSignals();
    void disconnectGradientSignals();
    void onGradientObjectChanged();

    GradientDirection direction_{GradientDirection::Horizontal};
    QObject* gradient_{nullptr};
    QVector<QMetaObject::Connection> gradientConnections_;
    GradientValueSource gradientValueMinSource_{GradientValueSource::DataRange};
    qreal gradientValueMin_{0.0};
    GradientValueSource gradientValueMaxSource_{GradientValueSource::DataRange};
    qreal gradientValueMax_{0.0};
    GradientFillBaseline baseline_{GradientFillBaseline::AxisMinimum};
    qreal baselineValue_{0.0};
    qreal opacity_{0.35};
};

} // namespace QAccelPlot
```


