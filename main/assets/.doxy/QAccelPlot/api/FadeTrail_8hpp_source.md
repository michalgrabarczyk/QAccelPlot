

# File FadeTrail.hpp

[**File List**](files.md) **>** [**effects**](dir_2cb1ffa6338b0172fd78b92843e0e53d.md) **>** [**FadeTrail.hpp**](FadeTrail_8hpp.md)

[Go to the documentation of this file](FadeTrail_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/SeriesEffect.hpp"

#include <QEasingCurve>

namespace QAccelPlot {

namespace Internal {
struct ItemTrail;
}

class FadeTrail : public SeriesEffect {
    Q_OBJECT
    QML_NAMED_ELEMENT(FadeTrail)

    
    Q_PROPERTY(qreal length READ length WRITE setLength NOTIFY lengthChanged)
    Q_PROPERTY(qreal minOpacity READ minOpacity WRITE setMinOpacity NOTIFY minOpacityChanged)
    Q_PROPERTY(QEasingCurve easing READ easing WRITE setEasing NOTIFY easingChanged)

public:
    explicit FadeTrail(QObject* parent = nullptr);

    qreal length() const;
    void setLength(qreal length);

    qreal minOpacity() const;
    void setMinOpacity(qreal opacity);

    QEasingCurve easing() const;
    void setEasing(const QEasingCurve& easing);

    Internal::ItemTrail trail(int itemCount) const;

signals:
    void lengthChanged();
    void minOpacityChanged();
    void easingChanged();

private:
    qreal length_{0.5};
    qreal minOpacity_{0.0};
    QEasingCurve easing_{QEasingCurve::InQuad};
};

} // namespace QAccelPlot
```


