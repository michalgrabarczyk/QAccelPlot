//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "PulsarWorker.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace QAccelPlotExample {

namespace {

constexpr auto kXMaximum = 1000.0f;
constexpr auto kRidgePitch = 6.0f;
constexpr auto kAnimationIntervalSeconds = 6.0;
constexpr auto kTwoPi = 6.283185307179586476925286766559;

inline float gaussian(const float x, const float center, const float width)
{
    const auto d = (x - center) / width;
    return std::exp(-d * d);
}

} // namespace

PulsarWorker::PulsarWorker() = default;

PulsarWorker::~PulsarWorker()
{
    stop();
}

void PulsarWorker::start()
{
    if (running_.exchange(true, std::memory_order_acq_rel)) {
        return;
    }

    workerThread_ = std::thread([this]() { run(); });
}

void PulsarWorker::stop()
{
    if (!running_.exchange(false, std::memory_order_acq_rel)) {
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
        return;
    }

    conditionVariable_.notify_one();

    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}

void PulsarWorker::setElapsedSeconds(const double elapsedSeconds)
{
    elapsedSeconds_.store(elapsedSeconds, std::memory_order_relaxed);
}

bool PulsarWorker::tryConsume(PulsarBatch& batch)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!dataReady_) {
        return false;
    }

    batch = std::move(readyBatch_);
    dataReady_ = false;
    conditionVariable_.notify_one();
    return true;
}

PulsarBatch PulsarWorker::computeBatchForPhase(const double phase)
{
    auto batch = PulsarBatch{};
    generateBatch(batch, phase);
    return batch;
}

void PulsarWorker::run()
{
    auto workBatch = PulsarBatch{};

    while (running_.load(std::memory_order_acquire)) {
        const auto elapsed = elapsedSeconds_.load(std::memory_order_relaxed);
        const auto phase = elapsed / kAnimationIntervalSeconds;
        generateBatch(workBatch, phase);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            readyBatch_ = std::move(workBatch);
            dataReady_ = true;
        }

        std::unique_lock<std::mutex> lock(mutex_);
        conditionVariable_.wait(lock, [this]() { return !dataReady_ || !running_.load(std::memory_order_acquire); });
    }
}

void PulsarWorker::generateBatch(PulsarBatch& batch, const double phase)
{
    const auto u = std::fmod(std::fmod(phase, 1.0) + 1.0, 1.0);
    batch.ridgeCount = pulsarRidgeCount;
    batch.pointsPerRidge = pulsarPointsPerRidge;

    if (batch.ridges.size() != static_cast<std::size_t>(pulsarRidgeCount)) {
        batch.ridges.resize(pulsarRidgeCount);
        for (auto& ridge : batch.ridges) {
            ridge.resize(static_cast<std::size_t>(pulsarPointsPerRidge) * 2);
        }
    }

    const auto xStep = kXMaximum / static_cast<float>(pulsarPointsPerRidge - 1);

    for (auto ridgeIndex = int{0}; ridgeIndex < pulsarRidgeCount; ++ridgeIndex) {
        const auto v = static_cast<double>(ridgeIndex) / static_cast<double>(pulsarRidgeCount - 1);
        // Integer pixel baseline pitch, spread across the original plot depth.
        const auto yBase = 15.0f + static_cast<float>(ridgeIndex) * kRidgePitch;

        // Downward traveling pulse wave (3.0s per sweep = 2 sweeps per 6.0s loop)
        const auto phi1 = kTwoPi * (2.0 * u + 2.0 * v);
        const auto cosHalf1 = std::cos(phi1 * 0.5);
        const auto pulse1 = static_cast<float>(cosHalf1 * cosHalf1 * cosHalf1 * cosHalf1);
        const auto primaryPulse = 0.10f + 0.90f * pulse1;

        // Counter-propagating harmonic reflection
        const auto phi2 = kTwoPi * (2.0 * u - 1.0 * v + 0.25);
        const auto cosHalf2 = std::cos(phi2 * 0.5);
        const auto pulse2 = static_cast<float>(cosHalf2 * cosHalf2 * cosHalf2 * cosHalf2 * cosHalf2 * cosHalf2);

        // Sub-pulse centers that vary organically across depth
        const auto centerA = 500.0f - 46.0f * static_cast<float>(std::sin(kTwoPi * (v + 0.12)));
        const auto centerB = 500.0f + 42.0f * static_cast<float>(std::cos(kTwoPi * (v - 0.08)));
        const auto centerC = 500.0f + 92.0f * static_cast<float>(std::sin(kTwoPi * 2.0 * v));

        auto& ridgeBuffer = batch.ridges[static_cast<std::size_t>(ridgeIndex)];

        for (auto pointIndex = int{0}; pointIndex < pulsarPointsPerRidge; ++pointIndex) {
            const auto x = static_cast<float>(pointIndex) * xStep;

            // Gaussian corridor envelope around center
            const auto corridorDist = (x - 500.0f) / 185.0f;
            const auto corridorEnv = std::exp(-2.2f * corridorDist * corridorDist);

            // Sub-peaks
            const auto gA = gaussian(x, centerA, 28.0f);
            const auto gB = gaussian(x, centerB, 22.0f);
            const auto gC = gaussian(x, centerC, 35.0f);

            // Jagged radio scintillation micro-texture
            const auto normX = static_cast<double>(x) / static_cast<double>(kXMaximum);
            const auto r1 = std::sin(kTwoPi * (22.0 * normX - 8.0 * u + v * 3.0));
            const auto r2 = std::cos(kTwoPi * (44.0 * normX + 14.0 * u - v * 5.0));
            const auto r3 = std::sin(kTwoPi * (78.0 * normX - 18.0 * u));
            const auto microTexture = static_cast<float>(0.42 * r1 + 0.24 * r2 + 0.14 * r3);

            // Towering peak structure scaled in pixel units
            const auto peakStructure = primaryPulse * (24.0f * gA + 16.0f * gB) + pulse2 * 8.0f * gC;
            const auto textureMod = 4.5f * microTexture * (gA + gB + gC);
            const auto rawHeight = corridorEnv * (peakStructure + textureMod);
            const auto height = std::max(0.0f, rawHeight);

            const auto y = yBase + height;

            const auto offset = static_cast<std::size_t>(pointIndex) * 2;
            ridgeBuffer[offset] = x;
            ridgeBuffer[offset + 1] = y;
        }
    }

    // Continuous, analytical sub-pixel burst tracker
    const auto burstV = std::fmod(1.0 - 2.0 * u + 10.0, 1.0);
    const auto continuousRidge = burstV * static_cast<double>(pulsarRidgeCount - 1);
    const auto continuousYBase = 15.0 + continuousRidge * kRidgePitch;

    // Continuous X position following emission corridor curvature
    const auto continuousX = 500.0 - 46.0 * std::sin(kTwoPi * (burstV + 0.12)) + 14.0 * std::cos(kTwoPi * 4.0 * u);

    // Continuous pulse elevation at the exact sub-ridge coordinate
    const auto phiTrack1 = kTwoPi * (2.0 * u + 2.0 * burstV);
    const auto cosTrackHalf1 = std::cos(phiTrack1 * 0.5);
    const auto pulseTrack1 = static_cast<float>(cosTrackHalf1 * cosTrackHalf1 * cosTrackHalf1 * cosTrackHalf1);
    const auto primaryTrackPulse = 0.10f + 0.90f * pulseTrack1;

    const auto phiTrack2 = kTwoPi * (2.0 * u - 1.0 * burstV + 0.25);
    const auto cosTrackHalf2 = std::cos(phiTrack2 * 0.5);
    const auto pulseTrack2 = static_cast<float>(cosTrackHalf2 * cosTrackHalf2 * cosTrackHalf2 * cosTrackHalf2 * cosTrackHalf2 * cosTrackHalf2);

    const auto normTrackX = continuousX / static_cast<double>(kXMaximum);
    const auto rTrack1 = std::sin(kTwoPi * (22.0 * normTrackX - 8.0 * u + burstV * 3.0));
    const auto rTrack2 = std::cos(kTwoPi * (44.0 * normTrackX + 14.0 * u - burstV * 5.0));
    const auto trackMicro = static_cast<float>(0.42 * rTrack1 + 0.24 * rTrack2);

    const auto continuousElevation = primaryTrackPulse * 24.0f + pulseTrack2 * 8.0f + 2.5f * trackMicro;
    const auto continuousY = static_cast<float>(continuousYBase) + std::max(2.0f, continuousElevation);

    // Smooth fade at the top/bottom wrap-around boundary so it never visually pops or teleports
    auto opacity = 1.0;
    if (burstV < 0.07) {
        opacity = burstV / 0.07;
    } else if (burstV > 0.93) {
        opacity = (1.0 - burstV) / 0.07;
    }

    batch.peakX = static_cast<float>(continuousX);
    batch.peakY = continuousY;
    batch.burstOpacity = static_cast<float>(std::clamp(opacity, 0.0, 1.0));
    batch.peakRidgeIndex = std::clamp(static_cast<int>(std::round(continuousRidge)), 0, pulsarRidgeCount - 1);
}

} // namespace QAccelPlotExample
