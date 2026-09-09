//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExampleUtils.hpp"
#include "SampleDataGenerators.hpp"
#include "series/LineCurve.hpp"

using namespace QAccelPlot;

#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QLocale>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <utility>

namespace {

void populateExampleData(QQmlApplicationEngine& engine)
{
    auto* root = engine.rootObjects().value(0);
    if (!root) {
        return;
    }

    const auto configuredPointCount = root->property("pointCount").toInt();
    const auto pointCount = configuredPointCount > 0 ? configuredPointCount : 1001;
    auto scenario = QAccelPlotExample::generateAnnotationScenarioData(pointCount);

    root->setProperty("peakX", scenario.peakX);
    root->setProperty("peakY", scenario.peakY);
    root->setProperty("valleyX", scenario.valleyX);
    root->setProperty("valleyY", scenario.valleyY);

    if (auto* curve = root->findChild<LineCurve*>("curve1")) {
        curve->setDataF(std::move(scenario.points), pointCount);
    }
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

    engine.load(QUrl(u"qrc:/app/qml/main.qml"_qs));

    populateExampleData(engine);
    QAccelPlotExample::setupScreenshotHandler(app, engine);

    return app.exec();
}
