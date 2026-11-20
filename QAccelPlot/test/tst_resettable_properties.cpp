//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/PlotView.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/axis/AxisLabel.hpp"
#include "QAccelPlot/axis/AxisTicker.hpp"
#include "QAccelPlot/axis/ColorBar.hpp"
#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/effects/Glow.hpp"
#include "QAccelPlot/effects/GradientFill.hpp"
#include "QAccelPlot/effects/GradientStroke.hpp"
#include "QAccelPlot/export/PlotExporter.hpp"
#include "QAccelPlot/formatters/TextTickLabelFormatter.hpp"
#include "QAccelPlot/inspection/SelectionTool.hpp"
#include "QAccelPlot/series/BandSeries.hpp"
#include "QAccelPlot/series/Category.hpp"
#include "QAccelPlot/series/HeatmapSeries.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"
#include "QAccelPlot/transitions/GrowTransition.hpp"

#include <QMetaProperty>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QtTest/QtTest>

#include <cmath>
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
        {QStringLiteral("AxisLabel"), make<AxisLabel>()},
        {QStringLiteral("AxisTicker"), make<AxisTicker>()},
        {QStringLiteral("BandEdges"), make<BandEdges>()},
        {QStringLiteral("Category"), make<Category>()},
        {QStringLiteral("Colormap"), make<Colormap>()},
        {QStringLiteral("Glow"), make<Glow>()},
        {QStringLiteral("GradientFill"), make<GradientFill>()},
        {QStringLiteral("GradientStroke"), make<GradientStroke>()},
        {QStringLiteral("GrowTransition"), make<GrowTransition>()},
        {QStringLiteral("HeatmapSeries"), make<HeatmapSeries>()},
        {QStringLiteral("PlotExporter"), make<PlotExporter>()},
        {QStringLiteral("TextLabel"), make<TextLabel>()},
    };
    return instances;
}

// NaN is the unset readback of several properties, and it never compares equal to itself.
bool sameValue(const QVariant& left, const QVariant& right)
{
    if (left.typeId() == QMetaType::Double && right.typeId() == QMetaType::Double && std::isnan(left.toDouble()) && std::isnan(right.toDouble())) {
        return true;
    }
    return left == right;
}

} // namespace

class ResettablePropertiesTest : public QObject {
    Q_OBJECT

private slots:
    void resetRestoresTheDefault_data();
    void resetRestoresTheDefault();
    void tickLabelColorFollowsTickColor();
    void axisLabelColorFollowsBaselineColor();
    void colorBarLabelColorResetsToAxisLine();
    void bandEdgeColorFollowsBandColor();
    void heatmapEdgesFollowTheShape();
    void tickLabelFormatterResetsToTheDefault();
    void selectionToolAxesResetToThePrimaryAxes();
    void undefinedResetsFromQml();
};

void ResettablePropertiesTest::resetRestoresTheDefault_data()
{
    QTest::addColumn<QString>("type");
    QTest::addColumn<QByteArray>("property");
    QTest::addColumn<QVariant>("value");

    const auto row = [](const char* type, const char* property, const QVariant& value) {
        QTest::addRow("%s.%s", type, property) << QString::fromLatin1(type) << QByteArray{property} << value;
    };

    row("AxisLabel", "rotation", 45.0);
    row("AxisLabel", "color", QColor{Qt::cyan});
    row("TextLabel", "color", QColor{Qt::cyan});
    row("AxisTicker", "tickLabelColor", QColor{Qt::red});
    row("BandEdges", "color", QColor{Qt::darkRed});
    row("Category", "color", QColor{Qt::gray});
    row("Colormap", "min", 1.0);
    row("Colormap", "max", 5.0);
    row("Glow", "color", QColor{Qt::red});
    row("GradientFill", "valueMin", -2.0);
    row("GradientFill", "valueMax", 8.0);
    row("GradientFill", "baselineValue", 1.25);
    row("GradientStroke", "valueMin", -2.0);
    row("GradientStroke", "valueMax", 8.0);
    row("GrowTransition", "baseline", 3.0);
    row("HeatmapSeries", "xMin", -1.0);
    row("HeatmapSeries", "xMax", 4.0);
    row("HeatmapSeries", "yMin", -1.0);
    row("HeatmapSeries", "yMax", 4.0);
    row("PlotExporter", "scale", 2.0);
    row("PlotExporter", "quality", 50);
    row("PlotExporter", "dpi", 300.0);
}

void ResettablePropertiesTest::resetRestoresTheDefault()
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
    QVERIFY(metaProperty.isResettable());

    const auto defaultValue = metaProperty.read(object.get());
    auto spy = QSignalSpy{object.get(), metaProperty.notifySignal()};
    QVERIFY(metaProperty.write(object.get(), value));
    QCOMPARE(metaProperty.read(object.get()), value);

    QVERIFY(metaProperty.reset(object.get()));
    QVERIFY2(sameValue(metaProperty.read(object.get()), defaultValue), qPrintable(metaProperty.read(object.get()).toString()));
    QCOMPARE(spy.count(), 2);

    QVERIFY(metaProperty.reset(object.get()));
    QCOMPARE(spy.count(), 2);
}

void ResettablePropertiesTest::tickLabelColorFollowsTickColor()
{
    auto ticker = AxisTicker{};
    auto spy = QSignalSpy{&ticker, &AxisTicker::tickLabelColorChanged};

    ticker.setTickColor(Qt::blue);
    QCOMPARE(ticker.tickLabelColor(), QColor{Qt::blue});
    QCOMPARE(spy.count(), 1);

    ticker.setTickLabelColor(Qt::red);
    ticker.setTickColor(Qt::green);
    QCOMPARE(ticker.tickLabelColor(), QColor{Qt::red});
    QCOMPARE(spy.count(), 2);

    ticker.resetTickLabelColor();
    QCOMPARE(ticker.tickLabelColor(), QColor{Qt::green});
    QCOMPARE(spy.count(), 3);

    // Pinning the value in effect is not a change.
    ticker.setTickLabelColor(Qt::green);
    ticker.setTickColor(Qt::yellow);
    QCOMPARE(ticker.tickLabelColor(), QColor{Qt::green});
    QCOMPARE(spy.count(), 3);
}

void ResettablePropertiesTest::axisLabelColorFollowsBaselineColor()
{
    auto axis = Axis{};
    auto* label = axis.label();
    QCOMPARE(label->color(), axis.baselineColor());
    auto spy = QSignalSpy{label, &TextLabel::colorChanged};

    axis.setBaselineColor(Qt::magenta);
    QCOMPARE(label->color(), QColor{Qt::magenta});
    QCOMPARE(spy.count(), 1);

    label->setColor(Qt::cyan);
    axis.setBaselineColor(Qt::red);
    QCOMPARE(label->color(), QColor{Qt::cyan});

    label->setColor(QColor{});
    QCOMPARE(label->color(), QColor{Qt::red});
    QCOMPARE(spy.count(), 3);
}

void ResettablePropertiesTest::colorBarLabelColorResetsToAxisLine()
{
    auto bar = ColorBar{};
    bar.label()->setColor(Qt::cyan);
    bar.label()->resetColor();
    QCOMPARE(bar.label()->color(), ColorPalette::dark().axisLine);
}

void ResettablePropertiesTest::bandEdgeColorFollowsBandColor()
{
    auto band = BandSeries{};
    auto* edges = band.edges();
    auto expected = band.color();
    expected.setAlpha(255);
    QCOMPARE(edges->color(), expected);

    auto spy = QSignalSpy{edges, &BandEdges::colorChanged};
    band.setColor(QColor{10, 20, 30, 64});
    QCOMPARE(edges->color(), QColor(10, 20, 30, 255));
    QCOMPARE(spy.count(), 1);

    edges->setColor(Qt::white);
    band.setColor(Qt::black);
    QCOMPARE(edges->color(), QColor{Qt::white});
    edges->resetColor();
    QCOMPARE(edges->color(), QColor{Qt::black});
    QCOMPARE(spy.count(), 3);
}

void ResettablePropertiesTest::heatmapEdgesFollowTheShape()
{
    auto heatmap = HeatmapSeries{};
    auto xMaxSpy = QSignalSpy{&heatmap, &HeatmapSeries::xMaxChanged};
    auto yMaxSpy = QSignalSpy{&heatmap, &HeatmapSeries::yMaxChanged};
    auto xMinSpy = QSignalSpy{&heatmap, &HeatmapSeries::xMinChanged};

    heatmap.setValues(QList<qreal>{0, 1, 2, 3, 4, 5}, 3);
    QCOMPARE(heatmap.xMin(), 0.0);
    QCOMPARE(heatmap.xMax(), 3.0);
    QCOMPARE(heatmap.yMax(), 2.0);
    QCOMPARE(xMaxSpy.count(), 1);
    QCOMPARE(yMaxSpy.count(), 1);
    QCOMPARE(xMinSpy.count(), 0);

    heatmap.setXMax(10.0);
    heatmap.setColumns(2);
    QCOMPARE(heatmap.xMax(), 10.0);
    QCOMPARE(heatmap.yMax(), 3.0);
    QCOMPARE(xMaxSpy.count(), 2);
    QCOMPARE(yMaxSpy.count(), 2);

    heatmap.resetXMax();
    QCOMPARE(heatmap.xMax(), 2.0);
    QCOMPARE(xMaxSpy.count(), 3);

    // Setting the value in effect pins it without a change.
    heatmap.setXMin(0.0);
    QCOMPARE(xMinSpy.count(), 0);
    heatmap.setXMin(std::numeric_limits<qreal>::quiet_NaN());
    QCOMPARE(heatmap.xMin(), 0.0);
    QCOMPARE(xMinSpy.count(), 0);
}

void ResettablePropertiesTest::tickLabelFormatterResetsToTheDefault()
{
    auto ticker = AxisTicker{};
    auto* defaultFormatter = ticker.tickLabelFormatter();
    QVERIFY(defaultFormatter);
    auto custom = TextTickLabelFormatter{};
    auto spy = QSignalSpy{&ticker, &AxisTicker::tickLabelFormatterChanged};

    ticker.setTickLabelFormatter(&custom);
    QCOMPARE(ticker.tickLabelFormatter(), &custom);
    ticker.resetTickLabelFormatter();
    QCOMPARE(ticker.tickLabelFormatter(), defaultFormatter);
    QCOMPARE(spy.count(), 2);
}

void ResettablePropertiesTest::selectionToolAxesResetToThePrimaryAxes()
{
    auto plot = PlotView{};
    auto primaryX = Axis{};
    auto primaryY = Axis{};
    plot.setXAxis(&primaryX);
    plot.setYAxis(&primaryY);
    auto tool = SelectionTool{};
    tool.setPlot(&plot);
    QCOMPARE(tool.xAxis(), &primaryX);

    auto xSpy = QSignalSpy{&tool, &SelectionTool::xAxisChanged};
    tool.setXAxis(&primaryX);
    QCOMPARE(xSpy.count(), 0);

    auto other = Axis{};
    tool.setXAxis(&other);
    tool.setYAxis(&other);
    QCOMPARE(tool.xAxis(), &other);
    tool.resetXAxis();
    tool.resetYAxis();
    QCOMPARE(tool.xAxis(), &primaryX);
    QCOMPARE(tool.yAxis(), &primaryY);
    QCOMPARE(xSpy.count(), 2);

    plot.setXAxis(nullptr);
    plot.setYAxis(nullptr);
}

void ResettablePropertiesTest::undefinedResetsFromQml()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(R"(
        import QtQuick
        import QAccelPlot as QAccelPlot
        Item {
            id: root
            property bool automatic: false
            QAccelPlot.Axis {
                objectName: "axis"
                label.color: root.automatic ? undefined : "red"
                label.rotation: root.automatic ? undefined : 45
                ticker.tickLabelColor: root.automatic ? undefined : "red"
            }
            QAccelPlot.HeatmapSeries { objectName: "heatmap"; xMax: root.automatic ? undefined : 7 }
            QAccelPlot.BandSeries { objectName: "band"; edges.color: root.automatic ? undefined : "red" }
            QAccelPlot.Colormap { objectName: "colormap"; min: root.automatic ? undefined : 2 }
            QAccelPlot.GradientFill {
                objectName: "fill"
                valueMin: root.automatic ? undefined : 2
                baselineValue: root.automatic ? undefined : 0
            }
            QAccelPlot.Category { objectName: "category"; color: root.automatic ? undefined : "red" }
            QAccelPlot.Glow { objectName: "glow"; color: root.automatic ? undefined : "red" }
            QAccelPlot.GrowTransition { objectName: "grow"; baseline: root.automatic ? undefined : 3 }
            QAccelPlot.PlotExporter { objectName: "exporter"; scale: root.automatic ? undefined : 2 }
            QAccelPlot.SelectionTool { id: tool; objectName: "tool" }
            function assignUndefined() {
                tool.xAxis = undefined;
                return tool.xAxis === null;
            }
        }
    )",
        QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(root, qPrintable(component.errorString()));

    const auto find = [&root](const char* name) {
        auto* object = root->findChild<QObject*>(QString::fromLatin1(name));
        return object;
    };
    auto* axis = qobject_cast<Axis*>(find("axis"));
    auto* heatmap = qobject_cast<HeatmapSeries*>(find("heatmap"));
    auto* band = qobject_cast<BandSeries*>(find("band"));
    QVERIFY(axis && heatmap && band);
    QCOMPARE(axis->label()->color(), QColor{Qt::red});
    QCOMPARE(heatmap->xMax(), 7.0);

    root->setProperty("automatic", true);
    QCOMPARE(axis->label()->color(), axis->baselineColor());
    QCOMPARE(axis->label()->rotation(), 0.0);
    QCOMPARE(axis->ticker()->tickLabelColor(), axis->ticker()->tickColor());
    QCOMPARE(heatmap->xMax(), 0.0);
    auto bandEdgeColor = band->color();
    bandEdgeColor.setAlpha(255);
    QCOMPARE(band->edges()->color(), bandEdgeColor);
    QVERIFY(std::isnan(qobject_cast<Colormap*>(find("colormap"))->min()));
    QVERIFY(std::isnan(qobject_cast<GradientFill*>(find("fill"))->valueMin()));
    QVERIFY(std::isnan(qobject_cast<GradientFill*>(find("fill"))->baselineValue()));
    QVERIFY(!qobject_cast<Category*>(find("category"))->color().isValid());
    QVERIFY(!qobject_cast<Glow*>(find("glow"))->color().isValid());
    QVERIFY(std::isnan(qobject_cast<GrowTransition*>(find("grow"))->baseline()));
    QCOMPARE(qobject_cast<PlotExporter*>(find("exporter"))->scale(), 0.0);

    root->setProperty("automatic", false);
    QCOMPARE(axis->label()->color(), QColor{Qt::red});
    QCOMPARE(heatmap->xMax(), 7.0);

    auto result = QVariant{};
    QVERIFY(QMetaObject::invokeMethod(root.get(), "assignUndefined", Q_RETURN_ARG(QVariant, result)));
    QVERIFY(result.toBool());
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::ResettablePropertiesTest)
#include "tst_resettable_properties.moc"
