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

#include <QList>
#include <QObject>
#include <QPointer>
#include <QQmlListProperty>

#include <functional>

QT_FORWARD_DECLARE_CLASS(QQuickItem)
QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot::Internal {

/// \brief The \c effects list of one series.
///
/// Redraws the series when an effect changes, refreshes the effects once per frame, and warns
/// about effects the series ignores: unsupported ones and repeated types. Warnings wait for the
/// series' component to complete, so they can name it by its \c objectName.
class SeriesEffectList : public QObject {
    Q_OBJECT

public:
    /// \brief Returns whether the series draws \a effect.
    using Supports = std::function<bool(const SeriesEffect& effect)>;

    /// \brief Constructs the list of \a series, which must outlive it; \a supports tells which effects it draws.
    SeriesEffectList(QQuickItem* series, Supports supports);

    /// \brief Returns the QML list property over the effects.
    QQmlListProperty<SeriesEffect> property();
    /// \brief Returns the effects in list order, including ignored ones.
    const QList<SeriesEffect*>& effects() const;

    /// \brief Returns the first enabled effect of type \a Effect, or \c nullptr.
    template <typename Effect> const Effect* first() const
    {
        for (auto* effect : effects_) {
            if (auto* typed = qobject_cast<const Effect*>(effect); typed && typed->enabled()) {
                return typed;
            }
        }
        return nullptr;
    }

    /// \brief Returns the halo of the first enabled \c Glow, in \a seriesColor unless it has its own color; none without a \c Glow.
    GlowStyle glowStyle(const QColor& seriesColor) const;

    /// \brief Holds back warnings while the series' component loads.
    void classBegin();
    /// \brief Reports the effects added while the series' component loaded.
    void componentComplete();
    /// \brief Refreshes the effects once per frame of \a window.
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
