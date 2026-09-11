

# File TickLabelFormatter.hpp

[**File List**](files.md) **>** [**formatters**](dir_d8921b289ee138c4c21e0a56c5dc2c98.md) **>** [**TickLabelFormatter.hpp**](TickLabelFormatter_8hpp.md)

[Go to the documentation of this file](TickLabelFormatter_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QJSValue>
#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

class TickLabelFormatter : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(QJSValue tickLabel READ tickLabel WRITE setTickLabel NOTIFY tickLabelChanged)

public:
    explicit TickLabelFormatter(QObject* parent = nullptr);

    QString format(qreal value, qreal tickStep) const;

    QJSValue tickLabel() const;
    void setTickLabel(const QJSValue& tickLabel);

signals:
    void tickLabelChanged();
    void formatChanged();

protected:
    virtual QString doFormat(qreal value, qreal tickStep) const = 0;

private:
    QJSValue tickLabel_;
};

} // namespace QAccelPlot
```


