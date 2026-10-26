//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "MonitorController.hpp"

#include "LevelHistogram.hpp"

#include <QAccelPlot/series/BandSeries.hpp>
#include <QAccelPlot/series/BarSeries.hpp>
#include <QAccelPlot/series/HeatmapSeries.hpp>
#include <QAccelPlot/series/LineCurve.hpp>

#include <QDebug>
#include <QList>
#include <QMetaObject>
#include <QQuickItem>
#include <QQuickWindow>

#include <algorithm>
#include <utility>

namespace QAccelPlotExample {

namespace {

// Finds the item named name in the item tree below root. Repeater delegates have no QObject
// parent, so QObject::findChild() would miss them.
template <typename Item> Item* findItem(QQuickItem* root, const QString& name)
{
    auto pending = QList<QQuickItem*>{root};
    while (!pending.isEmpty()) {
        auto* candidate = pending.takeLast();
        if (candidate->objectName() == name) {
            if (auto* item = qobject_cast<Item*>(candidate)) {
                return item;
            }
        }
        pending.append(candidate->childItems());
    }
    return nullptr;
}

template <typename Item> bool findTarget(QQuickItem* root, const QString& name, Item*& target)
{
    target = findItem<Item>(root, name);
    if (!target) {
        qCritical().noquote() << QStringLiteral("The dashboard has no series named '%1'.").arg(name);
    }
    return target != nullptr;
}

} // namespace

MonitorController::MonitorController(const ClockMode clockMode, const bool demo, QObject* parent)
    : QObject(parent)
    , clockMode_(clockMode)
    , demo_(demo)
{
    framesPerSecondTimer_.setInterval(1000);
    connect(&framesPerSecondTimer_, &QTimer::timeout, this, &MonitorController::updateFramesPerSecond);
}

MonitorController::~MonitorController()
{
    stop();
}

bool MonitorController::attach(QQuickWindow* window)
{
    auto targets = AcquisitionTargets{};
    auto* root = window->contentItem();
    auto found = true;
    for (auto channel = 0; channel < VibrationModel::kChannelCount; ++channel) {
        found = findTarget(root, QStringLiteral("waveform%1").arg(channel), targets.waveforms.at(static_cast<std::size_t>(channel))) && found;
    }
    found = findTarget(root, QStringLiteral("spectrumCurve"), targets.spectrum) && found;
    found = findTarget(root, QStringLiteral("spectrogramSeries"), targets.spectrogram) && found;
    found = findTarget(root, QStringLiteral("rmsSeries"), targets.rms) && found;
    found = findTarget(root, QStringLiteral("alarmEnvelope"), targets.alarmEnvelope) && found;
    auto* levelBars = static_cast<QAccelPlot::BarSeries*>(nullptr);
    found = findTarget(root, QStringLiteral("levelHistogram"), levelBars) && found;
    if (!found) {
        return false;
    }
    // Owned by the controller; it counts on the UI thread, where the spectrogram's view changes.
    new LevelHistogram(targets.spectrogram, levelBars, this);

    worker_ = std::make_unique<AcquisitionWorker>(clock(), targets,
        [this](const double dataTime) { QMetaObject::invokeMethod(this, [this, dataTime]() { onFrameDelivered(dataTime); }, Qt::QueuedConnection); });
    worker_->setSelectedChannel(selectedChannel_);
    worker_->start();

    if (clockMode_ == ClockMode::RealTime) {
        connect(window, &QQuickWindow::afterAnimating, this, [this]() { worker_->requestFrame(); });
        framesPerSecondTimer_.start();
    }
    return true;
}

void MonitorController::stop()
{
    if (worker_) {
        worker_->stop();
    }
    framesPerSecondTimer_.stop();
}

void MonitorController::showFrameAt(const double seconds, std::function<void()> ready)
{
    Q_ASSERT(clockMode_ == ClockMode::Stepped);
    steppedClock_.setTime(seconds);
    frameReady_ = std::move(ready);
    worker_->requestFrame();
}

double MonitorController::acceleration(const int channel, const double plotY) const
{
    return AcquisitionWorker::acceleration(channel, plotY);
}

double MonitorController::channelOffset(const int channel) const
{
    return AcquisitionWorker::channelOffset(channel);
}

QStringList MonitorController::channelNames() const
{
    return VibrationModel::channelNames();
}

int MonitorController::sampleRate() const
{
    return VibrationModel::kSampleRate;
}

int MonitorController::windowSeconds() const
{
    return AcquisitionWorker::kWindowSeconds;
}

int MonitorController::pointCount() const
{
    return AcquisitionWorker::kWindowSamples * VibrationModel::kChannelCount;
}

int MonitorController::faultChannel() const
{
    return VibrationModel::kFaultChannel;
}

double MonitorController::bpfoHz() const
{
    return VibrationModel::kBpfoHz;
}

double MonitorController::maxFrequencyHz() const
{
    return AcquisitionWorker::kMaxFrequencyHz;
}

double MonitorController::spectrogramBinHz() const
{
    return static_cast<double>(VibrationModel::kSampleRate) / AcquisitionWorker::kSpectrogramFftSize;
}

bool MonitorController::recording() const
{
    return clockMode_ == ClockMode::Stepped;
}

bool MonitorController::demo() const
{
    return demo_;
}

int MonitorController::selectedChannel() const
{
    return selectedChannel_;
}

void MonitorController::setSelectedChannel(const int channel)
{
    const auto clamped = std::clamp(channel, 0, VibrationModel::kChannelCount - 1);
    if (clamped == selectedChannel_) {
        return;
    }
    selectedChannel_ = clamped;
    if (worker_) {
        worker_->setSelectedChannel(clamped);
        // Shows the new channel's spectrum at once, also while the acquisition is on hold.
        worker_->requestFrame();
    }
    emit selectedChannelChanged();
}

bool MonitorController::paused() const
{
    return paused_;
}

void MonitorController::setPaused(const bool paused)
{
    if (paused == paused_ || clockMode_ != ClockMode::RealTime) {
        return;
    }
    paused_ = paused;
    realTimeClock_.setPaused(paused);
    emit pausedChanged();
}

double MonitorController::dataTime() const
{
    return dataTime_;
}

int MonitorController::framesPerSecond() const
{
    return framesPerSecond_;
}

const AcquisitionClock& MonitorController::clock() const
{
    if (clockMode_ == ClockMode::RealTime) {
        return realTimeClock_;
    }
    return steppedClock_;
}

void MonitorController::onFrameDelivered(const double dataTime)
{
    // The data posted for this frame was queued before this call, so the series already hold it.
    if (dataTime != dataTime_) {
        dataTime_ = dataTime;
        ++framesThisSecond_;
        emit dataTimeChanged();
    }
    worker_->acknowledgeFrame();
    if (frameReady_) {
        std::exchange(frameReady_, {})();
    }
}

void MonitorController::updateFramesPerSecond()
{
    framesPerSecond_ = std::exchange(framesThisSecond_, 0);
    emit framesPerSecondChanged();
}

} // namespace QAccelPlotExample
