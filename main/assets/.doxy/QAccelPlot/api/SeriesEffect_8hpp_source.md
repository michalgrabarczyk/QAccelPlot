

# File SeriesEffect.hpp

[**File List**](files.md) **>** [**effects**](dir_2cb1ffa6338b0172fd78b92843e0e53d.md) **>** [**SeriesEffect.hpp**](SeriesEffect_8hpp.md)

[Go to the documentation of this file](SeriesEffect_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

class SeriesEffect : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit SeriesEffect(QObject* parent = nullptr);

    bool enabled() const;
    void setEnabled(bool enabled);

    virtual void refresh();

signals:
    void enabledChanged();
    void effectChanged();

private:
    bool enabled_{true};
};

} // namespace QAccelPlot
```


