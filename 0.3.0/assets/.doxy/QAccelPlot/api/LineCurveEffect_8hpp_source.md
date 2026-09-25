

# File LineCurveEffect.hpp

[**File List**](files.md) **>** [**effects**](dir_2cb1ffa6338b0172fd78b92843e0e53d.md) **>** [**LineCurveEffect.hpp**](LineCurveEffect_8hpp.md)

[Go to the documentation of this file](LineCurveEffect_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

class LineCurveEffect : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit LineCurveEffect(QObject* parent = nullptr);

    bool enabled() const;
    void setEnabled(bool enabled);

signals:
    void enabledChanged();
    void effectChanged();

private:
    bool enabled_{true};
};

} // namespace QAccelPlot
```


