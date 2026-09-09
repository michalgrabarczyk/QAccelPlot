//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "BenchmarkReporter.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSysInfo>
#include <QTextStream>
#include <QThread>

#include <cstdio>

namespace QAccelPlot {

namespace {

QString workloadName(const BenchmarkScenario& scenario)
{
    if (scenario.name().startsWith(QStringLiteral("data_ingestion"))) {
        return QStringLiteral("existing_buffer_ingestion");
    }
    if (scenario.name().startsWith(QStringLiteral("vertex_cache"))) {
        return QStringLiteral("vertex_cache_build");
    }
    return scenario.updateMode() == BenchmarkScenario::UpdateMode::Static ? QStringLiteral("static_buffer_bounded_pan")
                                                                          : QStringLiteral("existing_buffer_update_and_render");
}

int reportedSourceBufferCount(const BenchmarkScenario& scenario)
{
    if (scenario.name().startsWith(QStringLiteral("data_ingestion")) || scenario.name().startsWith(QStringLiteral("vertex_cache"))) {
        return 1;
    }
    return scenario.updateMode() == BenchmarkScenario::UpdateMode::Static ? 1 : 2;
}

} // namespace

BenchmarkReporter::BenchmarkReporter(const QString& libraryVersion)
    : libraryVersion_(libraryVersion)
{
}

void BenchmarkReporter::setGraphicsApi(const QString& api)
{
    graphicsApi_ = api;
}

void BenchmarkReporter::addResult(const BenchmarkScenario& scenario, const BenchmarkMetrics::Results& results)
{
    entries_.push_back({scenario, results});
}

QString BenchmarkReporter::toJson() const
{
    auto root = QJsonObject{};
    root[QStringLiteral("library")] = QStringLiteral("QAccelPlot");
    root[QStringLiteral("version")] = libraryVersion_;
    root[QStringLiteral("timestamp")] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    root[QStringLiteral("frame_time_scope")] = QStringLiteral("data_or_viewport_update_to_offscreen_gpu_completion");
    root[QStringLiteral("frame_interval_scope")] = QStringLiteral("consecutive_offscreen_gpu_completions");

    // System info.
    auto system = QJsonObject{};
    system[QStringLiteral("os")] = QSysInfo::prettyProductName();
    system[QStringLiteral("cpu_arch")] = QSysInfo::currentCpuArchitecture();
    system[QStringLiteral("cpu_cores")] = QThread::idealThreadCount();
    system[QStringLiteral("qt_version")] = QString::fromLatin1(qVersion());
    system[QStringLiteral("graphics_api")] = graphicsApi_;
    root[QStringLiteral("system")] = system;

    // Scenarios.
    auto scenarios = QJsonArray{};
    for (const auto& entry : entries_) {
        auto obj = QJsonObject{};
        obj[QStringLiteral("name")] = entry.scenario.name();
        obj[QStringLiteral("description")] = entry.scenario.description();
        obj[QStringLiteral("point_count")] = entry.scenario.pointCount();
        obj[QStringLiteral("curve_count")] = entry.scenario.curveCount();
        obj[QStringLiteral("duration_seconds")] = entry.scenario.durationSeconds();
        obj[QStringLiteral("workload")] = workloadName(entry.scenario);
        obj[QStringLiteral("data_generation_included")] = false;
        obj[QStringLiteral("source_buffer_count")] = reportedSourceBufferCount(entry.scenario);

        auto results = QJsonObject{};
        results[QStringLiteral("fps_avg")] = entry.results.fpsAvg;
        results[QStringLiteral("fps_min")] = entry.results.fpsMin;
        results[QStringLiteral("frame_time_p50_ms")] = entry.results.frameTimeP50Ms;
        results[QStringLiteral("frame_time_p95_ms")] = entry.results.frameTimeP95Ms;
        results[QStringLiteral("frame_time_p99_ms")] = entry.results.frameTimeP99Ms;
        results[QStringLiteral("frame_time_min_ms")] = entry.results.frameTimeMinMs;
        results[QStringLiteral("frame_time_max_ms")] = entry.results.frameTimeMaxMs;
        results[QStringLiteral("frame_interval_p50_ms")] = entry.results.frameIntervalP50Ms;
        results[QStringLiteral("frame_interval_p95_ms")] = entry.results.frameIntervalP95Ms;
        results[QStringLiteral("frame_interval_p99_ms")] = entry.results.frameIntervalP99Ms;
        results[QStringLiteral("frame_interval_min_ms")] = entry.results.frameIntervalMinMs;
        results[QStringLiteral("frame_interval_max_ms")] = entry.results.frameIntervalMaxMs;
        results[QStringLiteral("total_frames")] = static_cast<qint64>(entry.results.totalFrames);
        results[QStringLiteral("elapsed_seconds")] = entry.results.elapsedSeconds;
        results[QStringLiteral("data_throughput_mpps")] = entry.results.dataThroughputMpps;
        obj[QStringLiteral("results")] = results;

        scenarios.append(obj);
    }
    root[QStringLiteral("scenarios")] = scenarios;

    const auto doc = QJsonDocument(root);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
}

bool BenchmarkReporter::writeJson(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    file.write(toJson().toUtf8());
    return true;
}

void BenchmarkReporter::printConsoleTable() const
{
    if (entries_.empty()) {
        std::printf("No benchmark results to report.\n");
        return;
    }

    // Header.
    std::printf("\n");
    std::printf("QAccelPlot Benchmark Results  (v%s)\n", qPrintable(libraryVersion_));
    std::printf("OS: %s | CPU: %s (%d cores) | Qt: %s | Graphics: %s\n", qPrintable(QSysInfo::prettyProductName()),
        qPrintable(QSysInfo::currentCpuArchitecture()), QThread::idealThreadCount(), qVersion(), qPrintable(graphicsApi_));
    std::printf("\n");

    // Table header.
    std::printf(
        "%-28s %8s %8s %10s %10s %10s %10s %10s %12s\n", "Scenario", "FPS avg", "FPS min", "P50 (ms)", "P95 (ms)", "P99 (ms)", "Int P95", "Frames", "Mpts/s");
    std::printf("%-28s %8s %8s %10s %10s %10s %10s %10s %12s\n", "----------------------------", "--------", "--------", "----------", "----------",
        "----------", "----------", "----------", "------------");

    // Rows.
    for (const auto& entry : entries_) {
        const auto& r = entry.results;
        std::printf("%-28s %8.1f %8.1f %10.2f %10.2f %10.2f %10.2f %10lld %12.1f\n", qPrintable(entry.scenario.name()), r.fpsAvg, r.fpsMin, r.frameTimeP50Ms,
            r.frameTimeP95Ms, r.frameTimeP99Ms, r.frameIntervalP95Ms, static_cast<long long>(r.totalFrames), r.dataThroughputMpps);
    }

    std::printf("\n");
}

QString BenchmarkReporter::formatNumber(const double value, const int decimals) const
{
    return QString::number(value, 'f', decimals);
}

} // namespace QAccelPlot
