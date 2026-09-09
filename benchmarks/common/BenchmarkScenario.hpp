//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

/// \file BenchmarkScenario.hpp
/// \brief Parameterized benchmark scenario definitions.

#include <QString>

#include <vector>

namespace QAccelPlot {

/// \brief Describes a single benchmark scenario with its parameters.
///
/// Each scenario defines a specific workload (point count, curve count, update
/// rate, etc.) that can be executed by both the regression benchmark runner and
/// the user-facing benchmark application.
///
/// \sa BenchmarkMetrics, BenchmarkReporter
class BenchmarkScenario {
public:
    /// \brief How data is updated each frame.
    enum class UpdateMode {
        Static,  ///< One existing buffer is set once, then the viewport is panned every frame.
        Live,    ///< Existing interleaved-float buffers are copied into the curves every frame.
        MaxRate, ///< Existing buffers are copied as fast as possible without an update-rate cap.
    };

    /// \brief Constructs a scenario with all parameters.
    BenchmarkScenario(const QString& name, const QString& label, const QString& description, int pointCount, int curveCount, UpdateMode updateMode, int durationSeconds = 10,
        int warmupSeconds = 2);

    /// \brief Returns the scenario's short identifier (e.g. "live_2m_single_curve").
    const QString& name() const;

    /// \brief Returns a short human-readable label for the UI (e.g. "Live 2M").
    const QString& label() const;

    /// \brief Returns a human-readable description of what this scenario measures.
    const QString& description() const;

    /// \brief Number of data points per curve.
    int pointCount() const;

    /// \brief Number of curves rendered simultaneously.
    int curveCount() const;

    /// \brief How data is updated between frames.
    UpdateMode updateMode() const;

    /// \brief Duration of the benchmark run in seconds (excluding warmup).
    int durationSeconds() const;

    /// \brief Warmup duration in seconds (frames discarded before measurement).
    int warmupSeconds() const;

    /// \brief Returns the default set of benchmark scenarios.
    ///
    /// This is the canonical list shared between the regression runner and
    /// the benchmark application.
    static std::vector<BenchmarkScenario> defaultScenarios();

private:
    QString name_;
    QString label_;
    QString description_;
    int pointCount_;
    int curveCount_;
    UpdateMode updateMode_;
    int durationSeconds_;
    int warmupSeconds_;
};

} // namespace QAccelPlot
