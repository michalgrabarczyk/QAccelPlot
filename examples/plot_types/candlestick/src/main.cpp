//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "ExampleUtils.hpp"
#include "MarketFeed.hpp"

#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[])
{
    QLocale::setDefault(QLocale::English);
    QAccelPlotExample::configureGraphicsApi();

    QGuiApplication app(argc, argv);
    // Declared before the engine, so the QML that calls it is destroyed first.
    QAccelPlotExample::MarketFeed feed;
    QQmlApplicationEngine engine;
    QAccelPlotExample::setupEngineFailureHandler(app, engine);

    engine.setInitialProperties({{QStringLiteral("feed"), QVariant::fromValue(static_cast<QObject*>(&feed))}});
    engine.load(QUrl(QStringLiteral("qrc:/app/qml/main.qml")));

    QAccelPlotExample::setupScreenshotHandler(app, engine);
    return app.exec();
}
