

# File BandEdges.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**BandEdges.hpp**](BandEdges_8hpp.md)

[Go to the documentation of this file](BandEdges_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/linestyles/LineStyle.hpp"

#include <QColor>
#include <QObject>
#include <QPointer>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

namespace QAccelPlot {

class BandEdges : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(LineStyle* lineStyle READ lineStyle WRITE setLineStyle NOTIFY lineStyleChanged)

public:
    explicit BandEdges(QObject* parent = nullptr);

    qreal width() const;
    void setWidth(qreal width);

    QColor color() const;
    void setColor(const QColor& color);

    LineStyle* lineStyle() const;
    void setLineStyle(LineStyle* style);

signals:
    void widthChanged();
    void colorChanged();
    void lineStyleChanged();

private:
    void onLineStyleDestroyed();

    qreal width_{0.0};
    QColor color_;
    QPointer<LineStyle> lineStyle_;
};

} // namespace QAccelPlot
```


