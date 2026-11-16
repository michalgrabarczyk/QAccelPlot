

# File StepPath.hpp

[**File List**](files.md) **>** [**internal**](dir_70e6e0d61970c92b37b608a046280901.md) **>** [**StepPath.hpp**](StepPath_8hpp.md)

[Go to the documentation of this file](StepPath_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/renderers/CurveRendererParams.hpp"
#include "QAccelPlot/series/StepMode.hpp"

#include <QPointF>

#include <optional>

namespace QAccelPlot::Internal {

[[nodiscard]] int stepCornerCount(StepMode mode) noexcept;

[[nodiscard]] std::optional<QPointF> stepCorner(const CurveDataView& data, int from, int to, int corner, StepMode mode, bool logScaleX, bool logScaleY);

[[nodiscard]] std::optional<QPointF> linePathPoint(
    const CurveDataView& data, int pointCount, int index, int corner, StepMode mode, bool logScaleX, bool logScaleY);

} // namespace QAccelPlot::Internal
```


