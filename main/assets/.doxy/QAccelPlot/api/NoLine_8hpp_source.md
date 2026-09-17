

# File NoLine.hpp

[**File List**](files.md) **>** [**linestyles**](dir_45c5c52b7ff7182bdf01ac21f48c7474.md) **>** [**NoLine.hpp**](NoLine_8hpp.md)

[Go to the documentation of this file](NoLine_8hpp.md)


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

namespace QAccelPlot {

class NoLine : public LineStyle {
    Q_OBJECT
    QML_NAMED_ELEMENT(NoLine)

public:
    explicit NoLine(QObject* parent = nullptr);

    bool showLine() const override;
};

} // namespace QAccelPlot
```


