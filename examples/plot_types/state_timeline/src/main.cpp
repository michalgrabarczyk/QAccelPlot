//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExampleUtils.hpp"

#include <QAccelPlot/series/RectangleList.hpp>

#include <QDateTime>
#include <QDebug>
#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <random>
#include <utility>
#include <vector>

namespace {

constexpr auto kMachineCount = 12;
constexpr auto kDayCount = 14;
constexpr auto kLaneHalfHeight = 0.4;
constexpr auto kMinuteMs = 60'000.0;

// Must match stateNames in main.qml.
enum State { Running, Idle, Setup, Alarm };

struct Timeline {
    std::vector<double> rects; // x1, y1, x2, y2 per state interval
    std::vector<int> states;
};

double stateMinutes(const State state, std::mt19937& rng)
{
    constexpr auto kMeanMinutes = std::array<double, 4>{45.0, 12.0, 20.0, 3.0};
    auto duration = std::exponential_distribution<double>{1.0 / kMeanMinutes[state]};
    return std::max(duration(rng), 0.5);
}

State nextState(const State current, std::mt19937& rng)
{
    if (current != Running) {
        return Running;
    }
    auto pick = std::discrete_distribution<int>{55, 25, 20};
    constexpr auto kAfterRunning = std::array<State, 3>{Idle, Setup, Alarm};
    return kAfterRunning[static_cast<std::size_t>(pick(rng))];
}

// Simulates each machine switching between states, one lane per machine.
Timeline generateTimeline(const double start, const double end)
{
    auto rng = std::mt19937{20260302};
    auto timeline = Timeline{};
    for (auto machine = 0; machine < kMachineCount; ++machine) {
        auto time = start;
        auto state = Running;
        while (time < end) {
            const auto stateEnd = std::min(time + stateMinutes(state, rng) * kMinuteMs, end);
            timeline.rects.insert(timeline.rects.end(), {time, machine - kLaneHalfHeight, stateEnd, machine + kLaneHalfHeight});
            timeline.states.push_back(state);
            time = stateEnd;
            state = nextState(state, rng);
        }
    }
    return timeline;
}

} // namespace

int main(int argc, char* argv[])
{
    QLocale::setDefault(QLocale::English);
    QAccelPlotExample::configureGraphicsApi();

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QAccelPlotExample::setupEngineFailureHandler(app, engine);

    const auto start = static_cast<double>(QDateTime{QDate{2026, 3, 2}, QTime{0, 0}}.toMSecsSinceEpoch());
    const auto end = start + kDayCount * 24 * 60 * kMinuteMs;
    engine.setInitialProperties({{QStringLiteral("timelineStart"), start}, {QStringLiteral("timelineEnd"), end}});
    engine.load(QUrl(u"qrc:/app/qml/main.qml"_qs));
    auto* root = engine.rootObjects().value(0);
    auto* states = root ? root->findChild<QAccelPlot::RectangleList*>(QStringLiteral("states")) : nullptr;
    if (!states) {
        qCritical() << "Unable to find the state timeline";
        return EXIT_FAILURE;
    }

    auto timeline = generateTimeline(start, end);
    const auto count = static_cast<int>(timeline.states.size());
    states->setData(std::move(timeline.rects), std::move(timeline.states), count);

    QAccelPlotExample::setupScreenshotHandler(app, engine);
    return app.exec();
}
