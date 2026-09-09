//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
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

    engine.load(QUrl(u"qrc:/app/qml/main.qml"_qs));
    QAccelPlotExample::setupScreenshotHandler(app, engine);
    return app.exec();
}
