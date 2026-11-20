

# File SeriesSupport.hpp

[**File List**](files.md) **>** [**internal**](dir_70e6e0d61970c92b37b608a046280901.md) **>** [**SeriesSupport.hpp**](SeriesSupport_8hpp.md)

[Go to the documentation of this file](SeriesSupport_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/transitions/internal/TransitionFrame.hpp"

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot {
class Axis;
}

namespace QAccelPlot::Internal {

bool hoverEnabled();

bool supportsCustomShaderRendering(const QQuickWindow* window);

int maxTextureSize(QQuickWindow* window);

int recordCapacity(QQuickWindow* window, int floatsPerRecord, int vertexBytesPerRecord);

qreal clampAntialiasingFeather(qreal feather);

bool assignNonNegative(qreal& member, qreal requested);

GrowViewport growViewport(const Axis* valueAxis);

double growBaseline(double baseline, const Axis* valueAxis);

} // namespace QAccelPlot::Internal
```


