//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/annotations/DataAnchor.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/axis/AxisTicker.hpp"
#include "QAccelPlot/axis/ColorBar.hpp"
#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/effects/GradientFill.hpp"
#include "QAccelPlot/effects/GradientStroke.hpp"
#include "QAccelPlot/formatters/DateTimeTickLabelFormatter.hpp"
#include "QAccelPlot/formatters/TextTickLabelFormatter.hpp"
#include "QAccelPlot/grid/Grid.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/shapes/RectangleList.hpp"
#include "QAccelPlot/transitions/MorphTransition.hpp"

#include <QEasingCurve>
#include <QFont>
#include <QMetaProperty>
#include <QtTest/QtTest>

#include <functional>
#include <map>
#include <memory>

namespace QAccelPlot {

namespace {

using Factory = std::function<std::unique_ptr<QObject>()>;

template <typename T> Factory make()
{
    return [] { return std::make_unique<T>(); };
}

const std::map<QString, Factory>& factories()
{
    static const auto instances = std::map<QString, Factory>{
        {QStringLiteral("Axis"), make<Axis>()},
        {QStringLiteral("AxisTicker"), make<AxisTicker>()},
        {QStringLiteral("ColorBar"), make<ColorBar>()},
        {QStringLiteral("Colormap"), make<Colormap>()},
        {QStringLiteral("DataAnchor"), make<DataAnchor>()},
        {QStringLiteral("DateTimeTickLabelFormatter"), make<DateTimeTickLabelFormatter>()},
        {QStringLiteral("GradientFill"), make<GradientFill>()},
        {QStringLiteral("GradientStroke"), make<GradientStroke>()},
        {QStringLiteral("Grid"), make<Grid>()},
        {QStringLiteral("LineCurve"), make<LineCurve>()},
        {QStringLiteral("MorphTransition"), make<MorphTransition>()},
        {QStringLiteral("PlotView"), make<QAccelPlot>()},
        {QStringLiteral("RectangleList"), make<RectangleList>()},
        {QStringLiteral("TextTickLabelFormatter"), make<TextTickLabelFormatter>()},
    };
    return instances;
}

} // namespace

class PropertyNotificationsTest : public QObject {
    Q_OBJECT

private slots:
    void writeNotifiesOnceAndRoundTrips_data();
    void writeNotifiesOnceAndRoundTrips();
};

void PropertyNotificationsTest::writeNotifiesOnceAndRoundTrips_data()
{
    QTest::addColumn<QString>("type");
    QTest::addColumn<QByteArray>("property");
    QTest::addColumn<QVariant>("value");

    const auto row = [](const char* type, const char* property, const QVariant& value) {
        QTest::addRow("%s.%s", type, property) << QString::fromLatin1(type) << QByteArray{property} << value;
    };

    row("Axis", "label", QStringLiteral("Time"));
    row("Axis", "labelFont", QFont{QStringLiteral("Arial"), 17});
    row("Axis", "labelColor", QColor{Qt::cyan});
    row("Axis", "baselineColor", QColor{Qt::magenta});
    row("Axis", "hoverColor", QColor{Qt::yellow});
    row("Axis", "axisTitlePadding", 13);
    row("Axis", "axisLinePadding", 9);

    row("AxisTicker", "subtickLengthIn", 7.0);
    row("AxisTicker", "subtickLengthOut", 6.0);
    row("AxisTicker", "subtickColor", QColor{Qt::darkRed});
    row("AxisTicker", "subtickWidth", 3.0);

    row("ColorBar", "orientation", QVariant::fromValue(ColorBar::Horizontal));
    row("ColorBar", "label", QStringLiteral("Intensity"));
    row("ColorBar", "labelFont", QFont{QStringLiteral("Arial"), 17});
    row("ColorBar", "labelColor", QColor{Qt::cyan});
    row("ColorBar", "labelPadding", 9.0);
    row("ColorBar", "barThickness", 20.0);
    row("ColorBar", "borderColor", QColor{Qt::magenta});
    row("ColorBar", "borderWidth", 2.0);

    row("Colormap", "norm", QVariant::fromValue(Colormap::Normalization::Log));

    row("DataAnchor", "dataX1", 1.5);
    row("DataAnchor", "dataY1", 2.5);
    row("DataAnchor", "dataX2", 3.5);
    row("DataAnchor", "dataY2", 4.5);
    row("DataAnchor", "hoverThreshold", 2.0);

    row("DateTimeTickLabelFormatter", "dateTimeFormat", QStringLiteral("yyyy"));

    row("GradientFill", "direction", QVariant::fromValue(GradientDirection::Vertical));
    row("GradientFill", "gradientValueMinSource", QVariant::fromValue(GradientValueSource::Fixed));
    row("GradientFill", "gradientValueMin", -2.0);
    row("GradientFill", "gradientValueMaxSource", QVariant::fromValue(GradientValueSource::Fixed));
    row("GradientFill", "gradientValueMax", 8.0);
    row("GradientFill", "baseline", QVariant::fromValue(GradientFillBaseline::Value));
    row("GradientFill", "baselineValue", 1.25);
    row("GradientFill", "opacity", 0.8);
    row("GradientFill", "enabled", false);

    row("GradientStroke", "direction", QVariant::fromValue(GradientDirection::Vertical));
    row("GradientStroke", "gradientValueMinSource", QVariant::fromValue(GradientValueSource::Fixed));
    row("GradientStroke", "gradientValueMin", -2.0);
    row("GradientStroke", "gradientValueMaxSource", QVariant::fromValue(GradientValueSource::Fixed));
    row("GradientStroke", "gradientValueMax", 8.0);
    row("GradientStroke", "enabled", false);

    row("Grid", "lineWidth", 3.0);
    row("Grid", "subGridLineWidth", 2.0);
    row("Grid", "gridColor", QColor{Qt::red});
    row("Grid", "subGridColor", QColor{Qt::blue});
    row("Grid", "gridVisible", false);
    row("Grid", "subGridVisible", false);
    row("Grid", "gridHorizontalLinesVisible", false);
    row("Grid", "gridVerticalLinesVisible", false);
    row("Grid", "subGridHorizontalLinesVisible", false);
    row("Grid", "subGridVerticalLinesVisible", false);

    row("LineCurve", "name", QStringLiteral("Series A"));

    row("MorphTransition", "duration", 1234);
    row("MorphTransition", "easing", QEasingCurve{QEasingCurve::OutBounce});
    row("MorphTransition", "enabled", false);

    row("PlotView", "padding", 12.0);
    row("PlotView", "plotAreaColor", QColor{Qt::darkGreen});
    row("PlotView", "axesAreaColor", QColor{Qt::darkBlue});

    row("RectangleList", "color", QColor{Qt::darkCyan});

    row("TextTickLabelFormatter", "labels", QStringList{QStringLiteral("a"), QStringLiteral("b")});
}

void PropertyNotificationsTest::writeNotifiesOnceAndRoundTrips()
{
    QFETCH(QString, type);
    QFETCH(QByteArray, property);
    QFETCH(QVariant, value);

    const auto factory = factories().find(type);
    QVERIFY2(factory != factories().end(), qPrintable(type));
    const auto object = factory->second();
    const auto* meta = object->metaObject();
    const auto metaProperty = meta->property(meta->indexOfProperty(property.constData()));
    QVERIFY2(metaProperty.isValid(), property.constData());
    QVERIFY(metaProperty.hasNotifySignal());
    QVERIFY2(metaProperty.read(object.get()) != value, "test value must differ from the default");

    auto spy = QSignalSpy{object.get(), metaProperty.notifySignal()};
    QVERIFY(metaProperty.write(object.get(), value));
    QCOMPARE(metaProperty.read(object.get()), value);
    QCOMPARE(spy.count(), 1);

    QVERIFY(metaProperty.write(object.get(), value));
    QCOMPARE(spy.count(), 1);
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::PropertyNotificationsTest)
#include "tst_property_notifications.moc"
