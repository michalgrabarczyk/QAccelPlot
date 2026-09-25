//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExampleUtils.hpp"

#include <QAccelPlot/series/PointCloud.hpp>

#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>

#include <cmath>
#include <cstdlib>
#include <utility>
#include <vector>

namespace {

constexpr auto kPointCount = 200'000;

struct PointCloudData {
    std::vector<float> xy; // interleaved x0, y0, x1, y1, ...
    std::vector<float> values;
};

// Iterates the Clifford attractor. Each point's value is the length of the jump that reached it.
PointCloudData cliffordAttractor(const int pointCount)
{
    constexpr auto a = -1.7f;
    constexpr auto b = 1.8f;
    constexpr auto c = -1.9f;
    constexpr auto d = -0.4f;

    auto data = PointCloudData{};
    data.xy.reserve(static_cast<std::size_t>(pointCount) * 2);
    data.values.reserve(static_cast<std::size_t>(pointCount));
    auto x = 0.1f;
    auto y = 0.1f;
    for (auto i = 0; i < pointCount; ++i) {
        const auto nextX = std::sin(a * y) + c * std::cos(a * x);
        const auto nextY = std::sin(b * x) + d * std::cos(b * y);
        data.values.push_back(std::hypot(nextX - x, nextY - y));
        x = nextX;
        y = nextY;
        data.xy.push_back(x);
        data.xy.push_back(y);
    }
    return data;
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
    auto* root = engine.rootObjects().value(0);
    auto* cloud = root ? root->findChild<QAccelPlot::PointCloud*>(QStringLiteral("cloud")) : nullptr;
    if (!cloud) {
        qCritical() << "Unable to find the point cloud";
        return EXIT_FAILURE;
    }

    auto data = cliffordAttractor(kPointCount);
    cloud->setDataF(std::move(data.xy), std::move(data.values), kPointCount);

    QAccelPlotExample::setupScreenshotHandler(app, engine);
    return app.exec();
}
