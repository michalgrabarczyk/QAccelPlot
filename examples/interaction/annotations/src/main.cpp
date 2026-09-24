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
#include <limits>
#include <utility>
#include <vector>

namespace {

struct PressureResponse {
    std::vector<float> points;
    float peakX{0.0f};
    float peakY{0.0f};
    float valleyX{0.0f};
    float valleyY{0.0f};
};

PressureResponse pressureResponse(const int pointCount)
{
    auto response = PressureResponse{};
    response.points.resize(static_cast<size_t>(pointCount) * 2);
    response.peakY = std::numeric_limits<float>::lowest();
    response.valleyY = std::numeric_limits<float>::max();
    const auto xScale = 100.0f / static_cast<float>(std::max(1, pointCount - 1));

    for (auto i = 0; i < pointCount; ++i) {
        const auto x = static_cast<float>(i) * xScale;
        const auto y = 0.25f * std::sin(0.35f * x) + 6.2f * std::exp(-std::pow((x - 24.0f) / 4.0f, 2.0f)) - 4.8f * std::exp(-std::pow((x - 58.0f) / 5.0f, 2.0f))
            + 3.0f * std::exp(-std::pow((x - 79.0f) / 7.0f, 2.0f));
        response.points[static_cast<size_t>(i) * 2] = x;
        response.points[static_cast<size_t>(i) * 2 + 1] = y;

        if (y > response.peakY) {
            response.peakX = x;
            response.peakY = y;
        }
        if (y < response.valleyY) {
            response.valleyX = x;
            response.valleyY = y;
        }
    }

    return response;
}

void populateExampleData(QQmlApplicationEngine& engine)
{
    auto* root = engine.rootObjects().value(0);
    if (!root) {
        return;
    }

    const auto pointCount = root->property("pointCount").toInt();
    auto response = pressureResponse(pointCount);

    root->setProperty("peakX", response.peakX);
    root->setProperty("peakY", response.peakY);
    root->setProperty("valleyX", response.valleyX);
    root->setProperty("valleyY", response.valleyY);

    if (auto* curve = root->findChild<QAccelPlot::LineCurve*>(QStringLiteral("curve1"))) {
        curve->setDataF(std::move(response.points), pointCount);
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
