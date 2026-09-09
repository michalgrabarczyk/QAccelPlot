//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExampleUtils.hpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QPointer>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickPaintedItem>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSaveFile>
#include <QTimer>
#include <QtMath>

#include <cstdlib>

namespace QAccelPlotExample {

namespace {

QString graphicsApiName(const QSGRendererInterface::GraphicsApi api)
{
    switch (api) {
    case QSGRendererInterface::Software:
        return QStringLiteral("software");
    case QSGRendererInterface::OpenGL:
        return QStringLiteral("opengl");
    case QSGRendererInterface::Direct3D11:
        return QStringLiteral("d3d11");
    case QSGRendererInterface::Vulkan:
        return QStringLiteral("vulkan");
    case QSGRendererInterface::Metal:
        return QStringLiteral("metal");
    case QSGRendererInterface::Null:
        return QStringLiteral("null");
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    case QSGRendererInterface::Direct3D12:
        return QStringLiteral("d3d12");
#endif
    case QSGRendererInterface::Unknown:
        return QStringLiteral("unknown");
    default:
        return QStringLiteral("unsupported-%1").arg(static_cast<int>(api));
    }
}

QString requestedGraphicsApiName()
{
    auto requested = qEnvironmentVariable("QACCELPLOT_EXPECTED_GRAPHICS_API").trimmed().toLower();
    if (requested.isEmpty()) {
        requested = qEnvironmentVariable("QSG_RHI_BACKEND").trimmed().toLower();
    }
    if (requested.isEmpty() && qEnvironmentVariable("QT_QUICK_BACKEND").compare(QStringLiteral("software"), Qt::CaseInsensitive) == 0) {
        requested = QStringLiteral("software");
    }
    return requested.isEmpty() ? QStringLiteral("opengl") : requested;
}

struct PaintedItemTextureState {
    QPointer<QQuickPaintedItem> item;
    QSize textureSize;
};

void collectPaintedItems(QQuickItem* root, QList<QQuickPaintedItem*>& paintedItems)
{
    if (auto* paintedItem = qobject_cast<QQuickPaintedItem*>(root)) {
        paintedItems.append(paintedItem);
    }
    for (auto* child : root->childItems()) {
        collectPaintedItems(child, paintedItems);
    }
}

QList<PaintedItemTextureState> preparePaintedItemsForCapture(QQuickItem* root, const qreal effectiveDevicePixelRatio)
{
    auto paintedItems = QList<QQuickPaintedItem*>{};
    collectPaintedItems(root, paintedItems);

    auto states = QList<PaintedItemTextureState>{};
    states.reserve(paintedItems.size());
    for (auto* item : paintedItems) {
        if (item->width() <= 0.0 || item->height() <= 0.0) {
            continue;
        }

        states.append({item, item->textureSize()});

        // QQuickPaintedItem multiplies an explicit textureSize by the window's
        // effective DPR. Compensate for a scaled-down CI window so its backing
        // texture still contains one physical pixel per final screenshot pixel.
        const QSize captureTextureSize(
            qCeil(item->width() / effectiveDevicePixelRatio), qCeil(item->height() / effectiveDevicePixelRatio));
        item->setTextureSize(captureTextureSize);
        item->update();
    }
    return states;
}

void restorePaintedItemTextures(const QList<PaintedItemTextureState>& states)
{
    for (const auto& state : states) {
        if (state.item) {
            state.item->setTextureSize(state.textureSize);
            state.item->update();
        }
    }
}

} // namespace

void configureGraphicsApi()
{
    if (qEnvironmentVariableIsEmpty("QSG_RHI_BACKEND") && qEnvironmentVariableIsEmpty("QT_QUICK_BACKEND")) {
        QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    }
}

void setupEngineFailureHandler(QGuiApplication& app, QQmlApplicationEngine& engine)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
#else
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [](QObject* object, const QUrl&) {
            if (!object) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);
#endif
}

void setupScreenshotHandler(QGuiApplication& app, QQmlApplicationEngine& engine, int delayMs)
{
    const auto arguments = app.arguments();
    const auto screenshotIndex = arguments.indexOf("--screenshot");
    if (screenshotIndex == -1) {
        return;
    }

    const auto path = screenshotIndex + 1 < arguments.size() ? arguments.at(screenshotIndex + 1) : QStringLiteral("screenshot.png");
    auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().value(0));
    if (!window) {
        qCritical().noquote() << "Unable to capture screenshot: the QML root object is not a window.";
        app.exit(EXIT_FAILURE);
        return;
    }

    QObject::connect(window, &QQuickWindow::sceneGraphError, &app, [&app](const QQuickWindow::SceneGraphError error, const QString& message) {
        qCritical().noquote() << QStringLiteral("Qt Quick scene graph error %1: %2").arg(static_cast<int>(error)).arg(message);
        app.exit(EXIT_FAILURE);
    });

    QTimer::singleShot(delayMs, &app, [&app, window, path]() {
        auto fail = [&app](const QString& message) {
            qCritical().noquote() << message;
            app.exit(EXIT_FAILURE);
        };

        const auto requestedApi = requestedGraphicsApiName();
        const auto actualApi = graphicsApiName(window->rendererInterface()->graphicsApi());
        qInfo().noquote() << QStringLiteral("QAccelPlot graphics API: requested=%1 actual=%2").arg(requestedApi, actualApi);
        if (actualApi != requestedApi) {
            fail(QStringLiteral("Requested graphics API '%1', but Qt Quick initialized '%2'.").arg(requestedApi, actualApi));
            return;
        }
        const auto expectedQtVersion = qEnvironmentVariable("QACCELPLOT_EXPECTED_QT_VERSION").trimmed();
        const auto actualQtVersion = QString::fromLatin1(qVersion());
        if (!expectedQtVersion.isEmpty() && actualQtVersion != expectedQtVersion) {
            fail(QStringLiteral("Expected Qt %1, but the example is running with Qt %2.").arg(expectedQtVersion, actualQtVersion));
            return;
        }

        const QFileInfo outputInfo(path);
        auto outputDirectory = outputInfo.absoluteDir();
        if (!outputDirectory.exists() && !outputDirectory.mkpath(QStringLiteral("."))) {
            fail(QStringLiteral("Unable to create screenshot directory: %1").arg(outputDirectory.absolutePath()));
            return;
        }

        // The CI desktop may use QT_SCALE_FACTOR < 1 so that every example window
        // fits on screen. grabWindow() would then return a reduced framebuffer. An
        // item grab renders into an offscreen target of the requested pixel size,
        // preserving text and geometry detail while using the window's active RHI.
        const auto logicalWindowSize = window->size();
        const auto effectiveDevicePixelRatio = window->effectiveDevicePixelRatio();
        if (effectiveDevicePixelRatio <= 0.0) {
            fail(QStringLiteral("Unable to capture screenshot: the window has an invalid device pixel ratio."));
            return;
        }

        // Qt 6.2 treats grabToImage(targetSize) as a physical pixel size. Since
        // Qt 6.3 it treats targetSize as device-independent and multiplies it
        // by QQuickWindow::effectiveDevicePixelRatio(). Match those documented
        // implementations directly so every capture is a single render pass.
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
        constexpr bool grabTargetScaledByDevicePixelRatio = false;
        const auto offscreenTargetSize = logicalWindowSize;
#else
        constexpr bool grabTargetScaledByDevicePixelRatio = true;
        const QSize offscreenTargetSize(qRound(logicalWindowSize.width() / effectiveDevicePixelRatio),
            qRound(logicalWindowSize.height() / effectiveDevicePixelRatio));
#endif

        auto* contentItem = window->contentItem();
        if (!contentItem) {
            fail(QStringLiteral("Unable to capture screenshot: the window has no content item."));
            return;
        }

        const auto paintedItemTextureStates = preparePaintedItemsForCapture(contentItem, effectiveDevicePixelRatio);
        qInfo().noquote() << QStringLiteral("Prepared %1 painted items for native-resolution capture.").arg(paintedItemTextureStates.size());

        auto grabResult = contentItem->grabToImage(offscreenTargetSize);
        if (!grabResult) {
            restorePaintedItemTextures(paintedItemTextureStates);
            fail(QStringLiteral("Unable to start an offscreen screenshot capture."));
            return;
        }

        const auto windowColor = window->color();
        QObject::connect(
            grabResult.data(), &QQuickItemGrabResult::ready, &app,
            [&app, grabResult, path, requestedApi, actualApi, actualQtVersion, logicalWindowSize, offscreenTargetSize, effectiveDevicePixelRatio,
                grabTargetScaledByDevicePixelRatio, windowColor, paintedItemTextureStates]() {
                auto failCapture = [&app](const QString& message) {
                    qCritical().noquote() << message;
                    app.exit(EXIT_FAILURE);
                };

                auto grabbedImage = grabResult->image();
                restorePaintedItemTextures(paintedItemTextureStates);
                if (grabbedImage.isNull()) {
                    failCapture(QStringLiteral("Offscreen screenshot capture returned an empty image."));
                    return;
                }

                const auto capturedPixelSize = grabbedImage.size();
                if (capturedPixelSize != logicalWindowSize) {
                    failCapture(QStringLiteral("Offscreen screenshot has %1x%2 pixels; expected %3x%4. Refusing to resize the capture.")
                            .arg(capturedPixelSize.width())
                            .arg(capturedPixelSize.height())
                            .arg(logicalWindowSize.width())
                            .arg(logicalWindowSize.height()));
                    return;
                }

                // grabToImage() captures the content item, while QQuickWindow::color
                // is the scene's clear color. Composite the two to match grabWindow().
                grabbedImage.setDevicePixelRatio(1.0);
                QImage image(logicalWindowSize, QImage::Format_RGBA8888_Premultiplied);
                image.fill(windowColor);
                {
                    QPainter painter(&image);
                    painter.drawImage(QPoint(0, 0), grabbedImage);
                }
                image.setDevicePixelRatio(1.0);

                if (!image.save(path)) {
                    failCapture(QStringLiteral("Unable to save screenshot to: %1").arg(path));
                    return;
                }

                const auto metadataPath = path + QStringLiteral(".rhi.json");
                const auto metadata = QJsonDocument(QJsonObject{
                                                        {QStringLiteral("version"), 3},
                                                        {QStringLiteral("capture_method"), QStringLiteral("item_grab_to_image")},
                                                        {QStringLiteral("grab_target_scaled_by_device_pixel_ratio"), grabTargetScaledByDevicePixelRatio},
                                                        {QStringLiteral("requested_graphics_api"), requestedApi},
                                                        {QStringLiteral("actual_graphics_api"), actualApi},
                                                        {QStringLiteral("qt_version"), actualQtVersion},
                                                        {QStringLiteral("logical_window_width"), logicalWindowSize.width()},
                                                        {QStringLiteral("logical_window_height"), logicalWindowSize.height()},
                                                        {QStringLiteral("offscreen_target_width"), offscreenTargetSize.width()},
                                                        {QStringLiteral("offscreen_target_height"), offscreenTargetSize.height()},
                                                        {QStringLiteral("effective_device_pixel_ratio"), effectiveDevicePixelRatio},
                                                        {QStringLiteral("captured_pixel_width"), capturedPixelSize.width()},
                                                        {QStringLiteral("captured_pixel_height"), capturedPixelSize.height()},
                                                    })
                                          .toJson(QJsonDocument::Indented);
                QSaveFile metadataFile(metadataPath);
                if (!metadataFile.open(QIODevice::WriteOnly) || metadataFile.write(metadata) != metadata.size() || !metadataFile.commit()) {
                    failCapture(QStringLiteral("Unable to save graphics metadata to: %1").arg(metadataPath));
                    return;
                }

                qDebug() << "Screenshot saved to" << path;
                qDebug() << "Graphics metadata saved to" << metadataPath;
                app.quit();
            },
            Qt::SingleShotConnection);
    });
}

} // namespace QAccelPlotExample
