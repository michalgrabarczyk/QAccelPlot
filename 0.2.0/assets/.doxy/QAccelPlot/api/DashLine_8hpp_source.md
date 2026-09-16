

# File DashLine.hpp

[**File List**](files.md) **>** [**linestyles**](dir_2a035f8043b68ba8becf10bf9401d5c5.md) **>** [**DashLine.hpp**](DashLine_8hpp.md)

[Go to the documentation of this file](DashLine_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "linestyles/LineStyle.hpp"

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


