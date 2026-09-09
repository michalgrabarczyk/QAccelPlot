//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExampleUtils.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QLocale>
#include <QObject>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[])
{
    qDebug() << "Application Started";
    QLocale::setDefault(QLocale::English);
    QAccelPlotExample::configureGraphicsApi();

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QAccelPlotExample::setupEngineFailureHandler(app, engine);
    engine.setInitialProperties({ { QStringLiteral("isDark"), !app.arguments().contains(QStringLiteral("--light-theme")) } });

    engine.load(QUrl(u"qrc:/app/qml/main.qml"_qs));

    QAccelPlotExample::setupScreenshotHandler(app, engine);

    return app.exec();
}
