//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "BenchmarkRunner.hpp"

#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

int main(int argc, char* argv[])
{
    QLocale::setDefault(QLocale::English);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    auto app = QGuiApplication{argc, argv};
    QCoreApplication::setApplicationName(QStringLiteral("QAccelPlot Benchmark"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    auto runner = QAccelPlot::BenchmarkRunner{};

    auto engine = QQmlApplicationEngine{};
    engine.rootContext()->setContextProperty(QStringLiteral("benchmarkRunner"), &runner);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [](QObject* object, const QUrl&) {
            if (!object) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(QUrl(u"qrc:/app/qml/main.qml"_qs));

    return app.exec();
}
