

# File DrawTransition.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**transitions**](dir_2c4a66235e9a5453b987bd770b54fb17.md) **>** [**DrawTransition.hpp**](DrawTransition_8hpp.md)

[Go to the documentation of this file](DrawTransition_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "transitions/DataTransition.hpp"

namespace QAccelPlot {

class DrawTransition : public DataTransition {
    Q_OBJECT
    QML_NAMED_ELEMENT(DrawTransition)

public:
    explicit DrawTransition(QObject* parent = nullptr);

protected:
    void interpolate(double easedProgress, const std::vector<double>& fromData, int fromPointCount, const std::vector<double>& toData, int toPointCount,
        std::vector<double>& outData, int& outPointCount) override;
};

} // namespace QAccelPlot
```


