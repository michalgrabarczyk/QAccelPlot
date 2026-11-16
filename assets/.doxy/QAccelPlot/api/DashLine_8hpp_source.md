

# File DashLine.hpp

[**File List**](files.md) **>** [**linestyles**](dir_45c5c52b7ff7182bdf01ac21f48c7474.md) **>** [**DashLine.hpp**](DashLine_8hpp.md)

[Go to the documentation of this file](DashLine_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/linestyles/LineStyle.hpp"

#include <QList>

namespace QAccelPlot {

class DashLine : public LineStyle {
    Q_OBJECT
    QML_NAMED_ELEMENT(DashLine)

    
    Q_PROPERTY(QList<qreal> pattern READ pattern WRITE setPattern NOTIFY patternChanged)

public:
    explicit DashLine(QObject* parent = nullptr);

    QList<qreal> pattern() const;
    void setPattern(const QList<qreal>& pattern);

    DashParameters dashParameters() const override;

signals:
    void patternChanged();

private:
    QList<qreal> pattern_;
};

} // namespace QAccelPlot
```


