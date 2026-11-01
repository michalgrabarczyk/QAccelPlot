

# File DataTextureUpload.hpp

[**File List**](files.md) **>** [**internal**](dir_3c3be61dbf90c69b9ad6cd32d23f3e24.md) **>** [**DataTextureUpload.hpp**](DataTextureUpload_8hpp.md)

[Go to the documentation of this file](DataTextureUpload_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QtCore/qglobal.h>

#include <memory>

QT_FORWARD_DECLARE_CLASS(QImage)
QT_FORWARD_DECLARE_CLASS(QQuickWindow)
QT_FORWARD_DECLARE_CLASS(QSGTexture)

namespace QAccelPlot::Internal {

void uploadDataTexture(std::unique_ptr<QSGTexture>& texture, QQuickWindow* window, const QImage& image);

} // namespace QAccelPlot::Internal
```


