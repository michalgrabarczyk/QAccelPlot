

# File SeriesSupport.hpp

[**File List**](files.md) **>** [**internal**](dir_70e6e0d61970c92b37b608a046280901.md) **>** [**SeriesSupport.hpp**](SeriesSupport_8hpp.md)

[Go to the documentation of this file](SeriesSupport_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot::Internal {

bool hoverEnabled();

bool supportsCustomShaderRendering(const QQuickWindow* window);

int maxTextureSize(QQuickWindow* window);

} // namespace QAccelPlot::Internal
```


