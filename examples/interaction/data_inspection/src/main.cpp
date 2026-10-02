//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExampleUtils.hpp"

#include <QAccelPlot/series/LineCurve.hpp>

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <cmath>
#include <cstdint>
#include <vector>

namespace {

// Uniform noise in [-amplitude, amplitude] from a fixed sequence, so every run draws the same data.
float noise(std::uint32_t& state, const float amplitude)
{
    state = state * 1664525U + 1013904223U;
    return (static_cast<float>(state >> 8U) / static_cast<float>(1U << 24U) * 2.0f - 1.0f) * amplitude;
}

void populate(QObject& root)
{
    constexpr auto count = 500000;
    auto state = std::uint32_t{1};
    for (auto signal = 0; signal < 2; ++signal) {
        auto data = std::vector<float>(static_cast<std::size_t>(count) * 2);
        for (auto i = 0; i < count; ++i) {
            const auto x = static_cast<float>(i) / 10000;
            const auto base = static_cast<std::size_t>(i) * 2;
            data[base] = x;
            // One pixel column holds about 500 samples, so the noise draws as a band.
            data[base + 1] = std::sin(x * static_cast<float>(signal + 1)) + static_cast<float>(signal) + noise(state, 0.2f);
        }
        // A one-sample spike: only the value range under the cursor reveals it.
        data[static_cast<std::size_t>(count / 2) * 2 + 1] = 4.5f;
        const auto name = signal == 0 ? QStringLiteral("signalA") : QStringLiteral("signalB");
        if (auto* curve = root.findChild<QAccelPlot::LineCurve*>(name)) {
            curve->setDataF(std::move(data), count);
        }
    }
}

} // namespace

int main(int argc, char* argv[])
{
    QAccelPlotExample::configureGraphicsApi();
    auto app = QGuiApplication{argc, argv};
    auto engine = QQmlApplicationEngine{};
    QAccelPlotExample::setupEngineFailureHandler(app, engine);
    engine.load(QUrl{QStringLiteral("qrc:/app/qml/main.qml")});
    if (auto* root = engine.rootObjects().value(0)) {
        populate(*root);
        // A screenshot must not depend on where the mouse happens to be.
        root->setProperty("pinnedCursor", app.arguments().contains(QStringLiteral("--screenshot")));
    }
    QAccelPlotExample::setupScreenshotHandler(app, engine);
    return app.exec();
}
