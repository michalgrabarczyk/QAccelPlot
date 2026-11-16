

# File OpacityAnimations.hpp

[**File List**](files.md) **>** [**internal**](dir_698dae9af7fc703eaceb77bfe4886cff.md) **>** [**OpacityAnimations.hpp**](OpacityAnimations_8hpp.md)

[Go to the documentation of this file](OpacityAnimations_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/transitions/internal/OpacityTimeline.hpp"

#include <QList>
#include <QStringList>

QT_FORWARD_DECLARE_CLASS(QObject)

namespace QAccelPlot::Internal {

OpacityTimeline readOpacityAnimations(const QList<QObject*>& animations, QStringList* problems = nullptr);

} // namespace QAccelPlot::Internal
```


