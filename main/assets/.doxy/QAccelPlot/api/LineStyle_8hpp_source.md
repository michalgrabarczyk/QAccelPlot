

# File LineStyle.hpp

[**File List**](files.md) **>** [**linestyles**](dir_2a035f8043b68ba8becf10bf9401d5c5.md) **>** [**LineStyle.hpp**](LineStyle_8hpp.md)

[Go to the documentation of this file](LineStyle_8hpp.md)


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

namespace QAccelPlot {

struct DashParameters {
    bool enabled{false}; 
    float period{0.0f};  
    float offset{0.0f};  
    int patternSize{0};  
    float pattern[8]{};  
};

class LineStyle : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(bool showLine READ showLine CONSTANT)

public:
    explicit LineStyle(QObject* parent = nullptr);

    virtual bool showLine() const;
    virtual DashParameters dashParameters() const;

signals:
    void styleChanged();
};

} // namespace QAccelPlot
```


