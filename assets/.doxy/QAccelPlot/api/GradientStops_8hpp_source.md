

# File GradientStops.hpp

[**File List**](files.md) **>** [**effects**](dir_2cb1ffa6338b0172fd78b92843e0e53d.md) **>** [**internal**](dir_549760165a8bc07353c1d6f09e5e91eb.md) **>** [**GradientStops.hpp**](GradientStops_8hpp.md)

[Go to the documentation of this file](GradientStops_8hpp.md)


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
class Colormap;
} // namespace QAccelPlot

namespace QAccelPlot::Internal {

std::vector<GradientStopData> readGradientStopList(const QVariantList& stopObjects);

std::vector<GradientStopData> readEffectStops(const Colormap* colormap, QObject* gradient);

} // namespace QAccelPlot::Internal
```


