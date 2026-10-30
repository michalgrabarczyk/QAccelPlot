//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/legend/LegendEntry.hpp"
#include "QAccelPlot/legend/LegendModel.hpp"
#include "QAccelPlot/linestyles/DashLine.hpp"
#include "QAccelPlot/series/BarSeries.hpp"
#include "QAccelPlot/series/HeatmapSeries.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/PointCloud.hpp"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include <memory>

namespace QAccelPlot {

namespace {

LegendEntry entryAt(const LegendModel& model, const int row)
{
    return model.data(model.index(row), LegendModel::EntryRole).value<LegendEntry>();
}

PlotSeries* seriesAt(const LegendModel& model, const int row)
{
    return model.data(model.index(row), LegendModel::SeriesRole).value<PlotSeries*>();
}

} // namespace

class LegendModelTest : public QObject {
    Q_OBJECT

private slots:
    void listsSeriesWithAnEntry();
    void hiddenSeriesHaveNoRow();
    void entryChangesUpdateTheRow();
    void lineCurveEntryDescribesLineAndMarker();
    void pointCloudAndBarEntriesUseTheirSymbol();
    void namesFallBackToObjectNameAndPosition();
    void destroyedSeriesLeaveTheModel();
    void plotLegendHidesWithoutEntries();
};

void LegendModelTest::listsSeriesWithAnEntry()
{
    auto curve = LineCurve{};
    auto heatmap = HeatmapSeries{};
    auto bars = BarSeries{};
    auto model = LegendModel{};
    auto seriesSpy = QSignalSpy{&model, &LegendModel::seriesChanged};
    auto countSpy = QSignalSpy{&model, &LegendModel::countChanged};

    model.setSeries({&curve, &heatmap, &bars});
    QCOMPARE(model.series(), (QList<PlotSeries*>{&curve, &heatmap, &bars}));
    QCOMPARE(model.count(), 2);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(seriesAt(model, 0), &curve);
    QCOMPARE(seriesAt(model, 1), &bars);
    QCOMPARE(seriesSpy.count(), 1);
    QCOMPARE(countSpy.count(), 1);

    model.setSeries({&curve, &heatmap, &bars});
    QCOMPARE(seriesSpy.count(), 1);
}

void LegendModelTest::hiddenSeriesHaveNoRow()
{
    // Visibility is effective visibility, which a parentless item does not regain.
    auto plotArea = QQuickItem{};
    auto curve = LineCurve{&plotArea};
    auto bars = BarSeries{&plotArea};
    auto model = LegendModel{};
    model.setSeries({&curve, &bars});
    auto countSpy = QSignalSpy{&model, &LegendModel::countChanged};

    curve.setVisible(false);
    QCOMPARE(model.count(), 1);
    QCOMPARE(seriesAt(model, 0), &bars);
    curve.setVisible(true);
    QCOMPARE(model.count(), 2);
    QCOMPARE(seriesAt(model, 0), &curve);
    QCOMPARE(countSpy.count(), 2);
}

void LegendModelTest::entryChangesUpdateTheRow()
{
    auto curve = LineCurve{};
    auto model = LegendModel{};
    model.setSeries({&curve});
    auto changes = QSignalSpy{&model, &QAbstractItemModel::dataChanged};
    auto resets = QSignalSpy{&model, &QAbstractItemModel::modelReset};

    curve.setColor(Qt::red);
    QCOMPARE(changes.count(), 1);
    QCOMPARE(entryAt(model, 0).color, QColor{Qt::red});

    curve.marker()->setSize(9.0);
    curve.setLegendSymbol(PlotSeries::LegendSymbol::Fill);
    curve.setName(QStringLiteral("renamed"));
    QCOMPARE(changes.count(), 4);
    QCOMPARE(entryAt(model, 0).markerSize, 9.0);
    QCOMPARE(entryAt(model, 0).symbol, PlotSeries::LegendSymbol::Fill);
    QCOMPARE(model.data(model.index(0), LegendModel::NameRole).toString(), QStringLiteral("renamed"));

    QVERIFY(entryAt(model, 0).antialiasing);
    curve.setAntialiasing(false);
    QCOMPARE(changes.count(), 5);
    QVERIFY(!entryAt(model, 0).antialiasing);
    QCOMPARE(resets.count(), 0);
}

void LegendModelTest::lineCurveEntryDescribesLineAndMarker()
{
    auto curve = LineCurve{};
    auto dash = DashLine{};
    curve.setColor(Qt::green);
    curve.setLineWidth(3.0);
    curve.setLineStyle(&dash);
    curve.setAntialiasing(false);
    curve.marker()->setShape(MarkerShape::Diamond);
    curve.marker()->setSize(5.0);
    curve.marker()->setFilled(false);
    curve.marker()->setStrokeWidth(2.0);

    const auto entry = curve.legendEntry();
    QVERIFY(entry);
    QCOMPARE(entry->symbol, PlotSeries::LegendSymbol::Line);
    QCOMPARE(entry->color, QColor{Qt::green});
    QCOMPARE(entry->lineWidth, 3.0);
    QCOMPARE(entry->lineStyle, &dash);
    QCOMPARE(entry->markerShape, MarkerShape::Diamond);
    QCOMPARE(entry->markerSize, 5.0);
    QVERIFY(!entry->markerFilled);
    QCOMPARE(entry->markerStrokeWidth, 2.0);
    QVERIFY(!entry->antialiasing);
}

void LegendModelTest::pointCloudAndBarEntriesUseTheirSymbol()
{
    auto cloud = PointCloud{};
    cloud.setColor(Qt::blue);
    const auto cloudEntry = cloud.legendEntry();
    QVERIFY(cloudEntry);
    QCOMPARE(cloudEntry->symbol, PlotSeries::LegendSymbol::Marker);
    QCOMPARE(cloudEntry->color, QColor{Qt::blue});
    QCOMPARE(cloudEntry->markerShape, MarkerShape::Circle);

    auto bars = BarSeries{};
    bars.setColor(Qt::yellow);
    const auto barEntry = bars.legendEntry();
    QVERIFY(barEntry);
    QCOMPARE(barEntry->symbol, PlotSeries::LegendSymbol::Fill);
    QCOMPARE(barEntry->color, QColor{Qt::yellow});
}

void LegendModelTest::namesFallBackToObjectNameAndPosition()
{
    auto named = LineCurve{};
    named.setName(QStringLiteral("signal"));
    auto objectNamed = LineCurve{};
    objectNamed.setObjectName(QStringLiteral("fit"));
    auto unnamed = LineCurve{};
    auto model = LegendModel{};
    model.setSeries({&named, &objectNamed, &unnamed});

    QCOMPARE(model.data(model.index(0), LegendModel::NameRole).toString(), QStringLiteral("signal"));
    QCOMPARE(model.data(model.index(1), LegendModel::NameRole).toString(), QStringLiteral("fit"));
    QCOMPARE(model.data(model.index(2), LegendModel::NameRole).toString(), QStringLiteral("Series 3"));
    QCOMPARE(model.roleNames().value(LegendModel::EntryRole), QByteArray{"entry"});
}

void LegendModelTest::destroyedSeriesLeaveTheModel()
{
    auto kept = LineCurve{};
    auto removed = std::make_unique<LineCurve>();
    auto model = LegendModel{};
    model.setSeries({&kept, removed.get()});
    auto seriesSpy = QSignalSpy{&model, &LegendModel::seriesChanged};

    removed.reset();
    QCOMPARE(model.count(), 1);
    QCOMPARE(seriesAt(model, 0), &kept);
    QCOMPARE(model.series(), QList<PlotSeries*>{&kept});
    QCOMPARE(seriesSpy.count(), 1);
}

void LegendModelTest::plotLegendHidesWithoutEntries()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(R"(
        import QtQuick
        import QAccelPlot as QAccelPlot
        QAccelPlot.Plot {
            width: 400; height: 300
            xAxis: QAccelPlot.Axis {}
            yAxis: QAccelPlot.Axis {}
            QAccelPlot.LineCurve { objectName: "curve"; name: "Curve" }
            QAccelPlot.HeatmapSeries {}
        }
    )",
        QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(root, qPrintable(component.errorString()));
    auto* legend = root->property("legend").value<QQuickItem*>();
    auto* model = root->property("legendModel").value<LegendModel*>();
    auto* curve = root->findChild<PlotSeries*>(QStringLiteral("curve"));
    QVERIFY(legend && model && curve);

    // The heatmap has no entry.
    QCOMPARE(model->count(), 1);
    QVERIFY(legend->isVisible());

    QVERIFY(root->setProperty("legendVisible", false));
    QVERIFY(!legend->isVisible());
    QVERIFY(root->setProperty("legendVisible", true));
    QVERIFY(legend->isVisible());

    curve->setVisible(false);
    QCOMPARE(model->count(), 0);
    QVERIFY(!legend->isVisible());
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::LegendModelTest)

#include "tst_legend_model.moc"
