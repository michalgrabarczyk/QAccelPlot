//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExampleUtils.hpp"
#include "PulsarWorker.hpp"
#include "series/LineCurve.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QTimer>
#include <QVariant>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <vector>

namespace {

struct PerformanceMetrics {
    std::atomic<std::uint64_t> presentedFrames{0};
    std::atomic<std::uint64_t> appliedBatches{0};
    std::atomic<std::uint64_t> frameTimeNanoseconds{0};
    std::atomic<std::uint64_t> frameTimeSamples{0};
    std::atomic<std::uint64_t> previousFrameNanoseconds{0};
};

std::uint64_t steadyNanoseconds()
{
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(now).count());
}

void applyBatch(const std::vector<QAccelPlot::LineCurve*>& ridges, QObject* root, const QAccelPlotExample::PulsarBatch& batch, const bool initial)
{
    for (std::size_t i = 0; i < ridges.size(); ++i) {
        if (i < batch.ridges.size()) {
            if (initial) {
                ridges[i]->setDataF(batch.ridges[i].data(), batch.pointsPerRidge);
            } else {
                ridges[i]->setDataFNoRange(batch.ridges[i].data(), batch.pointsPerRidge);
            }
        }
    }

    (void)root;
}

void setupRenderingLoop(QGuiApplication& app, QQuickWindow* window, const std::vector<QAccelPlot::LineCurve*>& ridges, QObject* root,
    QAccelPlotExample::PulsarWorker& worker, PerformanceMetrics& metrics, const bool screenshotMode)
{
    if (screenshotMode) {
        return;
    }

    auto* elapsedTimer = new QElapsedTimer{};
    elapsedTimer->start();

    QObject::connect(window, &QQuickWindow::afterAnimating, &app, [window, ridges, root, &worker, &metrics, elapsedTimer]() {
        const auto elapsedSeconds = static_cast<double>(elapsedTimer->elapsed()) / 1000.0;
        worker.setElapsedSeconds(elapsedSeconds);
        auto batch = QAccelPlotExample::PulsarBatch{};
        if (!worker.tryConsume(batch)) {
            return;
        }
        applyBatch(ridges, root, batch, false);
        metrics.appliedBatches.fetch_add(1, std::memory_order_relaxed);
    });

    QObject::connect(&app, &QCoreApplication::aboutToQuit, &app, [elapsedTimer]() { delete elapsedTimer; });
}

void setupMetrics(QGuiApplication& app, QQuickWindow* window, QObject* root, PerformanceMetrics& metrics, const bool enabled)
{
    if (!enabled) {
        return;
    }

    QObject::connect(
        window, &QQuickWindow::frameSwapped, &app,
        [&metrics]() {
            const auto now = steadyNanoseconds();
            const auto previous = metrics.previousFrameNanoseconds.exchange(now, std::memory_order_relaxed);
            metrics.presentedFrames.fetch_add(1, std::memory_order_relaxed);
            if (previous > 0) {
                metrics.frameTimeNanoseconds.fetch_add(now - previous, std::memory_order_relaxed);
                metrics.frameTimeSamples.fetch_add(1, std::memory_order_relaxed);
            }
        },
        Qt::DirectConnection);

    auto* reportTimer = new QTimer(&app);
    reportTimer->setInterval(1000);
    QObject::connect(reportTimer, &QTimer::timeout, &app, [root, &metrics]() {
        const auto frames = metrics.presentedFrames.exchange(0, std::memory_order_relaxed);
        const auto updates = metrics.appliedBatches.exchange(0, std::memory_order_relaxed);
        const auto frameNanoseconds = metrics.frameTimeNanoseconds.exchange(0, std::memory_order_relaxed);
        const auto samples = metrics.frameTimeSamples.exchange(0, std::memory_order_relaxed);
        root->setProperty("fps", QVariant::fromValue(static_cast<qulonglong>(frames)));
        root->setProperty("updateRate", QVariant::fromValue(static_cast<qulonglong>(updates)));
        root->setProperty("frameTimeMs", samples > 0 ? frameNanoseconds / (samples * 1'000'000.0) : 0.0);
    });
    reportTimer->start();
}

double requestedPhase(const QStringList& arguments)
{
    const auto phaseIndex = arguments.indexOf(QStringLiteral("--phase"));
    if (phaseIndex == -1 || phaseIndex + 1 >= arguments.size()) {
        return 0.25; // Default phase showing majestic central cascade
    }

    auto valid = false;
    const auto phase = arguments.at(phaseIndex + 1).toDouble(&valid);
    return valid ? phase : 0.25;
}

} // namespace

int main(int argc, char* argv[])
{
    QLocale::setDefault(QLocale::English);
    QAccelPlotExample::configureGraphicsApi();

    auto surfaceFormat = QSurfaceFormat::defaultFormat();
    surfaceFormat.setSamples(8);
    QSurfaceFormat::setDefaultFormat(surfaceFormat);

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QAccelPlotExample::setupEngineFailureHandler(app, engine);

    const auto screenshotMode = app.arguments().contains(QStringLiteral("--screenshot"));
    engine.load(QUrl(u"qrc:/app/qml/main.qml"_qs));

    auto* root = engine.rootObjects().value(0);
    auto* window = qobject_cast<QQuickWindow*>(root);

    if (!window || !root) {
        return EXIT_FAILURE;
    }

    auto ridges = std::vector<QAccelPlot::LineCurve*>{};
    ridges.reserve(QAccelPlotExample::pulsarRidgeCount);
    for (auto i = int{0}; i < QAccelPlotExample::pulsarRidgeCount; ++i) {
        auto* curve = root->findChild<QAccelPlot::LineCurve*>(QStringLiteral("ridge_%1").arg(i));
        if (!curve) {
            qCritical() << "Unable to find ridge curve:" << i;
            return EXIT_FAILURE;
        }
        ridges.push_back(curve);
    }

    root->setProperty("pointCount", QAccelPlotExample::pulsarRidgeCount * QAccelPlotExample::pulsarPointsPerRidge);

    if (screenshotMode) {
        const auto phase = requestedPhase(app.arguments());
        const auto batch = QAccelPlotExample::PulsarWorker::computeBatchForPhase(phase);
        applyBatch(ridges, root, batch, true);
        root->setProperty("fps", 60);
        root->setProperty("updateRate", 60);
        QAccelPlotExample::setupScreenshotHandler(app, engine, 100);
        return app.exec();
    }

    auto worker = QAccelPlotExample::PulsarWorker{};
    auto metrics = PerformanceMetrics{};
    worker.start();

    // Initial batch at phase 0
    const auto initialBatch = QAccelPlotExample::PulsarWorker::computeBatchForPhase(0.0);
    applyBatch(ridges, root, initialBatch, true);

    setupRenderingLoop(app, window, ridges, root, worker, metrics, screenshotMode);
    setupMetrics(app, window, root, metrics, !screenshotMode);

    const auto result = app.exec();
    worker.stop();
    return result;
}
