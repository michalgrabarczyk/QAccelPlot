

# File PlotMouseEvent.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**PlotMouseEvent.hpp**](PlotMouseEvent_8hpp.md)

[Go to the documentation of this file](PlotMouseEvent_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

class QSinglePointEvent;

namespace QAccelPlot {

class QAccelPlot;

class PlotMouseEvent : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(PlotMouseEvent)
    QML_UNCREATABLE("PlotMouseEvent can only be created by QAccelPlot.")

    
    Q_PROPERTY(Qt::MouseButton button READ button CONSTANT)
    Q_PROPERTY(Qt::MouseButtons buttons READ buttons CONSTANT)
    Q_PROPERTY(qreal x READ x CONSTANT)
    Q_PROPERTY(qreal y READ y CONSTANT)
    Q_PROPERTY(Qt::KeyboardModifiers modifiers READ modifiers CONSTANT)
    Q_PROPERTY(bool accepted READ isAccepted WRITE setAccepted)

public:
    explicit PlotMouseEvent(QObject* parent = nullptr);

    Qt::MouseButton button() const;
    Qt::MouseButtons buttons() const;
    qreal x() const;
    qreal y() const;
    Qt::KeyboardModifiers modifiers() const;
    bool isAccepted() const;
    Q_INVOKABLE void accept();
    Q_INVOKABLE void ignore();
    void setAccepted(bool accepted);

private:
    friend class QAccelPlot;

    void reset(const QSinglePointEvent& event);

    Qt::MouseButton button_{Qt::NoButton};
    Qt::MouseButtons buttons_{Qt::NoButton};
    qreal x_{0.0};
    qreal y_{0.0};
    Qt::KeyboardModifiers modifiers_{Qt::NoModifier};
    bool accepted_{false};
};

} // namespace QAccelPlot
```


