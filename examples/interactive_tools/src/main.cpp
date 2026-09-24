//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExampleUtils.hpp"

#include <QAccelPlot/series/LineCurve.hpp>

#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

std::vector<float> terrainProfile(const int pointCount)
{
    constexpr auto distance = 1000.0f;
    auto data = std::vector<float>(static_cast<size_t>(pointCount) * 2);
    const auto xScale = distance / static_cast<float>(std::max(1, pointCount - 1));

    for (auto i = 0; i < pointCount; ++i) {
        const auto x = static_cast<float>(i) * xScale;
        const auto elevation = 28.0f + 20.0f * std::exp(-std::pow((x - 240.0f) / 120.0f, 2.0f)) - 11.0f * std::exp(-std::pow((x - 500.0f) / 75.0f, 2.0f))
            + 34.0f * std::exp(-std::pow((x - 720.0f) / 150.0f, 2.0f)) + 1.8f * std::sin(6.28318530718f * x / 95.0f);
        data[static_cast<size_t>(i) * 2] = x;
        data[static_cast<size_t>(i) * 2 + 1] = elevation;
    }

    return data;
}

void populateExampleData(QQmlApplicationEngine& engine)
{
    auto* root = engine.rootObjects().value(0);
    if (!root) {
        return;
    }

    const auto pointCount = root->property("pointCount").toInt();
    if (auto* curve = root->findChild<QAccelPlot::LineCurve*>(QStringLiteral("terrainProfile"))) {
        curve->setDataF(terrainProfile(pointCount), pointCount);
    }
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
    populateExampleData(engine);

    QAccelPlotExample::setupScreenshotHandler(app, engine);
    return app.exec();
}
