//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/internal/SeriesEffectList.hpp"

#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/effects/Glow.hpp"

#include <QQuickItem>
#include <QQuickWindow>

#include <algorithm>
#include <utility>

namespace QAccelPlot::Internal {

namespace {

/// \brief Returns the class name of \a object without its namespace, as QML names the type.
QString typeName(const QObject& object)
{
    const auto name = QString::fromLatin1(object.metaObject()->className());
    return name.mid(name.lastIndexOf(QLatin1Char(':')) + 1);
}

SeriesEffectList* listOf(QQmlListProperty<SeriesEffect>* list)
{
    return list ? static_cast<SeriesEffectList*>(list->data) : nullptr;
}

} // namespace

SeriesEffectList::SeriesEffectList(QQuickItem* series, Supports supports)
    : QObject(series)
    , series_(series)
    , supports_(std::move(supports))
{
}

QQmlListProperty<SeriesEffect> SeriesEffectList::property()
{
    return QQmlListProperty<SeriesEffect>(series_, this, &SeriesEffectList::append, &SeriesEffectList::count, &SeriesEffectList::at, &SeriesEffectList::clear);
}

const QList<SeriesEffect*>& SeriesEffectList::effects() const
{
    return effects_;
}

GlowStyle SeriesEffectList::glowStyle(const QColor& seriesColor) const
{
    const auto* glow = first<Glow>();
    return glow ? glow->style(seriesColor) : GlowStyle{};
}

void SeriesEffectList::classBegin()
{
    loading_ = true;
}

void SeriesEffectList::componentComplete()
{
    loading_ = false;
    for (const auto* effect : std::as_const(effects_)) {
        warnIfIgnored(effect);
    }
}

void SeriesEffectList::connectWindow(QQuickWindow* window)
{
    if (window_ == window) {
        return;
    }
    if (window_) {
        disconnect(window_, &QQuickWindow::afterAnimating, this, &SeriesEffectList::refresh);
    }
    window_ = window;
    if (window) {
        connect(window, &QQuickWindow::afterAnimating, this, &SeriesEffectList::refresh);
    }
}

void SeriesEffectList::append(QQmlListProperty<SeriesEffect>* list, SeriesEffect* effect)
{
    if (auto* self = listOf(list); self && effect) {
        self->add(effect);
    }
}

qsizetype SeriesEffectList::count(QQmlListProperty<SeriesEffect>* list)
{
    const auto* self = listOf(list);
    return self ? self->effects_.size() : 0;
}

SeriesEffect* SeriesEffectList::at(QQmlListProperty<SeriesEffect>* list, const qsizetype index)
{
    const auto* self = listOf(list);
    if (!self || index < 0 || index >= self->effects_.size()) {
        return nullptr;
    }
    return self->effects_.at(index);
}

void SeriesEffectList::clear(QQmlListProperty<SeriesEffect>* list)
{
    auto* self = listOf(list);
    if (!self) {
        return;
    }
    for (const auto* effect : std::as_const(self->effects_)) {
        disconnect(effect, nullptr, self, nullptr);
    }
    self->effects_.clear();
    self->series_->update();
}

void SeriesEffectList::add(SeriesEffect* effect)
{
    if (effects_.contains(effect)) {
        return;
    }
    if (!effect->parent()) {
        effect->setParent(series_);
    }
    effects_.append(effect);
    connect(effect, &SeriesEffect::effectChanged, series_, &QQuickItem::update);
    connect(effect, &QObject::destroyed, this, [this, effect] {
        effects_.removeAll(effect);
        series_->update();
    });
    // A series loading from QML reports its effects once its component completes.
    if (!loading_) {
        warnIfIgnored(effect);
    }
    series_->update();
}

void SeriesEffectList::warnIfIgnored(const SeriesEffect* effect) const
{
    const auto effectName = typeName(*effect);
    auto seriesName = typeName(*series_);
    if (!series_->objectName().isEmpty()) {
        seriesName += QStringLiteral(" \"%1\"").arg(series_->objectName());
    }
    if (!supports_(*effect)) {
        qCWarning(lcQAccelPlot).noquote() << QStringLiteral("%1 is not supported by %2; the effect is ignored").arg(effectName, seriesName);
        return;
    }
    const auto first
        = std::find_if(effects_.cbegin(), effects_.cend(), [effect](const SeriesEffect* other) { return other->metaObject() == effect->metaObject(); });
    if (*first != effect) {
        qCWarning(lcQAccelPlot).noquote()
            << QStringLiteral("%1 appears more than once in the effects of %2; only the first enabled one applies").arg(effectName, seriesName);
    }
}

void SeriesEffectList::refresh()
{
    // Loop by index rather than range-for: refresh() emits effectChanged(), and a handler that
    // removes an effect would invalidate the iterators.
    for (auto i = qsizetype{0}; i < effects_.size(); ++i) {
        effects_.at(i)->refresh();
    }
}

} // namespace QAccelPlot::Internal
