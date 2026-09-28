//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "DataDeliveryMetrics.hpp"
#include "ExampleUtils.hpp"
#include "ShowcaseController.hpp"

#include <QElapsedTimer>
#include <QGuiApplication>
#include <QLocale>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>

#include <atomic>
#include <memory>

using QAccelPlotExample::DataDeliveryMetrics;
using QAccelPlotExample::steadyNanoseconds;

namespace {

struct FrameMetrics {
    std::atomic<int> presentedFrames{0};
    std::atomic<qint64> accumulatedIntervalNanoseconds{0};
    std::atomic<int> intervalSamples{0};
    std::atomic<qint64> previousSwapNanoseconds{0};
};

void setupPerformanceMetrics(QGuiApplication& app, QQuickWindow* window, QObject* root, const std::shared_ptr<FrameMetrics>& metrics,
    const std::shared_ptr<DataDeliveryMetrics>& deliveryMetrics)
{
    QObject::connect(
        window, &QQuickWindow::frameSwapped, &app,
        [metrics]() {
            const auto nowNanoseconds = steadyNanoseconds();
            const auto previousNanoseconds = metrics->previousSwapNanoseconds.exchange(nowNanoseconds, std::memory_order_relaxed);

            metrics->presentedFrames.fetch_add(1, std::memory_order_relaxed);
            if (previousNanoseconds > 0) {
                metrics->accumulatedIntervalNanoseconds.fetch_add(nowNanoseconds - previousNanoseconds, std::memory_order_relaxed);
                metrics->intervalSamples.fetch_add(1, std::memory_order_relaxed);
            }
        },
        Qt::DirectConnection);

    auto reportElapsed = std::make_shared<QElapsedTimer>();
    reportElapsed->start();
    auto* reportTimer = new QTimer(&app);
    reportTimer->setInterval(1000);
    QObject::connect(reportTimer, &QTimer::timeout, &app, [root, metrics, deliveryMetrics, reportElapsed]() {
        const auto elapsedMilliseconds = reportElapsed->restart();
        const auto presentedFrames = metrics->presentedFrames.exchange(0, std::memory_order_relaxed);
        const auto accumulatedNanoseconds = metrics->accumulatedIntervalNanoseconds.exchange(0, std::memory_order_relaxed);
        const auto intervalSamples = metrics->intervalSamples.exchange(0, std::memory_order_relaxed);
        const auto deliverySnapshot = deliveryMetrics->takeSnapshot();

        const auto elapsedSeconds = elapsedMilliseconds > 0 ? elapsedMilliseconds / 1000.0 : 1.0;
        const auto displayFps = qRound(presentedFrames / elapsedSeconds);
        const auto dataUpdateRate = qRound(deliverySnapshot.appliedBatches / elapsedSeconds);
        const auto longestDataGapMs = deliverySnapshot.longestDataGapNanoseconds / 1'000'000.0;
        root->setProperty("fps", displayFps);
        root->setProperty("averageFrameTimeMs", intervalSamples > 0 ? accumulatedNanoseconds / (intervalSamples * 1'000'000.0) : 0.0);
        root->setProperty("updateRate", dataUpdateRate);
        root->setProperty("longestDataGapMs", longestDataGapMs);
    });
    reportTimer->start();
}

} // namespace

int main(int argc, char* argv[])
{
    QLocale::setDefault(QLocale::English);
    QAccelPlotExample::configureGraphicsApi();

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QAccelPlotExample::setupEngineFailureHandler(app, engine);

    const auto screenshotMode = app.arguments().contains(QStringLiteral("--screenshot"));
    const auto initialProperties = QVariantMap{{QStringLiteral("metricsEnabled"), !screenshotMode}};
    engine.setInitialProperties(initialProperties);
    engine.load(QUrl(u"qrc:/app/qml/main.qml"_qs));
    auto* root = engine.rootObjects().value(0);
    auto* window = qobject_cast<QQuickWindow*>(root);

    if (window) {
        auto deliveryMetrics = std::make_shared<DataDeliveryMetrics>();
        // Owned by the window, which stops the workers when it is destroyed.
        new QAccelPlotExample::ShowcaseController(window, root, deliveryMetrics, screenshotMode);
        if (!screenshotMode) {
            setupPerformanceMetrics(app, window, root, std::make_shared<FrameMetrics>(), deliveryMetrics);
        }
    }
    QAccelPlotExample::setupScreenshotHandler(app, engine);

    return app.exec();
}
