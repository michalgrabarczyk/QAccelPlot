//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "ExampleUtils.hpp"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[])
{
    QLocale::setDefault(QLocale::English);
    QAccelPlotExample::configureGraphicsApi();

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QAccelPlotExample::setupEngineFailureHandler(app, engine);
    engine.setInitialProperties({{QStringLiteral("isDark"), !app.arguments().contains(QStringLiteral("--light-theme"))}});

    engine.load(QUrl(QStringLiteral("qrc:/app/qml/main.qml")));

    QAccelPlotExample::setupScreenshotHandler(app, engine);
    return app.exec();
}
