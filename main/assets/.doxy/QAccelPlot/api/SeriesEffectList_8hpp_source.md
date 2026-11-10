

# File SeriesEffectList.hpp

[**File List**](files.md) **>** [**internal**](dir_70e6e0d61970c92b37b608a046280901.md) **>** [**SeriesEffectList.hpp**](SeriesEffectList_8hpp.md)

[Go to the documentation of this file](SeriesEffectList_8hpp.md)


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
#include "QAccelPlot/internal/GlowStyle.hpp"
#include "QAccelPlot/internal/ItemFade.hpp"

#include <QList>
#include <QObject>
#include <QPointer>
#include <QQmlListProperty>

#include <functional>

QT_FORWARD_DECLARE_CLASS(QQuickItem)
QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot::Internal {

class SeriesEffectList : public QObject {
    Q_OBJECT

public:
    using Supports = std::function<bool(const SeriesEffect& effect)>;

    SeriesEffectList(QQuickItem* series, Supports supports);

    QQmlListProperty<SeriesEffect> property();
    const QList<SeriesEffect*>& effects() const;

    template <typename Effect> const Effect* first() const
    {
        for (auto* effect : effects_) {
            if (auto* typed = qobject_cast<const Effect*>(effect); typed && typed->enabled()) {
                return typed;
            }
        }
        return nullptr;
    }

    GlowStyle glowStyle(const QColor& seriesColor) const;
    ItemTrail itemTrail(int itemCount) const;

    void classBegin();
    void componentComplete();
    void connectWindow(QQuickWindow* window);

private:
    static void append(QQmlListProperty<SeriesEffect>* list, SeriesEffect* effect);
    static qsizetype count(QQmlListProperty<SeriesEffect>* list);
    static SeriesEffect* at(QQmlListProperty<SeriesEffect>* list, qsizetype index);
    static void clear(QQmlListProperty<SeriesEffect>* list);

    void add(SeriesEffect* effect);
    void warnIfIgnored(const SeriesEffect* effect) const;
    void refresh();

    QQuickItem* series_;
    Supports supports_;
    QList<SeriesEffect*> effects_;
    QPointer<QQuickWindow> window_;
    bool loading_{false};
};

} // namespace QAccelPlot::Internal
```


