//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QStringList>

#include <array>
#include <cstdint>

namespace QAccelPlotExample {

/// \brief Synthetic accelerometer signals of a motor that drives a pump through a gearbox.
///
/// Every channel mixes shaft, gear-mesh, fan, and vane-pass tones with seeded noise. The outer
/// race of the Pump DE bearing has a fault: impacts at the ball pass frequency (BPFO) that ring a
/// structural resonance, strongest on Pump DE and weaker on Pump NDE. Every component repeats
/// after \c kPeriodSamples, so a recording of one period loops seamlessly, and a sample depends
/// only on its index, so the signals are deterministic. Thread-safe: \c generate() is const.
class VibrationModel {
public:
    /// \brief Number of accelerometer channels.
    static constexpr int kChannelCount = 8;
    /// \brief Samples per second of every channel.
    static constexpr int kSampleRate = 25600;
    /// \brief Samples after which every channel repeats: 8 seconds.
    static constexpr std::int64_t kPeriodSamples = std::int64_t{8} * kSampleRate;
    /// \brief Motor shaft speed in hertz (1500 RPM).
    static constexpr double kShaftHz = 25.0;
    /// \brief Ball pass frequency of the outer race of the faulty Pump DE bearing, in hertz.
    static constexpr double kBpfoHz = 106.25;
    /// \brief Index of the Pump DE channel, the one with the bearing fault.
    static constexpr int kFaultChannel = 4;

    /// \brief State of the machine that \c generate() simulates.
    enum class Condition {
        Faulty, ///< \brief With the outer-race fault of the Pump DE bearing.
        Healthy ///< \brief Without the fault, as when the alarm baselines were taken.
    };

    /// \brief Returns the channel names, from the motor drive end to the base plate.
    static QStringList channelNames();

    /// \brief Writes \a count samples of \a channel, in g, starting at sample index \a firstSample.
    void generate(int channel, std::int64_t firstSample, int count, float* samples, Condition condition = Condition::Faulty) const;

private:
    // Peak amplitudes in g of the components that one channel picks up.
    struct ChannelMix {
        double shaft1x{0.0};
        double shaft2x{0.0};
        double pumpShaft{0.0};
        double gearMesh{0.0};
        double fanBlades{0.0};
        double pumpVanes{0.0};
        double bearingFault{0.0};
        double noise{0.0};
        // Slow swing of the tones, as flow and load drift: relative depth and cycles per period.
        double swingDepth{0.0};
        int swingCycles{1};
        double swingPhase{0.0};
    };

    static const std::array<ChannelMix, kChannelCount>& mixes();
    static double tones(const ChannelMix& mix, double time);
    static double bearingImpacts(double time);
    // Time as the shafts see it: the angle every rotating part has turned through, in seconds at nominal speed.
    static double shaftTime(double time);
    static double noise(std::int64_t periodicIndex, int channel);
};

} // namespace QAccelPlotExample
