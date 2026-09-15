//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot.hpp"
#include "axis/Axis.hpp"
#include "series/LineCurve.hpp"
#include "shapes/RectangleList.hpp"
#include "theme/ColorPalette.hpp"
#include "theme/Colors.hpp"

#include <QMetaProperty>
#include <QtTest/QtTest>

class TestColorPalette : public QObject {
    Q_OBJECT

private slots:
    void palettesDefineEveryColor_data();
    void palettesDefineEveryColor();
    void paletteColorsAreReadOnly();
    void colorsSingletonExposesSharedPalettes();
    void cppDefaultsUseDarkPalette();
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

    const QAccelPlot::RectangleList rectangles;
    QCOMPARE(rectangles.color().rgb(), palette.seriesPrimary.rgb());
    QCOMPARE(rectangles.color().alpha(), 50);
}

QTEST_MAIN(TestColorPalette)
#include "tst_color_palette.moc"
