//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExampleUtils.hpp"
#include "RollingStats.hpp"

#include <QAccelPlot/series/BandSeries.hpp>
#include <QAccelPlot/series/LineCurve.hpp>

#include <QDebug>
#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>

#include <cstdlib>
#include <utility>

namespace {

constexpr auto kSampleCount = 10000;

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
    auto* samples = root ? root->findChild<QAccelPlot::LineCurve*>(QStringLiteral("rawSamples")) : nullptr;
    auto* mean = root ? root->findChild<QAccelPlot::LineCurve*>(QStringLiteral("rollingMean")) : nullptr;
    auto* band = root ? root->findChild<QAccelPlot::BandSeries*>(QStringLiteral("rollingBand")) : nullptr;
    if (!samples || !mean || !band) {
        qCritical() << "Unable to find the rolling statistics series";
        return EXIT_FAILURE;
    }

    auto stats = computeRollingStats(kSampleCount);
    samples->setDataF(std::move(stats.samples), kSampleCount);
    mean->setDataF(std::move(stats.mean), kSampleCount);
    band->setDataF(std::move(stats.band), kSampleCount);

    QAccelPlotExample::setupScreenshotHandler(app, engine);
    return app.exec();
}
