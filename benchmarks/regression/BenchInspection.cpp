//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include <QAccelPlot/series/LineCurve.hpp>
#include <QAccelPlot/series/PointCloud.hpp>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <vector>

namespace {

using QAccelPlot::InspectionStatus;

constexpr auto kInfinity = std::numeric_limits<double>::infinity();

QJsonObject timings(const std::function<void(int)>& query)
{
    constexpr auto kRuns = 1000;
    auto times = std::vector<double>{};
    times.reserve(kRuns);
    for (auto i = 0; i < kRuns; ++i) {
        auto timer = QElapsedTimer{};
        timer.start();
        query(i);
        times.push_back(static_cast<double>(timer.nsecsElapsed()) / 1000);
    }
    std::sort(times.begin(), times.end());
    return {{QStringLiteral("p50Us"), times[kRuns / 2]}, {QStringLiteral("p95Us"), times[kRuns * 95 / 100]}, {QStringLiteral("maxUs"), times.back()}};
}

void bindAxes(QAccelPlot::PlotSeries& series, QAccelPlot::Axis& x, QAccelPlot::Axis& y, const double xMax)
{
    y.setSide(QAccelPlot::Axis::Left);
    x.setViewportMin(0);
    x.setViewportMax(xMax);
    y.setViewportMin(-2);
    y.setViewportMax(2);
    series.setXAxis(&x);
    series.setYAxis(&y);
    series.setSize({1000, 400});
}

std::vector<double> scenarioData(const QString& name, const int count)
{
    auto generator = std::mt19937{42};
    auto random = std::uniform_real_distribution<double>{0, 1};
    auto data = std::vector<double>(static_cast<std::size_t>(count) * 2);
    for (auto i = 0; i < count; ++i) {
        const auto base = static_cast<std::size_t>(i) * 2;
        if (name == QLatin1String("ordered")) {
            data[base] = static_cast<double>(i) * 1000 / count;
            data[base + 1] = std::sin(data[base]);
        } else if (name == QLatin1String("random")) {
            data[base] = random(generator) * 1000;
            data[base + 1] = random(generator) * 4 - 2;
        } else {
            // Clustered: every point in two coincident clusters, out of X order.
            data[base] = i % 2 == 0 ? 500 : 250;
            data[base + 1] = 0;
        }
    }
    return data;
}

// Time until queries can run, with the event loop spinning freely as it does in an idle application.
QJsonObject prepare(QAccelPlot::SeriesInspection& inspection)
{
    auto timer = QElapsedTimer{};
    timer.start();
    auto worstEventLoopMs = 0.0;
    inspection.prepare();
    while (inspection.status() != InspectionStatus::Ready && timer.elapsed() < 60000) {
        auto iteration = QElapsedTimer{};
        iteration.start();
        QCoreApplication::processEvents();
        worstEventLoopMs = std::max(worstEventLoopMs, static_cast<double>(iteration.nsecsElapsed()) / 1e6);
    }
    return {{QStringLiteral("preparationMs"), static_cast<double>(timer.nsecsElapsed()) / 1e6}, {QStringLiteral("worstEventLoopMs"), worstEventLoopMs},
        {QStringLiteral("ready"), inspection.status() == InspectionStatus::Ready}};
}

QJsonObject scenario(const QString& name, const int count)
{
    auto cloud = QAccelPlot::PointCloud{};
    auto x = QAccelPlot::Axis{};
    auto y = QAccelPlot::Axis{};
    bindAxes(cloud, x, y, 1000);
    cloud.setData(scenarioData(name, count), count);
    auto* inspection = cloud.inspection();
    auto result = QJsonObject{{QStringLiteral("scenario"), name}, {QStringLiteral("points"), count}, {QStringLiteral("preparation"), prepare(*inspection)}};
    if (inspection->status() != InspectionStatus::Ready) {
        return result;
    }
    result.insert(QStringLiteral("nearestX"), timings([&](const int i) { inspection->nearestByX(i, 10); }));
    result.insert(QStringLiteral("nearestXY"), timings([&](const int i) { inspection->nearest({static_cast<double>(i), 200}, 10); }));
    result.insert(QStringLiteral("rangeSummary"), timings([&](const int i) { inspection->summarizeRange(i, i + 1); }));
    result.insert(QStringLiteral("boxSummary"), timings([&](const int i) { inspection->summarize(i, i + 1, -.5, .5); }));
    result.insert(QStringLiteral("fullSummary"), timings([&](const int /*i*/) { inspection->summarizeRange(-kInfinity, kInfinity); }));
    result.insert(QStringLiteral("selectionPage"), timings([&](const int i) { inspection->indices(0, 1000, -2, 2, i * 256, 256); }));
    // Reported after the queries, because statistics for ordered data are cached on first use.
    result.insert(QStringLiteral("indexBytes"), static_cast<qint64>(inspection->indexBytes()));
    result.insert(QStringLiteral("indexBytesPerPoint"), static_cast<double>(inspection->indexBytes()) / count);
    return result;
}

// A live curve: one record appended per frame, then inspected at the newest sample.
QJsonObject appendStream(const int count)
{
    auto curve = QAccelPlot::LineCurve{};
    auto x = QAccelPlot::Axis{};
    auto y = QAccelPlot::Axis{};
    bindAxes(curve, x, y, count + 1000.0);
    auto data = std::vector<double>(static_cast<std::size_t>(count) * 2);
    for (auto i = 0; i < count; ++i) {
        data[static_cast<std::size_t>(i) * 2] = i;
        data[static_cast<std::size_t>(i) * 2 + 1] = std::sin(i * 0.01);
    }
    curve.setData(std::move(data), count);
    auto* inspection = curve.inspection();
    inspection->summarizeRange(-kInfinity, kInfinity);
    auto next = count;
    auto readyFrames = 0;
    const auto frame = timings([&](const int /*i*/) {
        curve.appendData(next, std::sin(next * 0.01));
        readyFrames += inspection->nearestByX(x.coordToPixel(next, 1000)).index == next ? 1 : 0;
        readyFrames += inspection->summarizeRange(-kInfinity, kInfinity).count == next + 1 ? 1 : 0;
        ++next;
    });
    return {{QStringLiteral("scenario"), QStringLiteral("appendStream")}, {QStringLiteral("points"), count}, {QStringLiteral("appendAndInspect"), frame},
        {QStringLiteral("readyQueries"), readyFrames}, {QStringLiteral("expectedReadyQueries"), 2000}};
}

// A live curve whose whole buffer is replaced every frame; the cursor queries stay available.
QJsonObject replaceStream(const int count)
{
    auto curve = QAccelPlot::LineCurve{};
    auto x = QAccelPlot::Axis{};
    auto y = QAccelPlot::Axis{};
    bindAxes(curve, x, y, count);
    auto data = std::vector<float>(static_cast<std::size_t>(count) * 2);
    for (auto i = 0; i < count; ++i) {
        data[static_cast<std::size_t>(i) * 2] = static_cast<float>(i);
        data[static_cast<std::size_t>(i) * 2 + 1] = std::sin(static_cast<float>(i) * 0.01f);
    }
    auto* inspection = curve.inspection();
    const auto replaceOnly = timings([&](const int /*i*/) { curve.setDataFNoRange(data.data(), count); });
    auto readyFrames = 0;
    const auto replaceAndInspect = timings([&](const int i) {
        curve.setDataFNoRange(data.data(), count);
        readyFrames += inspection->nearestByX(i).valid() ? 1 : 0;
        inspection->summarizeRange(i * count / 1000.0, (i + 1) * count / 1000.0);
    });
    return {{QStringLiteral("scenario"), QStringLiteral("replaceStream")}, {QStringLiteral("points"), count}, {QStringLiteral("replaceOnly"), replaceOnly},
        {QStringLiteral("replaceAndInspect"), replaceAndInspect}, {QStringLiteral("readyQueries"), readyFrames},
        {QStringLiteral("expectedReadyQueries"), 1000}};
}

QJsonObject manySeries(const int total)
{
    constexpr auto kSeries = 16;
    auto x = QAccelPlot::Axis{};
    auto y = QAccelPlot::Axis{};
    auto clouds = std::vector<std::unique_ptr<QAccelPlot::PointCloud>>{};
    const auto count = std::max(1, total / kSeries);
    for (auto series = 0; series < kSeries; ++series) {
        auto cloud = std::make_unique<QAccelPlot::PointCloud>();
        bindAxes(*cloud, x, y, 1000);
        auto data = std::vector<double>{};
        data.reserve(static_cast<std::size_t>(count) * 2);
        for (auto point = 0; point < count; ++point) {
            const auto position = static_cast<double>(point) * 1000 / count;
            data.push_back(position);
            data.push_back(std::sin(position + series));
        }
        cloud->setData(std::move(data), count);
        clouds.push_back(std::move(cloud));
    }
    const auto frame = timings([&clouds](const int i) {
        for (const auto& cloud : clouds) {
            cloud->inspection()->bracketByX(i);
            cloud->inspection()->nearestByX(i);
            cloud->inspection()->summarizeRange(i, i + 1);
        }
    });
    return {{QStringLiteral("scenario"), QStringLiteral("manySeries")}, {QStringLiteral("series"), kSeries}, {QStringLiteral("pointsPerSeries"), count},
        {QStringLiteral("cursorFrame"), frame}};
}

} // namespace

int main(int argc, char* argv[])
{
    auto app = QGuiApplication{argc, argv};
    auto count = 1000000;
    if (app.arguments().size() > 1) {
        count = app.arguments().at(1).toInt();
    }
    if (count <= 0) {
        return 1;
    }
    auto results = QJsonArray{};
    for (const auto& name : {QStringLiteral("ordered"), QStringLiteral("random"), QStringLiteral("clustered")}) {
        results.push_back(scenario(name, count));
    }
    results.push_back(appendStream(count / 5));
    results.push_back(replaceStream(count));
    results.push_back(manySeries(count));
    std::cout << QJsonDocument{results}.toJson().constData() << std::flush;
    return 0;
}
