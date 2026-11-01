

# File Border.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**Border.hpp**](Border_8hpp.md)

[Go to the documentation of this file](Border_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>
#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

class Border : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)

public:
    explicit Border(const QColor& color, qreal width, QObject* parent = nullptr);

    QColor color() const;
    void setColor(const QColor& color);

    qreal width() const;
    void setWidth(qreal width);

signals:
    void colorChanged();
    void widthChanged();

private:
    QColor color_;
    qreal width_;
};

} // namespace QAccelPlot
```


