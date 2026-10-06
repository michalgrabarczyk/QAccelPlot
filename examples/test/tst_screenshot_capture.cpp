//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExampleUtils.hpp"

#include <QFile>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest/QtTest>

#include <cstdlib>

namespace {

QProcessEnvironment captureEnvironment(const QString& renderLoop)
{
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("QSG_RENDER_LOOP"), renderLoop);
    environment.insert(QStringLiteral("QSG_RHI_BACKEND"), QStringLiteral("opengl"));
    environment.insert(QStringLiteral("QACCELPLOT_EXPECTED_GRAPHICS_API"), QStringLiteral("opengl"));
    environment.insert(QStringLiteral("QT_SCALE_FACTOR"), QStringLiteral("1"));
    environment.insert(QStringLiteral("QT_ENABLE_HIGHDPI_SCALING"), QStringLiteral("0"));
    environment.insert(QStringLiteral("QT_FORCE_STDERR_LOGGING"), QStringLiteral("1"));
    environment.remove(QStringLiteral("QT_QUICK_BACKEND"));
    return environment;
}

void destroyBeforeDelivery(QQuickWindow* window)
{
    // Install before the screenshot's render callback so deletion is queued ahead of its GUI continuation.
    QTimer::singleShot(0, window, [window]() {
        QObject::connect(
            window, &QQuickWindow::beforeRendering, window,
            [window]() { QMetaObject::invokeMethod(window, [window]() { delete window; }, Qt::QueuedConnection); },
            static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::SingleShotConnection));
    });
}

int runCapture(QGuiApplication& app)
{
    app.setQuitOnLastWindowClosed(false);
    auto engine = QQmlApplicationEngine{};
    engine.loadData(R"(
        import QtQuick
        import QtQuick.Window
        Window {
            width: 128; height: 96; visible: true
            Rectangle { anchors.fill: parent; color: "#123456" }
        }
    )");
    auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().value(0));
    if (!window) {
        return EXIT_FAILURE;
    }

    const auto beforeDelay = app.arguments().contains(QStringLiteral("before-delay"));
    const auto beforeDelivery = app.arguments().contains(QStringLiteral("before-delivery"));
    if (beforeDelivery) {
        destroyBeforeDelivery(window);
    }
    QAccelPlotExample::setupScreenshotHandler(app, engine, beforeDelay ? 100 : 0);
    if (beforeDelay) {
        delete window;
    }
    if (beforeDelay || beforeDelivery) {
        QTimer::singleShot(300, &app, &QCoreApplication::quit);
    }
    QTimer::singleShot(8000, &app, [&app]() { app.exit(EXIT_FAILURE); });
    return app.exec();
}

} // namespace

class ScreenshotCaptureTest : public QObject {
    Q_OBJECT

private slots:
    void capture_data();
    void capture();
};

void ScreenshotCaptureTest::capture_data()
{
    QTest::addColumn<QString>("renderLoop");
    QTest::addColumn<QString>("mode");
    for (const auto* renderLoop : {"basic", "threaded"}) {
        for (const auto* mode : {"capture", "before-delay", "before-delivery"}) {
            const auto name = QStringLiteral("%1-%2").arg(QLatin1String(renderLoop), QLatin1String(mode));
            QTest::newRow(qPrintable(name)) << QString::fromLatin1(renderLoop) << QString::fromLatin1(mode);
        }
    }
}

void ScreenshotCaptureTest::capture()
{
    QFETCH(QString, renderLoop);
    QFETCH(QString, mode);
#ifdef Q_OS_MACOS
    if (renderLoop == QStringLiteral("threaded")) {
        QSKIP("Qt does not support the threaded render loop with OpenGL on macOS");
    }
#endif
    auto directory = QTemporaryDir{};
    QVERIFY(directory.isValid());
    const auto path = directory.filePath(QStringLiteral("capture.png"));
    auto process = QProcess{};
    process.setProcessEnvironment(captureEnvironment(renderLoop));
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(QCoreApplication::applicationFilePath(), {QStringLiteral("--capture"), QStringLiteral("--screenshot"), path, mode});
    const auto finished = QTest::qWaitFor([&process]() { return process.state() == QProcess::NotRunning; }, 15000);
    if (!finished) {
        process.kill();
        process.waitForFinished();
    }
    const auto output
        = QStringLiteral("Exit %1: %2\n%3").arg(process.exitCode()).arg(process.errorString(), QString::fromLocal8Bit(process.readAll())).toLocal8Bit();
    QVERIFY2(finished, output.constData());
    QVERIFY2(process.exitStatus() == QProcess::NormalExit, output.constData());
    QVERIFY2(process.exitCode() == EXIT_SUCCESS, output.constData());
    if (mode != QStringLiteral("capture")) {
        QVERIFY(!QFile::exists(path));
        QVERIFY(!QFile::exists(path + QStringLiteral(".rhi.json")));
        return;
    }

    const auto image = QImage{path};
    QCOMPARE(image.size(), QSize(128, 96));
    QCOMPARE(image.pixelColor(64, 48), QColor(QStringLiteral("#123456")));
    auto metadataFile = QFile{path + QStringLiteral(".rhi.json")};
    QVERIFY(metadataFile.open(QIODevice::ReadOnly));
    const auto metadata = QJsonDocument::fromJson(metadataFile.readAll()).object();
    QCOMPARE(metadata.value(QStringLiteral("actual_graphics_api")).toString(), QStringLiteral("opengl"));
    QVERIFY(metadata.value(QStringLiteral("opengl_context_observed")).toBool());
    QVERIFY(metadata.value(QStringLiteral("opengl_major_version")).toInt() >= 2);
}

int main(int argc, char* argv[])
{
    QAccelPlotExample::configureGraphicsApi();
    auto app = QGuiApplication{argc, argv};
    if (app.arguments().contains(QStringLiteral("--capture"))) {
        return runCapture(app);
    }
    auto test = ScreenshotCaptureTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_screenshot_capture.moc"
