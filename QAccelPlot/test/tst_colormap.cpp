//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/effects/GradientFill.hpp"
#include "QAccelPlot/effects/GradientStroke.hpp"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QtTest/QtTest>

namespace QAccelPlot {

namespace {

/// Instantiates \a qml and returns its root, or nullptr after printing the QML errors.
QObject* build(QQmlEngine& engine, QQmlComponent& component, const char* qml)
{
    component.setData(QByteArray{qml}, QUrl{});
    auto* root = component.create(engine.rootContext());
    if (!root) {
        for (const auto& error : component.errors()) {
            qWarning() << error.toString();
        }
    }
    return root;
}

template <typename Effect> void verifyEffectFollowsColormap()
{
    auto effect = Effect{};
    auto* colormap = new Colormap{};
    effect.setColormap(colormap);
    QCOMPARE(effect.payload().stops.front().color, QColor{"#440154"});

    auto effectSpy = QSignalSpy{&effect, &Effect::effectChanged};
    colormap->setPreset(Colormap::Preset::Grayscale);
    QCOMPARE(effectSpy.count(), 1);
    QCOMPARE(effect.payload().stops.front().color, QColor{Qt::black});

    auto colormapSpy = QSignalSpy{&effect, &Effect::colormapChanged};
    delete colormap;
    QCOMPARE(effect.colormap(), nullptr);
    QCOMPARE(colormapSpy.count(), 1);
    QVERIFY(effect.payload().stops.empty());
}

} // namespace

class ColormapTest : public QObject {
    Q_OBJECT

private slots:
    void defaultsToViridisRamp();
    void presetsDifferFromEachOther();
    void presetsSpanReferenceColors_data();
    void presetsSpanReferenceColors();
    void reversedFlipsPresetRamp();
    void qmlReversedFlipsCustomStops();
    void boundsAreUnsetUntilAssigned();
    void qmlStopsOverridePreset();
    void qmlGradientIsAccepted();
    void gradientFillFollowsColormap();
    void gradientStrokeFollowsColormap();
    void qmlEffectColormapOverridesGradient();
};

void ColormapTest::defaultsToViridisRamp()
{
    const auto colormap = Colormap{};
    const auto& stops = colormap.resolvedStops();

    QCOMPARE(colormap.preset(), Colormap::Preset::Viridis);
    QCOMPARE(colormap.norm(), Colormap::Normalization::Linear);
    QVERIFY(stops.size() >= 2);
    // The ramp always spans the full coordinate range.
    QCOMPARE(stops.front().position, 0.0f);
    QCOMPARE(stops.back().position, 1.0f);
    // Viridis runs dark purple to yellow.
    QCOMPARE(stops.front().color, QColor{"#440154"});
    QCOMPARE(stops.back().color, QColor{"#fde725"});
}

void ColormapTest::presetsDifferFromEachOther()
{
    auto colormap = Colormap{};
    const auto viridis = colormap.resolvedStops();

    colormap.setPreset(Colormap::Preset::Grayscale);
    const auto grayscale = colormap.resolvedStops();
    QCOMPARE(grayscale.front().color, QColor{Qt::black});
    QCOMPARE(grayscale.back().color, QColor{Qt::white});
    QVERIFY(grayscale.front().color != viridis.front().color);
}

void ColormapTest::presetsSpanReferenceColors_data()
{
    QTest::addColumn<Colormap::Preset>("preset");
    QTest::addColumn<QColor>("first");
    QTest::addColumn<QColor>("last");

    QTest::newRow("Viridis") << Colormap::Preset::Viridis << QColor{"#440154"} << QColor{"#fde725"};
    QTest::newRow("Plasma") << Colormap::Preset::Plasma << QColor{"#0d0887"} << QColor{"#f0f921"};
    QTest::newRow("Inferno") << Colormap::Preset::Inferno << QColor{"#000004"} << QColor{"#fcffa4"};
    QTest::newRow("Magma") << Colormap::Preset::Magma << QColor{"#000004"} << QColor{"#fcfdbf"};
}

void ColormapTest::presetsSpanReferenceColors()
{
    QFETCH(Colormap::Preset, preset);
    QFETCH(QColor, first);
    QFETCH(QColor, last);

    auto colormap = Colormap{};
    colormap.setPreset(preset);
    const auto& stops = colormap.resolvedStops();
    QCOMPARE(stops.size(), std::size_t{32});
    QCOMPARE(stops.front().position, 0.0f);
    QCOMPARE(stops.back().position, 1.0f);
    QCOMPARE(stops.front().color, first);
    QCOMPARE(stops.back().color, last);
}

void ColormapTest::reversedFlipsPresetRamp()
{
    auto colormap = Colormap{};
    const auto forward = colormap.resolvedStops();
    QVERIFY(!colormap.reversed());

    auto spy = QSignalSpy{&colormap, &Colormap::colormapChanged};
    colormap.setReversed(true);
    QCOMPARE(spy.count(), 1);
    colormap.setReversed(true);
    QCOMPARE(spy.count(), 1);

    const auto& reversed = colormap.resolvedStops();
    QCOMPARE(reversed.size(), forward.size());
    for (auto index = std::size_t{0}; index < reversed.size(); ++index) {
        const auto& mirrored = forward[forward.size() - 1 - index];
        QCOMPARE(reversed[index].color, mirrored.color);
        QCOMPARE(reversed[index].position, 1.0f - mirrored.position);
    }

    // The flip follows the ramp when the preset changes afterwards.
    colormap.setPreset(Colormap::Preset::Grayscale);
    QCOMPARE(colormap.resolvedStops().front().color, QColor{Qt::white});
    QCOMPARE(colormap.resolvedStops().back().color, QColor{Qt::black});
}

void ColormapTest::qmlReversedFlipsCustomStops()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    auto* root = build(engine, component,
        "import QtQuick\n"
        "import QAccelPlot\n"
        "Colormap {\n"
        "    reversed: true\n"
        "    stops: [ GradientStop { position: 0.0; color: \"#ff0000\" },\n"
        "             GradientStop { position: 0.25; color: \"#0000ff\" } ]\n"
        "}\n");
    QVERIFY(root);

    const auto* colormap = qobject_cast<Colormap*>(root);
    QVERIFY(colormap);
    // The last stop is extended to 1, then the whole ramp is mirrored.
    const auto& stops = colormap->resolvedStops();
    QCOMPARE(stops.size(), std::size_t{3});
    QCOMPARE(stops[0].color, QColor{"#0000ff"});
    QCOMPARE(stops[0].position, 0.0f);
    QCOMPARE(stops[1].color, QColor{"#0000ff"});
    QCOMPARE(stops[1].position, 0.75f);
    QCOMPARE(stops[2].color, QColor{"#ff0000"});
    QCOMPARE(stops[2].position, 1.0f);
    delete root;
}

void ColormapTest::boundsAreUnsetUntilAssigned()
{
    auto colormap = Colormap{};
    QVERIFY(std::isnan(colormap.min()));
    QVERIFY(std::isnan(colormap.max()));

    auto spy = QSignalSpy{&colormap, &Colormap::colormapChanged};
    colormap.setMin(-2.0);
    QCOMPARE(colormap.min(), -2.0);
    QCOMPARE(spy.count(), 1);
    // Setting the same value again must not re-notify.
    colormap.setMin(-2.0);
    QCOMPARE(spy.count(), 1);
}

void ColormapTest::qmlStopsOverridePreset()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    auto* root = build(engine, component,
        "import QtQuick\n"
        "import QAccelPlot\n"
        "Colormap {\n"
        "    preset: Colormap.Grayscale\n"
        "    stops: [ GradientStop { position: 0.0; color: \"#ff0000\" },\n"
        "             GradientStop { position: 1.0; color: \"#0000ff\" } ]\n"
        "}\n");
    QVERIFY(root);

    const auto* colormap = qobject_cast<Colormap*>(root);
    QVERIFY(colormap);
    const auto& stops = colormap->resolvedStops();
    QCOMPARE(stops.size(), std::size_t{2});
    // Explicit stops win over the preset, which would be black-to-white here.
    QCOMPARE(stops.front().color, QColor{"#ff0000"});
    QCOMPARE(stops.back().color, QColor{"#0000ff"});
    delete root;
}

void ColormapTest::qmlGradientIsAccepted()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    // Sharing one ramp between a colour bar and a series is the common case, so assigning
    // another object's stop list has to work.
    auto* root = build(engine, component,
        "import QtQuick\n"
        "import QAccelPlot\n"
        "Item {\n"
        "    property alias colormap: map\n"
        "    Gradient {\n"
        "        id: ramp\n"
        "        GradientStop { position: 0.0; color: \"#fde725\" }\n"
        "        GradientStop { position: 1.0; color: \"#440154\" }\n"
        "    }\n"
        "    Colormap { id: map; stops: ramp.stops }\n"
        "}\n");
    QVERIFY(root);

    auto* colormap = root->property("colormap").value<Colormap*>();
    QVERIFY(colormap);
    const auto& stops = colormap->resolvedStops();
    QCOMPARE(stops.size(), std::size_t{2});
    QCOMPARE(stops.front().color, QColor{"#fde725"});
    QCOMPARE(stops.back().color, QColor{"#440154"});
    delete root;
}

void ColormapTest::gradientFillFollowsColormap()
{
    verifyEffectFollowsColormap<GradientFill>();
}

void ColormapTest::gradientStrokeFollowsColormap()
{
    verifyEffectFollowsColormap<GradientStroke>();
}

void ColormapTest::qmlEffectColormapOverridesGradient()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    auto* root = build(engine, component,
        "import QtQuick\n"
        "import QAccelPlot\n"
        "Item {\n"
        "    property alias fill: fillEffect\n"
        "    property alias stroke: strokeEffect\n"
        "    Gradient {\n"
        "        id: ramp\n"
        "        GradientStop { position: 0.0; color: \"#ff0000\" }\n"
        "        GradientStop { position: 1.0; color: \"#0000ff\" }\n"
        "    }\n"
        "    GradientFill {\n"
        "        id: fillEffect\n"
        "        gradient: ramp\n"
        "        colormap: Colormap { preset: Colormap.Grayscale }\n"
        "    }\n"
        "    GradientStroke {\n"
        "        id: strokeEffect\n"
        "        gradient: ramp\n"
        "        colormap: Colormap { preset: Colormap.Grayscale }\n"
        "    }\n"
        "}\n");
    QVERIFY(root);

    auto* fill = root->property("fill").value<GradientFill*>();
    auto* stroke = root->property("stroke").value<GradientStroke*>();
    QVERIFY(fill);
    QVERIFY(stroke);
    QCOMPARE(fill->payload().stops.front().color, QColor{Qt::black});
    QCOMPARE(stroke->payload().stops.back().color, QColor{Qt::white});

    // Without a colormap the effects fall back to the gradient stops.
    fill->setColormap(nullptr);
    stroke->setColormap(nullptr);
    QCOMPARE(fill->payload().stops.front().color, QColor{"#ff0000"});
    QCOMPARE(stroke->payload().stops.back().color, QColor{"#0000ff"});
    delete root;
}

} // namespace QAccelPlot

using QAccelPlot::ColormapTest;
QTEST_MAIN(ColormapTest)
#include "tst_colormap.moc"
