//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/PointCloud.hpp"
#include "QAccelPlot/shapes/RectangleList.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"
#include "QAccelPlot/theme/Colors.hpp"

#include <QMetaProperty>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QtTest/QtTest>

class TestColorPalette : public QObject {
    Q_OBJECT

private slots:
    void palettesDefineEveryColor_data();
    void palettesDefineEveryColor();
    void paletteColorsAreReadOnly();
    void colorsSingletonExposesSharedPalettes();
    void cppDefaultsUseDarkPalette();
    void qmlLegendDefaultsUseDarkPalette();
};

void TestColorPalette::palettesDefineEveryColor_data()
{
    QTest::addColumn<bool>("dark");
    QTest::newRow("dark") << true;
    QTest::newRow("light") << false;
}

void TestColorPalette::palettesDefineEveryColor()
{
    QFETCH(bool, dark);
    const auto* palette = dark ? &QAccelPlot::ColorPalette::dark() : &QAccelPlot::ColorPalette::light();

    const auto* metaObject = palette->metaObject();
    for (auto i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i) {
        const auto property = metaObject->property(i);
        QVERIFY2(property.read(palette).value<QColor>().isValid(), property.name());
    }
}

void TestColorPalette::paletteColorsAreReadOnly()
{
    const auto* metaObject = &QAccelPlot::ColorPalette::staticMetaObject;
    QVERIFY(metaObject->propertyCount() > metaObject->propertyOffset());
    for (auto i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i) {
        const auto property = metaObject->property(i);
        QVERIFY2(!property.isWritable(), property.name());
        QVERIFY2(property.isConstant(), property.name());
    }
}

void TestColorPalette::colorsSingletonExposesSharedPalettes()
{
    const QAccelPlot::Colors colors;

    QCOMPARE(colors.dark(), &QAccelPlot::ColorPalette::dark());
    QCOMPARE(colors.light(), &QAccelPlot::ColorPalette::light());
    QCOMPARE(colors.light()->axisLabel, colors.light()->textSecondary);
}

void TestColorPalette::cppDefaultsUseDarkPalette()
{
    const auto& palette = QAccelPlot::ColorPalette::dark();

    const QAccelPlot::Axis axis;
    QCOMPARE(axis.hoverColor(), palette.hover);
    QCOMPARE(axis.baselineColor(), palette.axisLine);
    QCOMPARE(axis.ticker()->tickColor(), palette.tick);
    QCOMPARE(axis.ticker()->subtickColor(), palette.subtick);

    const QAccelPlot::QAccelPlot plot;
    QCOMPARE(plot.plotAreaColor(), palette.plotArea);
    QCOMPARE(plot.axesAreaColor(), palette.axesArea);
    QCOMPARE(plot.grid()->gridColor(), palette.grid);
    QCOMPARE(plot.grid()->subGridColor(), palette.subGrid);

    const QAccelPlot::LineCurve curve;
    QCOMPARE(curve.color(), palette.seriesPrimary);

    const QAccelPlot::PointCloud cloud;
    QCOMPARE(cloud.color(), palette.seriesPrimary);

    const QAccelPlot::RectangleList rectangles;
    QCOMPARE(rectangles.color().rgb(), palette.seriesPrimary.rgb());
    QCOMPARE(rectangles.color().alpha(), 50);
}

void TestColorPalette::qmlLegendDefaultsUseDarkPalette()
{
    QQmlEngine engine;
    QList<QQmlError> warnings;
    connect(&engine, &QQmlEngine::warnings, this, [&warnings](const QList<QQmlError>& errors) { warnings.append(errors); });

    QQmlComponent component(&engine);
    component.setData("import QtQuick\n"
                      "import QAccelPlot 1.0\n"
                      "Legend { series: [] }\n",
        QUrl(QStringLiteral("qrc:/tst_color_palette/LegendDefaults.qml")));
    QScopedPointer<QObject> legend(component.create());
    QVERIFY2(legend, qPrintable(component.errorString()));

    // A binding that cannot resolve the Colors singleton only warns and leaves QtQuick defaults.
    for (const auto& warning : std::as_const(warnings)) {
        QFAIL(qPrintable(warning.toString()));
    }

    const auto& palette = QAccelPlot::ColorPalette::dark();
    QCOMPARE(legend->property("color").value<QColor>(), palette.legendBackground);
    QCOMPARE(legend->property("textColor").value<QColor>(), palette.text);
    const auto* border = legend->property("border").value<QObject*>();
    QVERIFY(border);
    QCOMPARE(border->property("color").value<QColor>(), palette.legendBorder);
}

QTEST_MAIN(TestColorPalette)
#include "tst_color_palette.moc"
