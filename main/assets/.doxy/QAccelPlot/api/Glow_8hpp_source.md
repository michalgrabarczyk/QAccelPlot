

# File Glow.hpp

[**File List**](files.md) **>** [**effects**](dir_2cb1ffa6338b0172fd78b92843e0e53d.md) **>** [**Glow.hpp**](Glow_8hpp.md)

[Go to the documentation of this file](Glow_8hpp.md)


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

#include <QColor>

namespace QAccelPlot {

namespace Internal {
struct GlowStyle;
}

class Glow : public SeriesEffect {
    Q_OBJECT
    QML_NAMED_ELEMENT(Glow)

    
    Q_PROPERTY(QColor color READ color WRITE setColor RESET resetColor NOTIFY colorChanged)
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(qreal intensity READ intensity WRITE setIntensity NOTIFY intensityChanged)

public:
    explicit Glow(QObject* parent = nullptr);

    QColor color() const;
    void setColor(const QColor& color);
    void resetColor();

    qreal radius() const;
    void setRadius(qreal radius);

    qreal intensity() const;
    void setIntensity(qreal intensity);

    Internal::GlowStyle style(const QColor& seriesColor) const;

signals:
    void colorChanged();
    void radiusChanged();
    void intensityChanged();

private:
    QColor color_;
    qreal radius_{8.0};
    qreal intensity_{0.5};
};

} // namespace QAccelPlot
```


