

# File GradientUtils.hpp

[**File List**](files.md) **>** [**effects**](dir_2cb1ffa6338b0172fd78b92843e0e53d.md) **>** [**GradientUtils.hpp**](GradientUtils_8hpp.md)

[Go to the documentation of this file](GradientUtils_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/GradientColorTypes.hpp"

#include <QVariantList>

#include <vector>

class QObject;

namespace QAccelPlot {

std::vector<GradientStopData> readGradientStops(QObject* gradient);

std::vector<GradientStopData> readGradientStopList(const QVariantList& stopObjects);

} // namespace QAccelPlot
```


