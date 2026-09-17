

# File LogTickLabelFormatter.hpp

[**File List**](files.md) **>** [**formatters**](dir_4eeeb48ede2263d8d479500d459323d3.md) **>** [**LogTickLabelFormatter.hpp**](LogTickLabelFormatter_8hpp.md)

[Go to the documentation of this file](LogTickLabelFormatter_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/formatters/TickLabelFormatter.hpp"

#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

class LogTickLabelFormatter : public TickLabelFormatter {
    Q_OBJECT
    QML_NAMED_ELEMENT(LogTickLabelFormatter)

public:
    explicit LogTickLabelFormatter(QObject* parent = nullptr);

protected:
    QString doFormat(qreal value, qreal tickStep) const override;
};

} // namespace QAccelPlot
```


