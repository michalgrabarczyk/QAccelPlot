//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExampleUtils.hpp"
#include "series/LineCurve.hpp"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

#include <cmath>
#include <vector>

namespace {

constexpr double windowDuration = 20.0;
constexpr double samplesPerSecond = 200.0;
constexpr int sampleCount = static_cast<int>(windowDuration * samplesPerSecond) + 1;
constexpr double pi = 3.14159265358979323846;

double vibrationAt(const double time)
{
    const auto amplitudeModulation = 1.0 + 0.12 * std::sin(2.0 * pi * 0.11 * time);
    const auto base = amplitudeModulation * 0.72 * std::sin(2.0 * pi * 0.9 * time);
    const auto harmonic = 0.16 * std::sin(2.0 * pi * 3.1 * time + 0.4);
    const auto cycles = time / 4.0;
    const auto phase = cycles - std::floor(cycles);
    const auto impactPosition = (phase - 0.82) / 0.045;
    const auto impact = 0.95 * std::exp(-(impactPosition * impactPosition));
    return base + harmonic + impact;
}

std::vector<float> rollingVibrationData(const double currentTime)
{
    std::vector<float> points;
    points.reserve(sampleCount * 2);

    // Anchor samples to a fixed time grid. Existing Y values therefore remain
    // unchanged between frames and the waveform translates horizontally instead
    // of being resampled at a different phase on every update.
    const auto latestSample = static_cast<long long>(std::floor(currentTime * samplesPerSecond));
    const auto firstSample = latestSample - (sampleCount - 1);
    for (int sample = 0; sample < sampleCount; ++sample) {
        const auto sampleTime = static_cast<double>(firstSample + sample) / samplesPerSecond;
        const auto relativeTime = sampleTime - currentTime;
        points.push_back(static_cast<float>(relativeTime));
        points.push_back(static_cast<float>(vibrationAt(sampleTime)));
    }

    return points;
}

} // namespace

int main(int argc, char* argv[])
{
    QLocale::setDefault(QLocale::English);
    QAccelPlotExample::configureGraphicsApi();

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QAccelPlotExample::setupEngineFailureHandler(app, engine);

    const auto realtimeEnabled = !app.arguments().contains(QStringLiteral("--screenshot"));
    engine.setInitialProperties({ { QStringLiteral("realtimeEnabled"), realtimeEnabled } });
    engine.load(QUrl(u"qrc:/app/qml/main.qml"_qs));
    auto* root = engine.rootObjects().value(0);
    auto* curve = root ? root->findChild<QAccelPlot::LineCurve*>(QStringLiteral("vibrationCurve")) : nullptr;

    if (curve) {
        curve->setDataF(rollingVibrationData(0.0), sampleCount);
    }

    const auto updateCurve = [root, curve]() {
        if (curve) {
            const auto currentTime = root->property("animationElapsedSeconds").toDouble();
            curve->setDataFNoRange(rollingVibrationData(currentTime), sampleCount);
        }
    };

    if (realtimeEnabled) {
        if (auto* window = qobject_cast<QQuickWindow*>(root)) {
            QObject::connect(window, &QQuickWindow::afterAnimating, &app, [root, updateCurve]() {
                if (root->property("updatesRunning").toBool()) {
                    updateCurve();
                }
            });
        }
    }

    QAccelPlotExample::setupScreenshotHandler(app, engine);
    return app.exec();
}
