

# File GrowTransition.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**transitions**](dir_33e4f9f956353311d613e58793c6ec62.md) **>** [**GrowTransition.hpp**](GrowTransition_8hpp.md)

[Go to the documentation of this file](GrowTransition_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/transitions/EnterExitTransition.hpp"

namespace QAccelPlot {

class GrowTransition : public EnterExitTransition {
    Q_OBJECT
    QML_NAMED_ELEMENT(GrowTransition)

    
    Q_PROPERTY(qreal baseline READ baseline WRITE setBaseline RESET resetBaseline NOTIFY baselineChanged)

public:
    explicit GrowTransition(QObject* parent = nullptr);

    qreal baseline() const;
    void setBaseline(qreal baseline);
    void resetBaseline();

signals:
    void baselineChanged();

private:
    qreal baseline_;
};

} // namespace QAccelPlot
```


