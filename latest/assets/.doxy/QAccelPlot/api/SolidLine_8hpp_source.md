

# File SolidLine.hpp

[**File List**](files.md) **>** [**linestyles**](dir_45c5c52b7ff7182bdf01ac21f48c7474.md) **>** [**SolidLine.hpp**](SolidLine_8hpp.md)

[Go to the documentation of this file](SolidLine_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/linestyles/LineStyle.hpp"

namespace QAccelPlot {

class SolidLine : public LineStyle {
    Q_OBJECT
    QML_NAMED_ELEMENT(SolidLine)

public:
    explicit SolidLine(QObject* parent = nullptr);
};

} // namespace QAccelPlot
```


