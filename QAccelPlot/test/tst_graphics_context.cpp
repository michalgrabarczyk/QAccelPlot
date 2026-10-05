//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "OpenGlEsRequest.hpp"

#include <QMutex>
#include <QOpenGLContext>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

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
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    case QSGRendererInterface::Direct3D12:
        return QStringLiteral("d3d12");
#endif
    default:
        return QStringLiteral("unsupported-%1").arg(static_cast<int>(api));
    }
}

struct OpenGlContextInfo {
    bool isOpenGlEs = false;
    QVersionNumber version;
};

} // namespace

// Proves that the tests render with the backend the CI job asks for. A window that silently fell
// back to another backend, or to desktop OpenGL in an OpenGL ES job, would otherwise still pass.
class GraphicsContextTest : public QObject {
    Q_OBJECT

private slots:
    void windowUsesRequestedGraphicsContext();
};

void GraphicsContextTest::windowUsesRequestedGraphicsContext()
{
    const auto expectedApi = qEnvironmentVariable("QACCELPLOT_EXPECTED_GRAPHICS_API").trimmed().toLower();
    const auto openGlEsVersion = QAccelPlotTest::requestedOpenGlEsVersion();
    if (expectedApi.isEmpty() && !openGlEsVersion) {
        QSKIP("Neither QACCELPLOT_EXPECTED_GRAPHICS_API nor QACCELPLOT_OPENGL_ES_VERSION is set");
    }

    auto window = QQuickWindow{};
    window.resize(64, 64);
    auto mutex = QMutex{};
    auto context = std::optional<OpenGlContextInfo>{};
    // The context belongs to the render thread, so read it there.
    connect(
        &window, &QQuickWindow::beforeRendering, &window,
        [&window, &mutex, &context]() {
            const auto* openGlContext
                = static_cast<QOpenGLContext*>(window.rendererInterface()->getResource(&window, QSGRendererInterface::OpenGLContextResource));
            if (openGlContext) {
                const auto format = openGlContext->format();
                const auto locker = QMutexLocker{&mutex};
                context = OpenGlContextInfo{openGlContext->isOpenGLES(), QVersionNumber{format.majorVersion(), format.minorVersion()}};
            }
        },
        Qt::DirectConnection);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(!window.grabWindow().isNull());

    if (!expectedApi.isEmpty()) {
        QCOMPARE(graphicsApiName(window.rendererInterface()->graphicsApi()), expectedApi);
    }
    if (openGlEsVersion) {
        const auto locker = QMutexLocker{&mutex};
        QVERIFY2(context.has_value(), "The window did not render with an OpenGL context");
        QVERIFY2(context->isOpenGlEs, "The window rendered with desktop OpenGL instead of OpenGL ES");
        QVERIFY2(context->version >= *openGlEsVersion, qPrintable(QStringLiteral("OpenGL ES %1 is older than requested").arg(context->version.toString())));
    }
}

QTEST_MAIN(GraphicsContextTest)
#include "tst_graphics_context.moc"
