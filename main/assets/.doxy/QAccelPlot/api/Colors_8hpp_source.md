

# File Colors.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**theme**](dir_dc76dfbefc4bd2a9d0a8a59875947d7b.md) **>** [**Colors.hpp**](Colors_8hpp.md)

[Go to the documentation of this file](Colors_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/theme/ColorPalette.hpp"

#include <QObject>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
// QML_SINGLETON needs QQmlPrivate declared, which qqmlregistration.h alone does not pull in
// on Qt versions before the standalone QtQmlIntegration module (introduced in Qt 6.5).
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlregistration.h>
#endif

namespace QAccelPlot {

class Colors : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Colors)
    QML_SINGLETON
    Q_PROPERTY(ColorPalette* dark READ dark CONSTANT)
    Q_PROPERTY(ColorPalette* light READ light CONSTANT)

public:
    explicit Colors(QObject* parent = nullptr);

    ColorPalette* dark() const;
    ColorPalette* light() const;
};

} // namespace QAccelPlot
```


