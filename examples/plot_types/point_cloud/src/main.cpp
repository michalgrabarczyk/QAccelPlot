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
#include <QObject>
#include <QQmlApplicationEngine>
#include <QVariant>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <utility>
#include <vector>

namespace {

constexpr auto kPointCount = 200'000;

struct PointCloudData {
    std::vector<float> xy; // interleaved x0, y0, x1, y1, ...
    std::vector<float> values;
};

struct CliffordParameters {
    float a;
    float b;
    float c;
    float d;
};

// An attractor spans |x| <= 1 + |c| and |y| <= 1 + |d|, so these fit the axes of main.qml.
constexpr auto kAttractors = std::array<CliffordParameters, 4>{{
    {-1.7f, 1.8f, -1.9f, -0.4f},
    {2.0f, 1.2f, -2.0f, 0.5f},
    {1.9f, 1.9f, 1.9f, 0.5f},
    {1.3f, 1.7f, 2.0f, 0.45f},
}};

// Iterates the Clifford attractor. Each point's value is its distance from the previous point.
PointCloudData cliffordAttractor(const CliffordParameters& parameters, const int pointCount)
{
    const auto [a, b, c, d] = parameters;

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

// Shows the attractors in the cloud, one after another.
class AttractorSource : public QObject {
    Q_OBJECT

public:
    explicit AttractorSource(QAccelPlot::PointCloud* cloud)
        : cloud_(cloud)
    {
        show();
    }

    // Shows the next attractor, animated by the cloud's transition. A morph moves every point to the point of the same index.
    Q_INVOKABLE void next()
    {
        index_ = (index_ + 1) % kAttractors.size();
        show();
    }

private:
    void show()
    {
        auto data = cliffordAttractor(kAttractors[index_], kPointCount);
        cloud_->setDataF(std::move(data.xy), std::move(data.values), kPointCount);
    }

    QAccelPlot::PointCloud* cloud_;
    std::size_t index_{0};
};

} // namespace

int main(int argc, char* argv[])
{
    QLocale::setDefault(QLocale::English);
    QAccelPlotExample::configureGraphicsApi();

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QAccelPlotExample::setupEngineFailureHandler(app, engine);

    engine.load(QUrl(QStringLiteral("qrc:/app/qml/main.qml")));
    auto* root = engine.rootObjects().value(0);
    auto* cloud = root ? root->findChild<QAccelPlot::PointCloud*>(QStringLiteral("cloud")) : nullptr;
    if (!cloud) {
        qCritical() << "Unable to find the point cloud";
        return EXIT_FAILURE;
    }

    auto attractors = AttractorSource{cloud};
    root->setProperty("attractors", QVariant::fromValue<QObject*>(&attractors));

    QAccelPlotExample::setupScreenshotHandler(app, engine);
    return app.exec();
}

#include "main.moc"
