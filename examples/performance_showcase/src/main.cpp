//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "DataDeliveryMetrics.hpp"
#include "DataGenerationWorker.hpp"
#include "ExampleUtils.hpp"
#include "series/LineCurve.hpp"
#include "shapes/RectangleList.hpp"

using namespace QAccelPlot;

#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QLocale>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>

namespace {

struct FrameMetrics {
    std::atomic<int> presentedFrames { 0 };
    std::atomic<qint64> accumulatedIntervalNanoseconds { 0 };
    std::atomic<int> intervalSamples { 0 };
    std::atomic<qint64> previousSwapNanoseconds { 0 };
};

std::int64_t steadyNanoseconds()
{
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
}

void setupPerformanceMetrics(QGuiApplication& app, QQuickWindow* window, QObject* root, const std::shared_ptr<FrameMetrics>& metrics,
    const std::shared_ptr<QAccelPlotExample::DataDeliveryMetrics>& deliveryMetrics, const bool metricsEnabled)
{
    if (!metricsEnabled) {
        return;
    }

    QObject::connect(
        window,
        &QQuickWindow::frameSwapped,
        &app,
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
        root->setProperty(
            "averageFrameTimeMs", intervalSamples > 0 ? accumulatedNanoseconds / (intervalSamples * 1'000'000.0) : 0.0);
        root->setProperty("updateRate", dataUpdateRate);
        root->setProperty("longestDataGapMs", longestDataGapMs);
    });
    reportTimer->start();
}

void setupCurveUpdates(
    QGuiApplication& app, QQuickWindow* window, QObject* root, LineCurve* curve, RectangleList* rectangleList, DataGenerationWorker& generator,
    const std::shared_ptr<QAccelPlotExample::DataDeliveryMetrics>& deliveryMetrics, const bool screenshotMode)
{
    auto elapsedTimer = std::make_shared<QElapsedTimer>();
    elapsedTimer->start();

    QObject::connect(window, &QQuickWindow::afterAnimating, &app,
        [root, curve, rectangleList, &generator, deliveryMetrics, elapsedTimer, screenshotMode]() {
        const auto rectanglesVisible = root->property("rectanglesVisible").toBool();
        generator.setRectangleTestMode(rectanglesVisible);
        const auto phase = screenshotMode ? 0.0 : elapsedTimer->elapsed() * 0.0012;
        generator.setPhase(phase);
        generator.setPointCount(root->property("pointCount").toInt());
        generator.setRectangleCount(root->property("rectangleCount").toInt());

        DataGenerationBatch batch;
        if (!generator.tryConsume(batch)) {
            return;
        }
        if (curve && !batch.curve1.empty()) {
            if (batch.curve1VertexCache.empty()) {
                curve->setDataFNoRange(std::move(batch.curve1), batch.pointCount);
            } else {
                curve->setDataFNoRangeWithCache(std::move(batch.curve1), batch.pointCount, std::move(batch.curve1VertexCache));
            }
            deliveryMetrics->dataApplied(steadyNanoseconds());
        }
        if (rectangleList) {
            rectangleList->setRawData(batch.rects.empty() ? nullptr : batch.rects.data(), batch.rectangleCount);
        }
    });
}

} // namespace

int main(int argc, char* argv[])
{
    qDebug() << "Application Started";
    QLocale::setDefault(QLocale::English);
    QAccelPlotExample::configureGraphicsApi();

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QAccelPlotExample::setupEngineFailureHandler(app, engine);

    const auto screenshotMode = app.arguments().contains(QStringLiteral("--screenshot"));
    const auto initialProperties = QVariantMap{ { QStringLiteral("metricsEnabled"), !screenshotMode } };
    engine.setInitialProperties(initialProperties);
    engine.load(QUrl(u"qrc:/app/qml/main.qml"_qs));
    auto* root = engine.rootObjects().value(0);
    auto* window = qobject_cast<QQuickWindow*>(root);
    auto* curve = root ? root->findChild<LineCurve*>(QStringLiteral("curve1")) : nullptr;
    auto* rectangleList = root ? root->findChild<RectangleList*>(QStringLiteral("rectangleList")) : nullptr;

    DataGenerationConfig generationConfig;
    DataGenerationWorker generator(generationConfig);
    generator.start();

    if (root && window) {
        auto frameMetrics = std::make_shared<FrameMetrics>();
        auto deliveryMetrics = std::make_shared<QAccelPlotExample::DataDeliveryMetrics>();
        setupCurveUpdates(app, window, root, curve, rectangleList, generator, deliveryMetrics, screenshotMode);
        setupPerformanceMetrics(app, window, root, frameMetrics, deliveryMetrics, !screenshotMode);
    }
    QAccelPlotExample::setupScreenshotHandler(app, engine);

    int ret = app.exec();

    generator.stop();

    return ret;
}
