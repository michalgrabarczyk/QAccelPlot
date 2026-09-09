//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "BenchmarkMetrics.hpp"
#include "BenchmarkReporter.hpp"
#include "BenchmarkScenario.hpp"

#include "renderers/LineCurveLineRenderer.hpp"
#include "series/LineCurve.hpp"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>

#include <cmath>
#include <vector>

using namespace QAccelPlot;

namespace {

void runDataIngestionBenchmark(BenchmarkReporter& reporter, const BenchmarkScenario& scenario)
{
    qDebug() << "Running scenario:" << scenario.name();

    const auto pointCount = scenario.pointCount();
    const auto durationSeconds = scenario.durationSeconds();
    const auto warmupSeconds = scenario.warmupSeconds();
    const auto targetDurationMs = durationSeconds * 1000.0;
    const auto warmupDurationMs = warmupSeconds * 1000.0;

    auto data = std::vector<float>(pointCount * 2);
    for (auto i = 0; i < pointCount; ++i) {
        data[i * 2] = static_cast<float>(i);
        data[i * 2 + 1] = std::sin(static_cast<float>(i) * 0.01f);
    }

    auto curve = LineCurve{};
    auto metrics = BenchmarkMetrics{};
    // Warmups are based on time rather than frames for this raw throughput test,
    // whereas the metrics class expects frame counts. A custom timing loop is used
    // for this microbenchmark since it bypasses standard frame boundaries.

    // Custom metrics for CPU benchmark
    auto iterationTimesMs = std::vector<double>{};
    auto timer = QElapsedTimer{};
    auto totalTimer = QElapsedTimer{};

    totalTimer.start();

    // Warmup
    while (totalTimer.elapsed() < warmupDurationMs) {
        auto dataCopy = data; // simulate having new data
        curve.setDataFNoRange(std::move(dataCopy), pointCount);
    }

    // Measurement
    totalTimer.restart();
    while (totalTimer.elapsed() < targetDurationMs) {
        auto dataCopy = data;
        timer.restart();
        curve.setDataFNoRange(std::move(dataCopy), pointCount);
        iterationTimesMs.push_back(timer.nsecsElapsed() / 1'000'000.0);
    }

    if (iterationTimesMs.empty()) {
        return;
    }

    std::sort(iterationTimesMs.begin(), iterationTimesMs.end());

    auto results = BenchmarkMetrics::Results{};
    results.totalFrames = iterationTimesMs.size();

    auto sumMs = 0.0;
    for (const auto t : iterationTimesMs) {
        sumMs += t;
    }
    results.elapsedSeconds = sumMs / 1000.0;

    results.fpsAvg = results.totalFrames / results.elapsedSeconds;

    const auto percentile = [](const std::vector<double>& sorted, double p) {
        const auto rank = (p / 100.0) * (sorted.size() - 1);
        const auto lower = static_cast<size_t>(std::floor(rank));
        const auto upper = static_cast<size_t>(std::ceil(rank));
        const auto fraction = rank - std::floor(rank);
        if (lower == upper) {
            return sorted[lower];
        }
        return sorted[lower] * (1.0 - fraction) + sorted[upper] * fraction;
    };

    results.frameTimeP50Ms = percentile(iterationTimesMs, 50.0);
    results.frameTimeP95Ms = percentile(iterationTimesMs, 95.0);
    results.frameTimeP99Ms = percentile(iterationTimesMs, 99.0);
    results.frameTimeMinMs = iterationTimesMs.front();
    results.frameTimeMaxMs = iterationTimesMs.back();
    results.frameIntervalP50Ms = results.frameTimeP50Ms;
    results.frameIntervalP95Ms = results.frameTimeP95Ms;
    results.frameIntervalP99Ms = results.frameTimeP99Ms;
    results.frameIntervalMinMs = results.frameTimeMinMs;
    results.frameIntervalMaxMs = results.frameTimeMaxMs;
    results.fpsMin = 1000.0 / results.frameTimeMaxMs;
    results.dataThroughputMpps = (pointCount * results.fpsAvg) / 1'000'000.0;

    reporter.addResult(scenario, results);
}

void runVertexCacheBenchmark(BenchmarkReporter& reporter, const BenchmarkScenario& scenario)
{
    qDebug() << "Running scenario:" << scenario.name();

    const auto pointCount = scenario.pointCount();
    const auto durationSeconds = scenario.durationSeconds();
    const auto warmupSeconds = scenario.warmupSeconds();
    const auto targetDurationMs = durationSeconds * 1000.0;
    const auto warmupDurationMs = warmupSeconds * 1000.0;

    auto data = std::vector<float>(pointCount * 2);
    for (auto i = 0; i < pointCount; ++i) {
        data[i * 2] = static_cast<float>(i);
        data[i * 2 + 1] = std::sin(static_cast<float>(i) * 0.01f);
    }

    auto lineRenderer = LineCurveLineRenderer{};
    auto vertexCache = std::vector<char>{};

    auto iterationTimesMs = std::vector<double>{};
    auto timer = QElapsedTimer{};
    auto totalTimer = QElapsedTimer{};

    totalTimer.start();

    // Warmup
    while (totalTimer.elapsed() < warmupDurationMs) {
        lineRenderer.buildVertexCache(data, pointCount, vertexCache);
    }

    // Measurement
    totalTimer.restart();
    while (totalTimer.elapsed() < targetDurationMs) {
        timer.restart();
        lineRenderer.buildVertexCache(data, pointCount, vertexCache);
        iterationTimesMs.push_back(timer.nsecsElapsed() / 1'000'000.0);
    }

    if (iterationTimesMs.empty()) {
        return;
    }

    std::sort(iterationTimesMs.begin(), iterationTimesMs.end());

    auto results = BenchmarkMetrics::Results{};
    results.totalFrames = iterationTimesMs.size();

    auto sumMs = 0.0;
    for (double t : iterationTimesMs) {
        sumMs += t;
    }
    results.elapsedSeconds = sumMs / 1000.0;

    results.fpsAvg = results.totalFrames / results.elapsedSeconds;

    const auto percentile = [](const std::vector<double>& sorted, double p) {
        const auto rank = (p / 100.0) * (sorted.size() - 1);
        const auto lower = static_cast<size_t>(std::floor(rank));
        const auto upper = static_cast<size_t>(std::ceil(rank));
        const auto fraction = rank - std::floor(rank);
        if (lower == upper) {
            return sorted[lower];
        }
        return sorted[lower] * (1.0 - fraction) + sorted[upper] * fraction;
    };

    results.frameTimeP50Ms = percentile(iterationTimesMs, 50.0);
    results.frameTimeP95Ms = percentile(iterationTimesMs, 95.0);
    results.frameTimeP99Ms = percentile(iterationTimesMs, 99.0);
    results.frameTimeMinMs = iterationTimesMs.front();
    results.frameTimeMaxMs = iterationTimesMs.back();
    results.frameIntervalP50Ms = results.frameTimeP50Ms;
    results.frameIntervalP95Ms = results.frameTimeP95Ms;
    results.frameIntervalP99Ms = results.frameTimeP99Ms;
    results.frameIntervalMinMs = results.frameTimeMinMs;
    results.frameIntervalMaxMs = results.frameTimeMaxMs;
    results.fpsMin = 1000.0 / results.frameTimeMaxMs;
    results.dataThroughputMpps = (pointCount * results.fpsAvg) / 1'000'000.0;

    reporter.addResult(scenario, results);
}

} // namespace

int main(int argc, char* argv[])
{
    auto app = QCoreApplication{argc, argv};
    QCoreApplication::setApplicationName("QAccelPlotBenchDataIngestion");
    QCoreApplication::setApplicationVersion("0.1.0");

    auto parser = QCommandLineParser{};
    parser.setApplicationDescription("QAccelPlot CPU-only micro-benchmarks");
    parser.addHelpOption();
    parser.addVersionOption();

    auto outputOption = QCommandLineOption{QStringList() << "o" << "output", "Output JSON results to <file>.", "file"};
    parser.addOption(outputOption);

    parser.process(app);

    auto reporter = BenchmarkReporter{"0.1.0"};
    reporter.setGraphicsApi("CPU-Only");

    const auto scenarios = BenchmarkScenario::defaultScenarios();
    for (const auto& scenario : scenarios) {
        if (scenario.name().startsWith("data_ingestion")) {
            runDataIngestionBenchmark(reporter, scenario);
        } else if (scenario.name().startsWith("vertex_cache")) {
            runVertexCacheBenchmark(reporter, scenario);
        }
    }

    reporter.printConsoleTable();

    if (parser.isSet(outputOption)) {
        const auto outPath = parser.value(outputOption);
        if (reporter.writeJson(outPath)) {
            qDebug() << "Results written to" << outPath;
        } else {
            qWarning() << "Failed to write results to" << outPath;
            return 1;
        }
    }

    return 0;
}
