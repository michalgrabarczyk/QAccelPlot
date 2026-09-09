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

namespace {

void populateExampleData(QQmlApplicationEngine& engine)
{
    auto* root = engine.rootObjects().value(0);
    if (!root) {
        return;
    }

    const auto configuredPointCount = root->property("pointCount").toInt();
    const auto pointCount = configuredPointCount > 0 ? configuredPointCount : 1000;
    if (auto* curve = root->findChild<LineCurve*>("terrainProfile")) {
        curve->setDataF(QAccelPlotExample::generateTerrainProfile(pointCount), pointCount);
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
