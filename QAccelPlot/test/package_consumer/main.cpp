//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>

namespace {

constexpr auto minimumCurvePixels = 100;

QString screenshotPath(const QStringList& arguments)
{
    const auto optionIndex = arguments.indexOf(QStringLiteral("--screenshot"));
    if (optionIndex >= 0 && optionIndex + 1 < arguments.size()) {
        return arguments.at(optionIndex + 1);
    }
    return QStringLiteral("package-consumer.png");
}

bool containsRenderedCurve(const QImage& image)
{
    auto curvePixels = 0;
    for (auto y = 0; y < image.height(); ++y) {
        for (auto x = 0; x < image.width(); ++x) {
            const auto color = image.pixelColor(x, y);
            if (color.red() >= 180 && color.red() >= color.green() + 60 && color.red() >= color.blue() + 60) {
                ++curvePixels;
            }
        }
    }
    return curvePixels >= minimumCurvePixels;
}

bool saveScreenshot(const QImage& image, const QString& path)
{
    const auto directory = QFileInfo(path).absoluteDir();
    return directory.mkpath(QStringLiteral(".")) && image.save(path);
}

} // namespace

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/package-consumer/Main.qml")));

    if (engine.rootObjects().isEmpty()) {
        qCritical("The installed QAccelPlot QML module could not be loaded.");
        return 2;
    }

    auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().constFirst());
    if (window == nullptr) {
        qCritical("The package-consumer root object is not a QQuickWindow.");
        return 3;
    }

    QObject::connect(window, &QQuickWindow::sceneGraphError, &app, [&app](QQuickWindow::SceneGraphError, const QString& message) {
        qCritical().noquote() << "Qt Quick scene graph error:" << message;
        app.exit(4);
    });

    auto captureScheduled = false;
    QObject::connect(window, &QQuickWindow::frameSwapped, &app, [&]() {
        if (captureScheduled) {
            return;
        }
        captureScheduled = true;
        QTimer::singleShot(100, &app, [&]() {
            const auto image = window->grabWindow();
            const auto path = screenshotPath(app.arguments());
            if (image.isNull()) {
                qCritical("The first rendered frame could not be captured.");
                app.exit(5);
            } else if (!saveScreenshot(image, path)) {
                qCritical().noquote() << "The rendered frame could not be saved to" << path;
                app.exit(6);
            } else if (!containsRenderedCurve(image)) {
                qCritical().noquote() << "The rendered frame does not contain the expected curve. Screenshot:" << path;
                app.exit(7);
            } else {
                qInfo().noquote() << "Installed package rendered its QML curve successfully. Screenshot:" << path;
                app.exit(0);
            }
        });
    });

    QTimer::singleShot(10000, &app, [&app]() {
        qCritical("Timed out waiting for a rendered frame.");
        app.exit(8);
    });
    window->requestUpdate();
    return app.exec();
}
