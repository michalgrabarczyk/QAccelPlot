

# File AxisLabel.hpp

[**File List**](files.md) **>** [**axis**](dir_4047c0a16b95170c37806a99233d1784.md) **>** [**AxisLabel.hpp**](AxisLabel_8hpp.md)

[Go to the documentation of this file](AxisLabel_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/axis/TextLabel.hpp"
#include "QAccelPlot/internal/ResettableValue.hpp"

#include <QPointF>
#include <QPointer>
#include <QQmlComponent>

namespace QAccelPlot {

class AxisLabel : public TextLabel {
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(Axis::LabelAlignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation RESET resetRotation NOTIFY rotationChanged)
    Q_PROPERTY(QPointF offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(QQmlComponent* delegate READ delegate WRITE setDelegate NOTIFY delegateChanged)

public:
    explicit AxisLabel(QObject* parent = nullptr);

    Axis::LabelAlignment alignment() const;
    void setAlignment(Axis::LabelAlignment alignment);

    qreal rotation() const;
    void setRotation(qreal degrees);
    void resetRotation();
    void setDefaultRotation(qreal degrees);

    QPointF offset() const;
    void setOffset(const QPointF& offset);

    QQmlComponent* delegate() const;
    void setDelegate(QQmlComponent* delegate);

signals:
    void alignmentChanged();
    void rotationChanged();
    void offsetChanged();
    void delegateChanged();

private:
    Axis::LabelAlignment alignment_{Axis::Center};
    Internal::ResettableValue<qreal> rotation_{0.0};
    QPointF offset_;
    QPointer<QQmlComponent> delegate_;
};

} // namespace QAccelPlot
```


