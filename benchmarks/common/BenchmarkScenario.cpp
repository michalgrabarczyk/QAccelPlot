//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "BenchmarkScenario.hpp"

namespace QAccelPlot {

BenchmarkScenario::BenchmarkScenario(const QString& name, const QString& label, const QString& description, const int pointCount, const int curveCount,
    const UpdateMode updateMode, const int durationSeconds, const int warmupSeconds)
    : name_(name)
    , label_(label)
    , description_(description)
    , pointCount_(pointCount)
    , curveCount_(curveCount)
    , updateMode_(updateMode)
    , durationSeconds_(durationSeconds)
    , warmupSeconds_(warmupSeconds)
{
}

const QString& BenchmarkScenario::name() const
{
    return name_;
}

const QString& BenchmarkScenario::label() const
{
    return label_;
}

const QString& BenchmarkScenario::description() const
{
    return description_;
}

int BenchmarkScenario::pointCount() const
{
    return pointCount_;
}

int BenchmarkScenario::curveCount() const
{
    return curveCount_;
}

BenchmarkScenario::UpdateMode BenchmarkScenario::updateMode() const
{
    return updateMode_;
}

int BenchmarkScenario::durationSeconds() const
{
    return durationSeconds_;
}

int BenchmarkScenario::warmupSeconds() const
{
    return warmupSeconds_;
}

std::vector<BenchmarkScenario> BenchmarkScenario::defaultScenarios()
{
    return {
        {QStringLiteral("static_10k"), QStringLiteral("Static 10K + Pan"),
            QStringLiteral("One existing 10K-point buffer, with the viewport panning back and forth every frame"), 10'000, 1, UpdateMode::Static, 5, 1},
        {QStringLiteral("static_100k"), QStringLiteral("Static 100K + Pan"),
            QStringLiteral("One existing 100K-point buffer, with the viewport panning back and forth every frame"), 100'000, 1, UpdateMode::Static, 5, 1},
        {QStringLiteral("static_1m"), QStringLiteral("Static 1M + Pan"),
            QStringLiteral("One existing 1M-point buffer, with the viewport panning back and forth every frame"), 1'000'000, 1, UpdateMode::Static, 5, 1},
        {QStringLiteral("static_2m"), QStringLiteral("Static 2M + Pan"),
            QStringLiteral("One existing 2M-point buffer, with the viewport panning back and forth every frame"), 2'000'000, 1, UpdateMode::Static, 5, 1},
        {QStringLiteral("static_5m"), QStringLiteral("Static 5M + Pan"),
            QStringLiteral("One existing 5M-point buffer, with the viewport panning back and forth every frame"), 5'000'000, 1, UpdateMode::Static, 5, 1},
        {QStringLiteral("live_100k"), QStringLiteral("Live 100K"), QStringLiteral("Existing 100K-point float buffers copied and rendered every frame"), 100'000,
            1, UpdateMode::Live, 10, 2},
        {QStringLiteral("live_500k"), QStringLiteral("Live 500K"), QStringLiteral("Existing 500K-point float buffers copied and rendered every frame"), 500'000,
            1, UpdateMode::Live, 10, 2},
        {QStringLiteral("live_1m"), QStringLiteral("Live 1M"), QStringLiteral("Existing 1M-point float buffers copied and rendered every frame"), 1'000'000, 1,
            UpdateMode::Live, 10, 2},
        {QStringLiteral("live_2m"), QStringLiteral("Live 2M"), QStringLiteral("Existing 2M-point float buffers copied and rendered every frame"), 2'000'000, 1,
            UpdateMode::Live, 10, 2},
        {QStringLiteral("live_5m"), QStringLiteral("Live 5M"), QStringLiteral("Existing 5M-point float buffers copied and rendered every frame — stress test"),
            5'000'000, 1, UpdateMode::Live, 10, 2},
        {QStringLiteral("live_10m"), QStringLiteral("Live 10M"),
            QStringLiteral("Existing 10M-point float buffers copied and rendered every frame — extreme stress test"), 10'000'000, 1, UpdateMode::Live, 10, 2},
        {QStringLiteral("live_multi_4x100k"), QStringLiteral("Multi 4×100K"), QStringLiteral("Existing buffers copied into four 100K-point curves every frame"),
            100'000, 4, UpdateMode::Live, 10, 2},
        {QStringLiteral("live_multi_4x500k"), QStringLiteral("Multi 4×500K"), QStringLiteral("Existing buffers copied into four 500K-point curves every frame"),
            500'000, 4, UpdateMode::Live, 10, 2},
        {QStringLiteral("data_ingestion_2m"), QStringLiteral("Ingestion 2M"), QStringLiteral("setDataF throughput at 2M points — CPU-only, max rate"),
            2'000'000, 1, UpdateMode::MaxRate, 5, 1},
        {QStringLiteral("vertex_cache_2m"), QStringLiteral("Vertex Cache 2M"), QStringLiteral("Vertex cache build cost at 2M points — CPU-only, max rate"),
            2'000'000, 1, UpdateMode::MaxRate, 5, 1},
    };
}

} // namespace QAccelPlot
