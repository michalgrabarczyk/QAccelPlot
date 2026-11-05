

# File TextLabel.hpp

[**File List**](files.md) **>** [**axis**](dir_4047c0a16b95170c37806a99233d1784.md) **>** [**TextLabel.hpp**](TextLabel_8hpp.md)

[Go to the documentation of this file](TextLabel_8hpp.md)


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
#include <QFont>
#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

class TextLabel : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal padding READ padding WRITE setPadding NOTIFY paddingChanged)

public:
    explicit TextLabel(const QColor& color = {}, qreal padding = 0.0, QObject* parent = nullptr);

    QString text() const;
    void setText(const QString& text);

    QFont font() const;
    void setFont(const QFont& font);

    QColor color() const;
    void setColor(const QColor& color);

    qreal padding() const;
    void setPadding(qreal padding);

signals:
    void textChanged();
    void fontChanged();
    void colorChanged();
    void paddingChanged();

private:
    QString text_;
    QFont font_;
    QColor color_;
    qreal padding_;
};

} // namespace QAccelPlot
```


