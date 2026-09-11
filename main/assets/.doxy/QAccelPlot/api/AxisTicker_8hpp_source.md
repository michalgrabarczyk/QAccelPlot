

# File AxisTicker.hpp

[**File List**](files.md) **>** [**axis**](dir_f07047c7c39e03c70b8bf3fe272880c9.md) **>** [**AxisTicker.hpp**](AxisTicker_8hpp.md)

[Go to the documentation of this file](AxisTicker_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "formatters/TickLabelFormatter.hpp"

#include <QColor>
#include <QFont>
#include <QObject>
#include <QPointer>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

class AxisTicker : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(QColor tickColor READ tickColor WRITE setTickColor NOTIFY tickColorChanged)
    Q_PROPERTY(QColor tickLabelColor READ tickLabelColor WRITE setTickLabelColor NOTIFY tickLabelColorChanged)
    Q_PROPERTY(int tickCount READ tickCount WRITE setTickCount NOTIFY tickCountChanged)
    Q_PROPERTY(int subtickCount READ subtickCount WRITE setSubtickCount NOTIFY subtickCountChanged)
    Q_PROPERTY(qreal tickLength READ tickLength WRITE setTickLength NOTIFY tickLengthChanged)
    Q_PROPERTY(qreal subtickLength READ subtickLength WRITE setSubtickLength NOTIFY subtickLengthChanged)
    Q_PROPERTY(qreal tickLengthIn READ tickLengthIn WRITE setTickLengthIn NOTIFY tickLengthInChanged)
    Q_PROPERTY(qreal tickLengthOut READ tickLengthOut WRITE setTickLengthOut NOTIFY tickLengthOutChanged)
    Q_PROPERTY(qreal subtickLengthIn READ subtickLengthIn WRITE setSubtickLengthIn NOTIFY subtickLengthInChanged)
    Q_PROPERTY(qreal subtickLengthOut READ subtickLengthOut WRITE setSubtickLengthOut NOTIFY subtickLengthOutChanged)
    Q_PROPERTY(QColor subtickColor READ subtickColor WRITE setSubtickColor NOTIFY subtickColorChanged)
    Q_PROPERTY(qreal tickWidth READ tickWidth WRITE setTickWidth NOTIFY tickWidthChanged)
    Q_PROPERTY(qreal subtickWidth READ subtickWidth WRITE setSubtickWidth NOTIFY subtickWidthChanged)
    Q_PROPERTY(qreal tickLabelPadding READ tickLabelPadding WRITE setTickLabelPadding NOTIFY tickLabelPaddingChanged)
    Q_PROPERTY(qreal tickLabelRotation READ tickLabelRotation WRITE setTickLabelRotation NOTIFY tickLabelRotationChanged)
    Q_PROPERTY(QFont tickLabelFont READ tickLabelFont WRITE setTickLabelFont NOTIFY tickLabelFontChanged)
    Q_PROPERTY(TickLabelFormatter* tickLabelFormatter READ tickLabelFormatter WRITE setTickLabelFormatter NOTIFY tickLabelFormatterChanged)

public:
    explicit AxisTicker(QObject* parent = nullptr);

    QColor tickColor() const;
    void setTickColor(const QColor& c);

    QColor tickLabelColor() const;
    void setTickLabelColor(const QColor& c);

    int tickCount() const;
    void setTickCount(int count);

    int subtickCount() const;
    void setSubtickCount(int count);

    qreal tickLength() const;
    void setTickLength(qreal length);

    qreal subtickLength() const;
    void setSubtickLength(qreal length);

    qreal tickLengthIn() const;
    void setTickLengthIn(qreal length);

    qreal tickLengthOut() const;
    void setTickLengthOut(qreal length);

    qreal subtickLengthIn() const;
    void setSubtickLengthIn(qreal length);

    qreal subtickLengthOut() const;
    void setSubtickLengthOut(qreal length);

    QColor subtickColor() const;
    void setSubtickColor(const QColor& c);

    qreal tickWidth() const;
    void setTickWidth(qreal width);

    qreal subtickWidth() const;
    void setSubtickWidth(qreal width);

    qreal tickLabelPadding() const;
    void setTickLabelPadding(qreal padding);

    qreal tickLabelRotation() const;
    void setTickLabelRotation(qreal rotation);

    QFont tickLabelFont() const;
    void setTickLabelFont(const QFont& f);

    TickLabelFormatter* tickLabelFormatter() const;
    void setTickLabelFormatter(TickLabelFormatter* formatter);

signals:
    void tickColorChanged();
    void tickLabelColorChanged();
    void tickCountChanged();
    void subtickCountChanged();
    void tickLengthChanged();
    void subtickLengthChanged();
    void tickLengthInChanged();
    void tickLengthOutChanged();
    void subtickLengthInChanged();
    void subtickLengthOutChanged();
    void subtickColorChanged();
    void tickWidthChanged();
    void subtickWidthChanged();
    void tickLabelPaddingChanged();
    void tickLabelRotationChanged();
    void tickLabelFontChanged();
    void tickLabelFormatterChanged();
    void tickLabelFormatChanged();

private:
    QColor tickColor_{QColor(Qt::black)};
    QColor tickLabelColor_;
    int tickCount_{5};
    int subtickCount_{10};
    qreal tickLengthIn_{8.0};
    qreal tickLengthOut_{8.0};
    qreal subtickLengthIn_{4.0};
    qreal subtickLengthOut_{4.0};
    QColor subtickColor_{QColor(Qt::darkGray)};
    qreal tickWidth_{2.0};
    qreal subtickWidth_{1.0};
    qreal tickLabelPadding_{5.0};
    qreal tickLabelRotation_{0.0};
    QFont tickLabelFont_;
    TickLabelFormatter* defaultTickLabelFormatter_{nullptr};
    QPointer<TickLabelFormatter> customTickLabelFormatter_;
    QMetaObject::Connection customFormatterDestroyedConnection_;
};

} // namespace QAccelPlot
```


