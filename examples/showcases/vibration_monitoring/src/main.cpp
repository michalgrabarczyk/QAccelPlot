//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExampleUtils.hpp"
#include "MonitorController.hpp"

#include <QDebug>
#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QVariant>

#include <cstdlib>

namespace {

using QAccelPlotExample::MonitorController;

// One loop of the scripted demo; every signal component repeats after it too.
constexpr auto kDemoLoopSeconds = 8;
constexpr auto kDefaultRecordingFps = 25;
// Acquisition time shown by --screenshot.
constexpr auto kScreenshotSeconds = 2.0;

} // namespace

int main(int argc, char* argv[])
{
    QLocale::setDefault(QLocale::English);
    QAccelPlotExample::configureGraphicsApi();
    QAccelPlotExample::configureFrameRecording(argc, argv);

    QGuiApplication app(argc, argv);
    const auto arguments = QCoreApplication::arguments();

    auto error = QString{};
    const auto recordingDefaults = QAccelPlotExample::RecordingOptions{QString{}, kDemoLoopSeconds * kDefaultRecordingFps, kDefaultRecordingFps};
    const auto recording = QAccelPlotExample::recordingOptions(arguments, recordingDefaults, error);
    if (!error.isEmpty()) {
        qCritical().noquote() << error;
        return EXIT_FAILURE;
    }
    const auto screenshot = arguments.contains(QStringLiteral("--screenshot"));
    const auto clockMode = recording || screenshot ? MonitorController::ClockMode::Stepped : MonitorController::ClockMode::RealTime;
    const auto demo = recording.has_value() || arguments.contains(QStringLiteral("--demo"));

    if (recording) {
        QAccelPlotExample::installRecordingClock();
    }

    // Declared before the engine, so the window and its bindings go away while the monitor still exists.
    MonitorController monitor(clockMode, demo);
    QQmlApplicationEngine engine;
    QAccelPlotExample::setupEngineFailureHandler(app, engine);
    engine.setInitialProperties({
        {QStringLiteral("monitor"), QVariant::fromValue(&monitor)},
        {QStringLiteral("darkTheme"), !QAccelPlotExample::lightThemeRequested(arguments)},
    });
    engine.load(QUrl(QStringLiteral("qrc:/app/qml/main.qml")));

    auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().value(0));
    if (!window || !monitor.attach(window)) {
        return EXIT_FAILURE;
    }
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &monitor, &MonitorController::stop);

    if (recording) {
        const auto prepareFrame
            = [&monitor](int /*frameIndex*/, const double seconds, const std::function<void()>& ready) { monitor.showFrameAt(seconds, ready); };
        if (!QAccelPlotExample::startFrameRecording(app, window, *recording, prepareFrame)) {
            return EXIT_FAILURE;
        }
    } else if (screenshot) {
        monitor.showFrameAt(kScreenshotSeconds, [&app, &engine]() { QAccelPlotExample::setupScreenshotHandler(app, engine, 300); });
    }

    const auto result = QGuiApplication::exec();
    monitor.stop();
    return result;
}
