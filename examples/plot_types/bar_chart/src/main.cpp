//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "ExampleUtils.hpp"

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

    engine.load(QUrl(QStringLiteral("qrc:/app/qml/main.qml")));

    QAccelPlotExample::setupScreenshotHandler(app, engine);
    return QGuiApplication::exec();
}
