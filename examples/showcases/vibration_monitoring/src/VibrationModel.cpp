//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "VibrationModel.hpp"

#include <cmath>

namespace QAccelPlotExample {

namespace {

constexpr auto kTwoPi = 6.283185307179586476925286766559;
// The pump turns at 25 Hz * 23 / 40 behind the gearbox; the gears mesh at 25 Hz * 23 teeth.
constexpr auto kPumpShaftHz = VibrationModel::kShaftHz * 23.0 / 40.0;
constexpr auto kGearMeshHz = VibrationModel::kShaftHz * 23.0;
constexpr auto kFanBladeHz = VibrationModel::kShaftHz * 6.0;
constexpr auto kPumpVaneHz = kPumpShaftHz * 6.0;
// Bearing impacts ring a housing resonance and push the housing once per impact.
constexpr auto kResonanceHz = 2900.0;
constexpr auto kRingDecaySeconds = 0.0011;
constexpr auto kPushDecaySeconds = 0.004;
constexpr auto kRingPeak = 2.6;
constexpr auto kPushPeak = 0.5;
// The pump load swings slowly, so the impacts grow and fade with a 4 s period.
constexpr auto kLoadPeriodSeconds = 4.0;
constexpr auto kLoadSwing = 0.2;
// The shafts speed up and slow down by 1.5 % once per period, so the spectral lines wander a little.
// The drift is zero two seconds into the period, where the demo zooms into an impact.
constexpr auto kSpeedDrift = 0.015;
constexpr auto kSpeedDriftCenterSeconds = 2.0;
constexpr auto kNoiseSeed = std::uint64_t{0x5eedf00dcafe1234};

std::uint64_t splitMix64(std::uint64_t value)
{
    value += 0x9e3779b97f4a7c15;
    value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9;
    value = (value ^ (value >> 27U)) * 0x94d049bb133111eb;
    return value ^ (value >> 31U);
}

} // namespace

QStringList VibrationModel::channelNames()
{
    return {QStringLiteral("Motor DE"), QStringLiteral("Motor NDE"), QStringLiteral("Gearbox In"), QStringLiteral("Gearbox Out"), QStringLiteral("Pump DE"),
        QStringLiteral("Pump NDE"), QStringLiteral("Fan"), QStringLiteral("Base")};
}

void VibrationModel::generate(const int channel, const std::int64_t firstSample, const int count, float* samples, const Condition condition) const
{
    const auto& mix = mixes().at(static_cast<std::size_t>(channel));
    for (auto i = 0; i < count; ++i) {
        const auto periodicIndex = ((firstSample + i) % kPeriodSamples + kPeriodSamples) % kPeriodSamples;
        const auto time = static_cast<double>(periodicIndex) / kSampleRate;
        auto value = tones(mix, time) + mix.noise * noise(periodicIndex, channel);
        if (mix.bearingFault > 0.0 && condition == Condition::Faulty) {
            value += mix.bearingFault * bearingImpacts(time);
        }
        samples[i] = static_cast<float>(value);
    }
}

const std::array<VibrationModel::ChannelMix, VibrationModel::kChannelCount>& VibrationModel::mixes()
{
    // shaft1x, shaft2x, pumpShaft, gearMesh, fanBlades, pumpVanes, bearingFault, noise, swingDepth, swingCycles, swingPhase
    static const auto table = std::array<ChannelMix, kChannelCount>{{
        {0.12, 0.05, 0.0, 0.05, 0.02, 0.0, 0.0, 0.025, 0.3, 1, 0.0}, // Motor DE
        {0.09, 0.03, 0.0, 0.0, 0.07, 0.0, 0.0, 0.025, 0.3, 2, 1.0},  // Motor NDE
        {0.06, 0.0, 0.0, 0.18, 0.0, 0.0, 0.0, 0.03, 0.35, 3, 2.0},   // Gearbox In
        {0.0, 0.0, 0.07, 0.16, 0.0, 0.0, 0.0, 0.03, 0.35, 3, 2.6},   // Gearbox Out
        {0.0, 0.0, 0.09, 0.04, 0.0, 0.06, 1.0, 0.035, 0.0, 1, 0.0},  // Pump DE
        {0.0, 0.0, 0.07, 0.0, 0.0, 0.05, 0.45, 0.03, 0.0, 1, 0.0},   // Pump NDE
        {0.04, 0.0, 0.0, 0.0, 0.09, 0.0, 0.0, 0.03, 0.4, 2, 4.0},    // Fan
        {0.03, 0.0, 0.02, 0.0, 0.0, 0.0, 0.0, 0.015, 0.4, 1, 3.0},   // Base
    }};
    return table;
}

double VibrationModel::tones(const ChannelMix& mix, const double time)
{
    const auto turns = shaftTime(time);
    const auto pinion = std::sin(kTwoPi * kShaftHz * turns);
    const auto gear = std::sin(kTwoPi * kPumpShaftHz * turns + 0.5);
    // The mesh tone is modulated by both shafts, which adds sidebands around it.
    const auto meshEnvelope = 1.0 + 0.35 * pinion + 0.25 * gear;
    const auto periodSeconds = static_cast<double>(kPeriodSamples) / kSampleRate;
    const auto swing = 1.0 + mix.swingDepth * std::sin(kTwoPi * mix.swingCycles * time / periodSeconds + mix.swingPhase);
    return swing
        * (mix.shaft1x * std::sin(kTwoPi * kShaftHz * turns + 0.3) + mix.shaft2x * std::sin(kTwoPi * 2.0 * kShaftHz * turns + 1.1)
            + mix.pumpShaft * std::sin(kTwoPi * kPumpShaftHz * turns + 0.7) + mix.gearMesh * meshEnvelope * std::sin(kTwoPi * kGearMeshHz * turns)
            + mix.fanBlades * std::sin(kTwoPi * kFanBladeHz * turns + 0.2) + mix.pumpVanes * std::sin(kTwoPi * kPumpVaneHz * turns + 2.0));
}

double VibrationModel::bearingImpacts(const double time)
{
    const auto impactPhase = shaftTime(time) * kBpfoHz;
    const auto sinceImpact = (impactPhase - std::floor(impactPhase)) / kBpfoHz;
    const auto ring = kRingPeak * std::exp(-sinceImpact / kRingDecaySeconds) * std::sin(kTwoPi * kResonanceHz * sinceImpact);
    // Subtracting the mean of the decaying push keeps the signal centered on zero.
    const auto pushMean = kPushDecaySeconds * kBpfoHz * (1.0 - std::exp(-1.0 / (kBpfoHz * kPushDecaySeconds)));
    const auto push = kPushPeak * (std::exp(-sinceImpact / kPushDecaySeconds) - pushMean);
    const auto load = 1.0 + kLoadSwing * std::sin(kTwoPi * time / kLoadPeriodSeconds);
    return load * (ring + push);
}

double VibrationModel::shaftTime(const double time)
{
    const auto periodSeconds = static_cast<double>(kPeriodSamples) / kSampleRate;
    const auto driftPhase = kTwoPi * (time - kSpeedDriftCenterSeconds) / periodSeconds;
    return time + kSpeedDrift * periodSeconds / kTwoPi * std::sin(driftPhase);
}

double VibrationModel::noise(const std::int64_t periodicIndex, const int channel)
{
    // The sum of four uniform values is close to normal; scaled to unit variance.
    const auto bits = splitMix64(static_cast<std::uint64_t>(periodicIndex) * kChannelCount + static_cast<std::uint64_t>(channel) + kNoiseSeed);
    auto sum = 0.0;
    for (auto part = 0U; part < 4U; ++part) {
        sum += static_cast<double>((bits >> (16U * part)) & 0xffffU) / 65536.0;
    }
    return (sum - 2.0) * std::sqrt(3.0);
}

} // namespace QAccelPlotExample
