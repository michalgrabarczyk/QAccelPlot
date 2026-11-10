//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/transitions/DataTransition.hpp"

#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/transitions/internal/OpacityAnimations.hpp"

#include <QAbstractAnimation>

#include <algorithm>
#include <utility>

namespace QAccelPlot {

namespace {

class AnimationClock final : public QAbstractAnimation {
public:
    using QAbstractAnimation::QAbstractAnimation;

    int duration() const override
    {
        return -1;
    }

protected:
    void updateCurrentTime([[maybe_unused]] int currentTime) override
    {
    }
};

} // namespace

DataTransition::Run::~Run()
{
    end();
}

bool DataTransition::Run::active() const
{
    return transition_ != nullptr;
}

DataTransition* DataTransition::Run::transition() const
{
    return transition_;
}

void DataTransition::Run::end()
{
    if (auto* transition = std::exchange(transition_, nullptr)) {
        transition->removeRun(this);
    }
}

DataTransition::~DataTransition()
{
    for (auto* run : std::as_const(runs_)) {
        run->transition_ = nullptr;
    }
}

int DataTransition::duration() const
{
    return duration_;
}

void DataTransition::setDuration(const int duration)
{
    if (duration_ == duration) {
        return;
    }
    duration_ = duration;
    emit durationChanged();
}

QEasingCurve DataTransition::easing() const
{
    return easing_;
}

void DataTransition::setEasing(const QEasingCurve& easing)
{
    if (easing_ == easing) {
        return;
    }
    easing_ = easing;
    emit easingChanged();
}

bool DataTransition::enabled() const
{
    return enabled_;
}

void DataTransition::setEnabled(const bool enabled)
{
    if (enabled_ == enabled) {
        return;
    }
    enabled_ = enabled;
    emit enabledChanged();
}

bool DataTransition::running() const
{
    return running_;
}

qreal DataTransition::stagger() const
{
    return stagger_;
}

void DataTransition::setStagger(const qreal stagger)
{
    const auto clamped = std::clamp(stagger, 0.0, 1.0);
    if (stagger_ == clamped) {
        return;
    }
    stagger_ = clamped;
    emit staggerChanged();
}

QQmlListProperty<QObject> DataTransition::animations()
{
    return QQmlListProperty<QObject>(
        this, nullptr, &DataTransition::appendAnimation, &DataTransition::animationCount, &DataTransition::animationAt, &DataTransition::clearAnimations);
}

void DataTransition::start(Run& run)
{
    if (run.transition_ != this) {
        run.end();
        run.transition_ = this;
        runs_.append(&run);
    }
    setRunning(true);
    run.startTime_ = clock_->currentTime();
}

void DataTransition::cancel()
{
    for (auto* run : std::as_const(runs_)) {
        run->transition_ = nullptr;
    }
    runs_.clear();
    setRunning(false);
}

double DataTransition::progress(const Run& run) const
{
    if (run.transition_ != this || duration_ <= 0) {
        return 1.0;
    }
    const auto elapsed = clock_->currentTime() - run.startTime_;
    return std::clamp(static_cast<double>(elapsed) / duration_, 0.0, 1.0);
}

Internal::OpacityTimeline DataTransition::opacityTimeline() const
{
    return Internal::readOpacityAnimations(animations_);
}

void DataTransition::classBegin()
{
    loading_ = true;
}

void DataTransition::componentComplete()
{
    loading_ = false;
    warnAboutIgnored(animations_);
}

DataTransition::DataTransition(QObject* parent)
    : QObject(parent)
    , clock_(new AnimationClock(this))
{
}

void DataTransition::appendAnimation(QQmlListProperty<QObject>* list, QObject* animation)
{
    auto* self = static_cast<DataTransition*>(list->object);
    if (!animation || self->animations_.contains(animation)) {
        return;
    }
    if (!animation->parent()) {
        animation->setParent(self);
    }
    self->animations_.append(animation);
    connect(animation, &QObject::destroyed, self, [self, animation] { self->animations_.removeAll(animation); });
    // A transition loading from QML reports its animations once its component completes.
    if (!self->loading_) {
        self->warnAboutIgnored({animation});
    }
}

qsizetype DataTransition::animationCount(QQmlListProperty<QObject>* list)
{
    return static_cast<DataTransition*>(list->object)->animations_.size();
}

QObject* DataTransition::animationAt(QQmlListProperty<QObject>* list, const qsizetype index)
{
    return static_cast<DataTransition*>(list->object)->animations_.at(index);
}

void DataTransition::clearAnimations(QQmlListProperty<QObject>* list)
{
    auto* self = static_cast<DataTransition*>(list->object);
    for (const auto* animation : std::as_const(self->animations_)) {
        disconnect(animation, nullptr, self, nullptr);
    }
    self->animations_.clear();
}

void DataTransition::removeRun(Run* run)
{
    runs_.removeOne(run);
    setRunning(!runs_.isEmpty());
}

void DataTransition::setRunning(const bool running)
{
    if (running_ == running) {
        return;
    }
    running_ = running;
    // Runs measure time from their start, so the clock restarts from zero for each running phase.
    if (running) {
        clock_->start();
    } else {
        clock_->stop();
    }
    emit runningChanged();
}

void DataTransition::warnAboutIgnored(const QList<QObject*>& animations) const
{
    auto problems = QStringList{};
    Internal::readOpacityAnimations(animations, &problems);
    const auto name = QString::fromLatin1(metaObject()->className()).section(QLatin1Char(':'), -1);
    for (const auto& problem : std::as_const(problems)) {
        qCWarning(lcQAccelPlot).noquote() << QStringLiteral("%1: %2").arg(name, problem);
    }
}

} // namespace QAccelPlot
