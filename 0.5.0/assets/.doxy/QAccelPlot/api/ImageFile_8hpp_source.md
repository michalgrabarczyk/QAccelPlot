

# File ImageFile.hpp

[**File List**](files.md) **>** [**export**](dir_165621f1d3f543afcb149fc22fafadb8.md) **>** [**internal**](dir_502a5035d8e883b896944d94c324e4a9.md) **>** [**ImageFile.hpp**](ImageFile_8hpp.md)

[Go to the documentation of this file](ImageFile_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QByteArray>
#include <QImage>
#include <QString>

namespace QAccelPlot::Internal {

QString localFilePath(const QString& pathOrUrl);

QByteArray imageFormatForPath(const QString& path);

void setImageResolution(QImage& image, qreal dpi);

QString writeImageFile(const QImage& image, const QString& path, const QByteArray& format, int quality);

} // namespace QAccelPlot::Internal
```


