//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExampleUtils.hpp"
#include "PointCloudGenerators.hpp"
#include "PointCloudStreamer.hpp"
#include <QAccelPlot/series/PointCloud.hpp>

using namespace QAccelPlot;

#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QLocale>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>

#include <cstdlib>
#include <memory>
#include <utility>

namespace {

constexpr auto kDefaultClusterPointCount = 250'000;
constexpr auto kShapeSeriesCount = 6;
constexpr auto kShapeColumns = 8;
constexpr auto kPowerLawPointCount = 4'000;

struct SceneState {
    int appliedPointCount{0};
    bool animating{false};
};

int requestedClusterPointCount(const QObject* root)
{
    const auto configured = root->property("pointCount").toInt();
    return configured > 0 ? configured : kDefaultClusterPointCount;
}

std::shared_ptr<const QAccelPlotExample::ClusterCloud> makeClusterSource(const int pointCount)
{
    return std::make_shared<const QAccelPlotExample::ClusterCloud>(QAccelPlotExample::generateClusterCloud(pointCount));
}

QAccelPlotExample::PointCloudData firstFrame(const QAccelPlotExample::ClusterCloud& source)
{
    auto frame = QAccelPlotExample::PointCloudData{};
    QAccelPlotExample::composeClusterFrame(source, 0.0f, frame);
    return frame;
}

// Repeater delegates are visual children only, so QObject::findChild() cannot see them.
PointCloud* findPointCloud(QQuickItem* item, const QString& objectName)
{
    if (!item) {
        return nullptr;
    }
    if (auto* cloud = qobject_cast<PointCloud*>(item); cloud && cloud->objectName() == objectName) {
        return cloud;
    }
    for (auto* child : item->childItems()) {
        if (auto* found = findPointCloud(child, objectName)) {
            return found;
        }
    }
    return nullptr;
}

bool populateStaticSeries(QQuickWindow* window, QObject* root)
{
    for (auto row = 0; row < kShapeSeriesCount; ++row) {
        auto* shapeCloud = findPointCloud(window->contentItem(), QStringLiteral("shape_%1").arg(row));
        if (!shapeCloud) {
            qCritical() << "Unable to find marker shape series:" << row;
            return false;
        }
        shapeCloud->setDataF(QAccelPlotExample::generateShapeRow(row, kShapeColumns), kShapeColumns);
    }

    auto* powerLaw = root->findChild<PointCloud*>(QStringLiteral("powerLawCloud"));
    if (!powerLaw) {
        qCritical() << "Unable to find power-law series";
        return false;
    }
    const auto totalCount = kPowerLawPointCount + QAccelPlotExample::kPowerLawInvalidPointCount;
    powerLaw->setDataF(QAccelPlotExample::generatePowerLawScatter(kPowerLawPointCount), totalCount);
    root->setProperty("powerLawValidCount", kPowerLawPointCount);
    return true;
}

void setupInteractiveUpdates(
    QGuiApplication& app, QQuickWindow* window, QObject* root, QAccelPlotExample::PointCloudStreamer& streamer, const std::shared_ptr<SceneState>& state)
{
    QObject::connect(window, &QQuickWindow::frameSwapped, &app, [&streamer]() { streamer.notifyFramePresented(); }, Qt::DirectConnection);

    // Poll QML-owned controls once per animation tick, like the other examples do.
    QObject::connect(window, &QQuickWindow::afterAnimating, &app, [root, &streamer, state]() {
        const auto pointCount = requestedClusterPointCount(root);
        if (pointCount != state->appliedPointCount) {
            // Generating a million-point cloud must not run on this thread: afterAnimating is
            // the GUI thread, and the stall would show up as dropped frames. The streamer
            // builds it and posts the first frame when it is ready.
            streamer.requestPointCount(pointCount);
            state->appliedPointCount = pointCount;
        }

        const auto animate = root->property("animate").toBool();
        if (animate != state->animating) {
            streamer.setRunning(animate);
            state->animating = animate;
        }
    });
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
    engine.load(QUrl(u"qrc:/app/qml/main.qml"_qs));

    auto* root = engine.rootObjects().value(0);
    auto* window = qobject_cast<QQuickWindow*>(root);
    if (!root || !window) {
        return EXIT_FAILURE;
    }

    auto* clusterCloud = root->findChild<PointCloud*>(QStringLiteral("clusterCloud"));
    if (!clusterCloud || !populateStaticSeries(window, root)) {
        qCritical() << "Point cloud example scene is incomplete";
        return EXIT_FAILURE;
    }

    // --animate starts streaming immediately; combined with --screenshot it smoke-tests postData().
    const auto animateRequested = app.arguments().contains(QStringLiteral("--animate"));
    if (screenshotMode && !animateRequested) {
        // Deterministic still frame: default count, phase 0, no streaming.
        root->setProperty("animate", false);
        auto frame = firstFrame(*makeClusterSource(requestedClusterPointCount(root)));
        clusterCloud->setDataF(std::move(frame.xy), std::move(frame.values), frame.pointCount);
        QAccelPlotExample::setupScreenshotHandler(app, engine);
        return app.exec();
    }

    auto streamer = QAccelPlotExample::PointCloudStreamer{clusterCloud};
    auto state = std::make_shared<SceneState>();
    setupInteractiveUpdates(app, window, root, streamer, state);
    if (animateRequested) {
        root->setProperty("animate", true);
    }
    if (screenshotMode) {
        QAccelPlotExample::setupScreenshotHandler(app, engine);
    }

    const auto result = app.exec();
    streamer.stop();
    return result;
}
