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

#include <QAccelPlot/renderers/LineCurveLineRenderer.hpp>
#include <QAccelPlot/series/LineCurve.hpp>
#include <QAccelPlot/series/PointCloud.hpp>
#include <QAccelPlot/series/PointSpatialIndex.hpp>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>

using namespace QAccelPlot;

namespace {

std::vector<float> makeSineData(const int pointCount)
{
    auto data = std::vector<float>(static_cast<std::size_t>(pointCount) * 2);
    for (auto i = 0; i < pointCount; ++i) {
        data[static_cast<std::size_t>(i) * 2] = static_cast<float>(i);
        data[static_cast<std::size_t>(i) * 2 + 1] = std::sin(static_cast<float>(i) * 0.01f);
    }
    return data;
}

double percentile(const std::vector<double>& sorted, const double p)
{
    const auto rank = (p / 100.0) * static_cast<double>(sorted.size() - 1);
    const auto lower = static_cast<std::size_t>(std::floor(rank));
    const auto upper = static_cast<std::size_t>(std::ceil(rank));
    const auto fraction = rank - std::floor(rank);
    if (lower == upper) {
        return sorted[lower];
    }
    return sorted[lower] * (1.0 - fraction) + sorted[upper] * fraction;
}

// Runs \a prepare (untimed) and \a iteration (timed) for the scenario's warmup and duration,
// then reports iteration-time percentiles and point throughput.
void runTimedLoop(BenchmarkReporter& reporter, const BenchmarkScenario& scenario, const std::function<void()>& prepare, const std::function<void()>& iteration)
{
    qDebug() << "Running scenario:" << scenario.name();

    const auto targetDurationMs = scenario.durationSeconds() * 1000.0;
    const auto warmupDurationMs = scenario.warmupSeconds() * 1000.0;

    auto iterationTimesMs = std::vector<double>{};
    auto timer = QElapsedTimer{};
    auto totalTimer = QElapsedTimer{};

    totalTimer.start();
    while (totalTimer.elapsed() < warmupDurationMs) {
        prepare();
        iteration();
    }

    totalTimer.restart();
    while (totalTimer.elapsed() < targetDurationMs) {
        prepare();
        timer.restart();
        iteration();
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
    results.dataThroughputMpps = (scenario.pointCount() * results.fpsAvg) / 1'000'000.0;

    reporter.addResult(scenario, results);
}

void runDataIngestionBenchmark(BenchmarkReporter& reporter, const BenchmarkScenario& scenario)
{
    const auto pointCount = scenario.pointCount();
    const auto data = makeSineData(pointCount);
    auto curve = LineCurve{};
    auto dataCopy = std::vector<float>{};

    // Copying the source buffer simulates new data arriving and is excluded from timing.
    runTimedLoop(reporter, scenario, [&]() { dataCopy = data; }, [&]() { curve.setDataFNoRange(std::move(dataCopy), pointCount); });
}

void runVertexCacheBenchmark(BenchmarkReporter& reporter, const BenchmarkScenario& scenario)
{
    const auto pointCount = scenario.pointCount();
    const auto data = makeSineData(pointCount);
    auto lineRenderer = LineCurveLineRenderer{};
    auto vertexCache = std::vector<char>{};

    runTimedLoop(reporter, scenario, []() {}, [&]() { lineRenderer.buildVertexCache(data, pointCount, vertexCache); });
}

void runPointCloudIngestionBenchmark(BenchmarkReporter& reporter, const BenchmarkScenario& scenario)
{
    const auto pointCount = scenario.pointCount();
    const auto data = makeSineData(pointCount);
    auto values = std::vector<float>(static_cast<std::size_t>(pointCount));
    for (auto i = std::size_t{0}; i < values.size(); ++i) {
        values[i] = static_cast<float>(i % 256) / 255.0f;
    }

    auto cloud = PointCloud{};
    auto dataCopy = std::vector<float>{};
    auto valuesCopy = std::vector<float>{};
    // Measures validation, interleaving into the (x, y, value) texture buffer and value-range resolution.
    runTimedLoop(
        reporter, scenario,
        [&]() {
            dataCopy = data;
            valuesCopy = values;
        },
        [&]() { cloud.setDataFNoRange(std::move(dataCopy), std::move(valuesCopy), pointCount); });
}

void runPointCloudHoverIndexBenchmark(BenchmarkReporter& reporter, const BenchmarkScenario& scenario)
{
    const auto pointCount = scenario.pointCount();
    // A 2D scatter rather than a sine: the index cost depends on how points spread over cells.
    auto data = std::vector<float>(static_cast<std::size_t>(pointCount) * 2);
    for (auto i = 0; i < pointCount; ++i) {
        const auto angle = static_cast<float>(i) * 2.399963f; // golden angle spiral
        const auto radius = std::sqrt(static_cast<float>(i));
        data[static_cast<std::size_t>(i) * 2] = radius * std::cos(angle);
        data[static_cast<std::size_t>(i) * 2 + 1] = radius * std::sin(angle);
    }

    auto index = PointSpatialIndex{};
    runTimedLoop(reporter, scenario, []() {}, [&]() { index.build(data.data(), pointCount); });
}

} // namespace

int main(int argc, char* argv[])
{
    auto app = QCoreApplication{argc, argv};
    QCoreApplication::setApplicationName("QAccelPlotBenchDataIngestion");
    QCoreApplication::setApplicationVersion("0.2.0");

    auto parser = QCommandLineParser{};
    parser.setApplicationDescription("QAccelPlot CPU-only micro-benchmarks");
    parser.addHelpOption();
    parser.addVersionOption();

    auto outputOption = QCommandLineOption{QStringList() << "o" << "output", "Output JSON results to <file>.", "file"};
    parser.addOption(outputOption);

    parser.process(app);

    auto reporter = BenchmarkReporter{"0.2.0"};
    reporter.setGraphicsApi("CPU-Only");

    const auto scenarios = BenchmarkScenario::defaultScenarios();
    for (const auto& scenario : scenarios) {
        if (scenario.name().startsWith("data_ingestion")) {
            runDataIngestionBenchmark(reporter, scenario);
        } else if (scenario.name().startsWith("vertex_cache")) {
            runVertexCacheBenchmark(reporter, scenario);
        } else if (scenario.name().startsWith("point_cloud_ingestion")) {
            runPointCloudIngestionBenchmark(reporter, scenario);
        } else if (scenario.name().startsWith("point_cloud_hover_index")) {
            runPointCloudHoverIndexBenchmark(reporter, scenario);
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
