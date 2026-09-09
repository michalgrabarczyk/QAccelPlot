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

#include <cmath>
#include <vector>

namespace {

std::vector<float> dailyTemperatureData()
{
    constexpr int sampleCount = 97;
    constexpr double pi = 3.14159265358979323846;
    std::vector<float> points;
    points.reserve(sampleCount * 2);

    for (int sample = 0; sample < sampleCount; ++sample) {
        const auto hour = 24.0 * sample / (sampleCount - 1);
        const auto temperature = 21.5 + 4.0 * std::sin(2.0 * pi * (hour - 8.0) / 24.0)
            + 0.35 * std::sin(2.0 * pi * hour / 3.0);
        points.push_back(static_cast<float>(hour));
        points.push_back(static_cast<float>(temperature));
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

    engine.load(QUrl(u"qrc:/app/qml/main.qml"_qs));
    if (auto* root = engine.rootObjects().value(0)) {
        if (auto* curve = root->findChild<QAccelPlot::LineCurve*>(QStringLiteral("temperatureCurve"))) {
            curve->setDataF(dailyTemperatureData(), 97);
        }
    }

    QAccelPlotExample::setupScreenshotHandler(app, engine);
    return app.exec();
}
