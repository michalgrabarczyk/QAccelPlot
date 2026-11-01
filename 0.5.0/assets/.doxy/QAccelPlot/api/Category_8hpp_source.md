

# File Category.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**Category.hpp**](Category_8hpp.md)

[Go to the documentation of this file](Category_8hpp.md)


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
#include <QString>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

namespace QAccelPlot {

class Category : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Category)

    
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit Category(QObject* parent = nullptr);
    Category(int value, QString name, const QColor& color, QObject* parent = nullptr);

    int value() const;
    void setValue(int value);

    QString name() const;
    void setName(const QString& name);

    QColor color() const;
    void setColor(const QColor& color);

signals:
    void valueChanged();
    void nameChanged();
    void colorChanged();

private:
    int value_{0};
    QString name_;
    QColor color_;
};

} // namespace QAccelPlot
```


