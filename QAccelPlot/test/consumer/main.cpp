//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Renders one plot and checks that the curve reached the image, which needs the
// public headers, the library, the QML module, and the embedded shaders.
//
#include <QAccelPlot/series/LineCurve.hpp>

#include <QGuiApplication>
#include <QImage>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QSGRendererInterface>

#include <cstdio>
#include <vector>

namespace {

// The diagonal of the 400 x 300 window is far longer; a missing curve leaves none.
constexpr auto kMinimumCurvePixels = 200;

int fail(const char* message)
{
    std::fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

int countMagentaPixels(const QImage& image)
{
    auto count = 0;
    for (auto y = 0; y < image.height(); ++y) {
        for (auto x = 0; x < image.width(); ++x) {
            const auto pixel = image.pixelColor(x, y);
            if (pixel.red() > 200 && pixel.green() < 60 && pixel.blue() > 200) {
                ++count;
            }
        }
    }
    return count;
}

} // namespace

int main(int argc, char* argv[])
{
    auto app = QGuiApplication{argc, argv};
    auto engine = QQmlApplicationEngine{};
    engine.load(QUrl(QStringLiteral("qrc:/app/Main.qml")));

    auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().value(0));
    if (!window) {
        return fail("Main.qml did not load");
    }
    auto* curve = window->findChild<QAccelPlot::LineCurve*>(QStringLiteral("curve"));
    if (!curve) {
        return fail("Main.qml contains no LineCurve");
    }
    curve->setDataF(std::vector<float>{0.0F, 0.0F, 10.0F, 10.0F}, 2);

    if (window->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        return fail("The software scene graph backend cannot render curves");
    }
    // The window stays hidden; grabWindow() renders it offscreen.
    const auto curvePixels = countMagentaPixels(window->grabWindow());
    if (curvePixels < kMinimumCurvePixels) {
        std::fprintf(stderr, "FAIL: The curve covers %d pixels; expected at least %d\n", curvePixels, kMinimumCurvePixels);
        return 1;
    }
    std::printf("PASS: The curve covers %d pixels\n", curvePixels);
    return 0;
}
