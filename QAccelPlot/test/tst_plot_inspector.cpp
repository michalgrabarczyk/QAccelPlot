//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "InspectionPlotRig.hpp"

#include <QAccelPlot/inspection/PlotInspector.hpp>
#include <QAccelPlot/series/BarSeries.hpp>

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QtTest>

#include <cmath>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

using namespace QAccelPlot;
using namespace InspectionTest;

namespace {

// A formatter that runs arbitrary code the first time it formats a value.
class MutationFormatter : public TickLabelFormatter {
public:
    mutable std::function<void()> action;

protected:
    QString doFormat(const qreal value, const qreal /*step*/) const override
    {
        if (action) {
            std::exchange(action, {})();
        }
        return QString::number(value);
    }
};

// A shown window with three dense curves, an inspector, and a tooltip whose delegates count their creations.
// The inspector's cursor is set from code, so the real mouse cannot interfere.
struct TooltipScene {
    QQuickWindow window;
    InspectionPlot* plot{new InspectionPlot{}};
    Axis x;
    Axis y;
    PlotInspector inspector;
    QQmlEngine engine;
    std::unique_ptr<QObject> object;
    QQuickItem* tooltip{nullptr};
    QString error;

    TooltipScene()
    {
        window.resize(800, 400);
        plot->setParentItem(window.contentItem());
        plot->setSize({800, 400});
        x.setViewportMin(0);
        x.setViewportMax(10);
        y.setViewportMin(-2);
        y.setViewportMax(3);
        plot->setXAxis(&x);
        plot->setYAxis(&y);
        for (auto series = 0; series < 3; ++series) {
            auto* curve = new LineCurve(plot);
            curve->setName(QStringLiteral("S%1").arg(series));
            curve->setXAxis(&x);
            curve->setYAxis(&y);
            auto data = std::vector<double>{};
            for (auto i = 0; i < 2000; ++i) {
                data.push_back(i * 9.8 / 2000);
                data.push_back(std::sin(i * 0.01 + series));
            }
            curve->setData(std::move(data), 2000);
        }
        inspector.setPlot(plot);
        inspector.setFollowPointer(false);
        auto component = QQmlComponent{&engine};
        component.setData(R"(
            import QtQuick
            import QAccelPlot as QAccelPlot
            QAccelPlot.InspectionTooltip {
                id: tip
                property int created: 0
                rowDelegate: Text {
                    required property bool valid
                    required property string seriesName
                    required property string yText
                    visible: valid
                    text: seriesName + ": " + yText
                    Component.onCompleted: tip.created++
                }
            }
        )",
            QUrl{});
        object.reset(component.createWithInitialProperties({{"inspector", QVariant::fromValue(&inspector)}}));
        tooltip = qobject_cast<QQuickItem*>(object.get());
        error = component.errorString();
        window.show();
    }
};

} // namespace

class TestPlotInspector : public QObject {
    Q_OBJECT
private slots:
    void rows();
    void matching();
    void summariesAndFilters();
    void notifications();
    void cursor();
    void snapsToClosest();
    void formatterMutation();
    void touch();
    void tooltipReusesDelegates();
    void tooltipPlacement();
};

void TestPlotInspector::rows()
{
    auto rig = PlotRig{0, 10, 0, 10};
    auto* curve = rig.addCurve({1, 3.14159, 9, 6}, "curve");
    auto* bars = new BarSeries(&rig.plot);
    bars->setXAxis(&rig.x);
    bars->setYAxis(&rig.y);
    bars->setData(std::vector<double>{2, 7, 5, 4}, 2);
    QTRY_COMPARE(rig.plot.series().size(), 2);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    auto* model = inspector.model();
    auto resets = QSignalSpy{model, &QAbstractItemModel::modelReset};

    // Only series with XY samples get a row.
    rig.hover(rig.pixel(4, 5));
    inspector.refresh();
    QVERIFY(inspector.active());
    QCOMPARE(model->count(), 1);
    QCOMPARE(inspector.validCount(), 1);
    const auto row = model->get(0);
    QCOMPARE(row.value("series").value<PlotSeries*>(), curve);
    QCOMPARE(row.value("seriesName").toString(), QStringLiteral("curve"));
    QCOMPARE(row.value("seriesColor").value<QColor>(), curve->color());
    QVERIFY(row.value("valid").toBool());
    QCOMPARE(row.value("sampleIndex").toInt(), 0);
    QCOMPARE(row.value("sampleY").toDouble(), 3.14159);
    // Texts follow the pixel resolution instead of the tick step.
    QCOMPARE(row.value("yText").toString(), QStringLiteral("3.142"));
    QCOMPARE(row.value("xText").toString(), QStringLiteral("1.000"));
    QCOMPARE(row.value("pixelPosition").toPointF(), rig.pixel(1, 3.14159));
    QCOMPARE(resets.count(), 1);

    // Moving the cursor updates the row in place.
    auto changes = QSignalSpy{model, &QAbstractItemModel::dataChanged};
    rig.hover(rig.pixel(8, 5));
    inspector.refresh();
    QCOMPARE(model->get(0).value("sampleIndex").toInt(), 1);
    QCOMPARE(resets.count(), 1);
    QCOMPARE(changes.count(), 1);

    // Leaving the plot keeps the row, without a match; removing the series removes it at once.
    rig.leave();
    QTRY_VERIFY(!inspector.active());
    QCOMPARE(inspector.validCount(), 0);
    QCOMPARE(model->count(), 1);
    delete curve;
    QCOMPARE(model->count(), 0);
}

void TestPlotInspector::matching()
{
    auto rig = PlotRig{0, 10, 0, 10};
    rig.addCurve({1, 3, 9, 6});
    QTRY_COMPARE(rig.plot.series().size(), 1);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    const auto rowAt = [&](const QPointF& point) {
        rig.hover(point);
        inspector.refresh();
        return inspector.model()->get(0);
    };

    // Between two samples a series matches whatever the distance; past its ends the radius decides.
    QVERIFY(rowAt(rig.pixel(4, 5)).value("valid").toBool());
    QVERIFY(rowAt(rig.pixel(9.1, 5)).value("valid").toBool());
    const auto beyond = rowAt(rig.pixel(9.9, 5));
    QVERIFY(!beyond.value("valid").toBool());
    QCOMPARE(beyond.value("sampleStatus").value<InspectionStatus>(), InspectionStatus::NoMatch);
    QCOMPARE(inspector.validCount(), 0);

    // NearestXY always applies the radius.
    inspector.setMode(PlotInspector::NearestXY);
    QVERIFY(!rowAt(rig.pixel(4, 5)).value("valid").toBool());
    QCOMPARE(rowAt(rig.pixel(9, 6) + QPointF{3, 4}).value("distance").toDouble(), 5.0);
    inspector.setMode(PlotInspector::NearestX);

    // Interpolation reports the point on the line under the cursor.
    inspector.setInterpolate(true);
    const auto between = rowAt(rig.pixel(5, 5));
    QVERIFY(between.value("interpolated").toBool());
    QVERIFY(std::abs(between.value("sampleX").toDouble() - 5.0) < 1e-9);
    QVERIFY(std::abs(between.value("sampleY").toDouble() - 4.5) < 1e-9);
}

void TestPlotInspector::summariesAndFilters()
{
    auto rig = PlotRig{0, 10, 0, 10};
    auto* curve = rig.addCurve({1, 3, 9, 6});
    QTRY_COMPARE(rig.plot.series().size(), 1);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    auto* model = inspector.model();
    rig.hover(rig.pixel(5, 5));

    // Summaries cover the samples around the cursor and follow data changes.
    inspector.setSummaries(true);
    inspector.setSummaryRadius(1000);
    curve->setDataNoRange(std::vector<double>{1, 8, 9, 2}, 2);
    QTRY_COMPARE(model->get(0).value("summaryCount").toInt(), 2);
    const auto row = model->get(0);
    QVERIFY(row.value("hasSummary").toBool());
    QCOMPARE(row.value("maximum").toDouble(), 8.0);
    QCOMPARE(row.value("maximumText").toString(), QStringLiteral("8.000"));

    // Excluded and hidden series have no row.
    inspector.setExcludedSeries({curve});
    QTRY_COMPARE(model->count(), 0);
    inspector.setExcludedSeries({});
    QTRY_COMPARE(model->count(), 1);
    inspector.setIncludedSeries({curve});
    QTRY_COMPARE(inspector.validCount(), 1);
    curve->setVisible(false);
    QTRY_COMPARE(model->count(), 0);
    curve->setVisible(true);
    QTRY_COMPARE(inspector.validCount(), 1);
}

void TestPlotInspector::notifications()
{
    auto rig = PlotRig{0, 10, 0, 10};
    rig.addCurve({5, 5});
    QTRY_COMPARE(rig.plot.series().size(), 1);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    QTest::qWait(20);
    auto active = QSignalSpy{&inspector, &PlotInspector::activeChanged};
    auto position = QSignalSpy{&inspector, &PlotInspector::positionChanged};
    auto cursor = QSignalSpy{&inspector, &PlotInspector::cursorChanged};
    auto valid = QSignalSpy{&inspector, &PlotInspector::validCountChanged};
    auto radius = QSignalSpy{&inspector, &PlotInspector::radiusChanged};
    // Nothing observable changes while the pointer is outside the plot.
    for (auto i = 0; i < 5; ++i) {
        rig.x.setViewportMax(11 + i);
        QTest::qWait(5);
    }
    QCOMPARE(active.count() + position.count() + cursor.count() + valid.count(), 0);

    // Each property has its own signal and only fires on a change.
    inspector.setRadius(20);
    inspector.setRadius(20);
    inspector.setRadius(-1);
    QCOMPARE(radius.count(), 1);
    QCOMPARE(inspector.radius(), 20.0);

    rig.hover(rig.pixel(5, 5));
    QTRY_VERIFY(inspector.active());
    QCOMPARE(active.count(), 1);
    QCOMPARE(valid.count(), 1);
    const auto positions = position.count();
    rig.hover(rig.pixel(5, 5));
    inspector.refresh();
    QCOMPARE(position.count(), positions);

    // A disabled inspector goes inactive once and then does nothing.
    inspector.setEnabled(false);
    QTRY_VERIFY(!inspector.active());
    QCOMPARE(inspector.model()->count(), 0);
    QTest::qWait(20);
    auto refreshed = QSignalSpy{&inspector, &PlotInspector::refreshed};
    for (auto i = 0; i < 5; ++i) {
        rig.hover(rig.pixel(5, 5) + QPointF{static_cast<qreal>(i), 0});
        QTest::qWait(5);
    }
    QCOMPARE(refreshed.count(), 0);
    inspector.setEnabled(true);
    QTRY_VERIFY(inspector.active());
}

void TestPlotInspector::cursor()
{
    auto rig = PlotRig{0, 10, 0, 10};
    rig.addCurve({1, 1, 3, 2, 5, 9, 7, 4});
    QTRY_COMPARE(rig.plot.series().size(), 1);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);

    // Following the pointer publishes its data coordinates; a written cursor waits until following stops.
    rig.hover(rig.pixel(2.5, 6));
    inspector.refresh();
    QVERIFY(std::abs(inspector.cursorX() - 2.5) < 1e-9);
    QVERIFY(std::abs(inspector.cursorY() - 6.0) < 1e-9);
    QCOMPARE(inspector.cursorXText(), QStringLiteral("2.500"));
    inspector.setCursorX(8);
    QVERIFY(std::abs(inspector.cursorX() - 2.5) < 1e-9);
    rig.leave();
    inspector.refresh();
    QVERIFY(std::isnan(inspector.cursorX()));

    // A cursor set from code needs no pointer; without a Y it has no horizontal position.
    inspector.setFollowPointer(false);
    inspector.setCursorX(5.2);
    inspector.refresh();
    QVERIFY(inspector.active());
    QCOMPARE(inspector.position().x(), rig.pixel(5.2, 0).x());
    QVERIFY(std::isnan(inspector.position().y()));
    QCOMPARE(inspector.model()->get(0).value("sampleIndex").toInt(), 2);
    QCOMPARE(inspector.cursorYText(), QString{});
    inspector.setCursorY(4);
    inspector.refresh();
    QCOMPARE(inspector.position(), rig.pixel(5.2, 4));

    // Stepping walks the records of the first matching series.
    inspector.stepCursor(1);
    QCOMPARE(inspector.model()->get(0).value("sampleIndex").toInt(), 3);
    QVERIFY(std::abs(inspector.cursorX() - 7.0) < 1e-9);
    inspector.stepCursor(-2);
    QCOMPARE(inspector.model()->get(0).value("sampleIndex").toInt(), 1);
    inspector.stepCursor(-10);
    QCOMPARE(inspector.model()->get(0).value("sampleIndex").toInt(), 0);

    // Outside the viewport the cursor is inactive but kept.
    inspector.setCursorX(50);
    inspector.refresh();
    QVERIFY(!inspector.active());
    QCOMPARE(inspector.cursorX(), 50.0);

    // Stepping from a pointer-driven cursor stops following the pointer.
    inspector.setFollowPointer(true);
    rig.hover(rig.pixel(3, 2));
    inspector.refresh();
    inspector.stepCursor(1);
    QVERIFY(!inspector.followPointer());
    QCOMPARE(inspector.model()->get(0).value("sampleIndex").toInt(), 2);
}

void TestPlotInspector::snapsToClosest()
{
    auto rig = PlotRig{0, 10, 0, 10};
    rig.addCurve({2, 2}, "far");
    rig.addCurve({5.2, 5}, "near");
    QTRY_COMPARE(rig.plot.series().size(), 2);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    inspector.setMode(PlotInspector::NearestXY);
    inspector.setRadius(1000);
    inspector.setSnapToSample(true);
    rig.hover(rig.pixel(5, 5));
    inspector.refresh();
    QCOMPARE(inspector.validCount(), 2);
    // The crosshair moves to the closest sample, not to the first registered series.
    QCOMPARE(inspector.position(), rig.pixel(5.2, 5));
    inspector.setMode(PlotInspector::NearestX);
    inspector.refresh();
    QCOMPARE(inspector.position().x(), rig.pixel(5.2, 5).x());
    QCOMPARE(inspector.position().y(), rig.pixel(5, 5).y());
}

void TestPlotInspector::formatterMutation()
{
    auto rig = PlotRig{0, 10, 0, 10};
    auto* curve = rig.addCurve({5, 3}, "curve");
    auto* victim = rig.addCurve({5, 3}, "victim");
    QTRY_COMPARE(rig.plot.series().size(), 2);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    auto formatter = MutationFormatter{};
    rig.x.ticker()->setTickLabelFormatter(&formatter);
    QTest::qWait(20);
    // The formatter changes the data, deletes a series, and re-enters the inspector mid-refresh.
    formatter.action = [&] {
        curve->setDataNoRange(std::vector<double>{5, 8}, 1);
        delete victim;
        inspector.refresh();
    };
    rig.hover(rig.pixel(5, 5));
    inspector.refresh();
    // The interrupted refresh publishes nothing; in particular no row of the deleted series.
    QVERIFY(!inspector.active());
    QCOMPARE(inspector.model()->count(), 1);
    QTRY_COMPARE(inspector.validCount(), 1);
    QCOMPARE(inspector.model()->get(0).value("series").value<PlotSeries*>(), curve);
    QCOMPARE(inspector.model()->get(0).value("sampleY").toDouble(), 8.0);
    rig.x.ticker()->setTickLabelFormatter(nullptr);
}

void TestPlotInspector::touch()
{
    auto window = QQuickWindow{};
    window.resize(800, 400);
    auto* plot = new InspectionPlot{};
    plot->setParentItem(window.contentItem());
    plot->setSize({800, 400});
    auto x = Axis{};
    auto y = Axis{};
    x.setViewportMin(0);
    x.setViewportMax(10);
    y.setViewportMin(0);
    y.setViewportMax(10);
    plot->setXAxis(&x);
    plot->setYAxis(&y);
    auto* curve = new LineCurve(plot);
    curve->setXAxis(&x);
    curve->setYAxis(&y);
    curve->setData(std::vector<double>{1, 1, 9, 9}, 2);
    auto inspector = PlotInspector{};
    inspector.setPlot(plot);
    // The window stays hidden: synthetic events are still delivered, and the real mouse cannot interfere.
    const auto device = std::unique_ptr<QPointingDevice>{QTest::createTouchDevice()};
    const auto point = plot->plotRect().center().toPoint();
    QTest::mouseMove(&window, QPoint{-20, -20});

    // A tap inspects at the touched position and the cursor stays there after the finger lifts.
    QTest::touchEvent(&window, device.get()).press(0, point, &window);
    QTest::touchEvent(&window, device.get()).release(0, point, &window);
    QTRY_COMPARE(inspector.position().toPoint(), point);
    QVERIFY(inspector.active());
    QCOMPARE(inspector.validCount(), 1);

    // The mouse takes the cursor back as soon as it moves over the plot.
    QTest::mouseMove(&window, point + QPoint{40, 0});
    QTRY_COMPARE(inspector.position().toPoint(), point + QPoint(40, 0));
    QTest::mouseMove(&window, QPoint{-20, -20});
    QTRY_VERIFY(!inspector.active());
}

void TestPlotInspector::tooltipReusesDelegates()
{
    auto scene = TooltipScene{};
    QVERIFY2(scene.tooltip != nullptr, qPrintable(scene.error));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window));

    // Row positions are sampled where the scene graph reads them, with the GUI thread blocked.
    auto frames = 0;
    auto overlapping = 0;
    connect(
        &scene.window, &QQuickWindow::beforeSynchronizing, this,
        [&] {
            auto positions = QList<qreal>{};
            const auto rows = scene.tooltip->childItems().first()->childItems();
            for (const auto* row : rows) {
                if (row->isVisible() && row->height() > 0) {
                    positions.push_back(row->y());
                }
            }
            if (!scene.tooltip->isVisible() || positions.size() < 2) {
                return;
            }
            ++frames;
            overlapping += positions.at(0) == positions.at(1) ? 1 : 0;
        },
        Qt::DirectConnection);

    scene.inspector.setCursorX(5);
    QTRY_COMPARE(scene.inspector.validCount(), 3);
    QTRY_VERIFY(scene.tooltip->isVisible());
    QTest::qWait(50);
    QCOMPARE(scene.tooltip->property("created").toInt(), 3);
    for (auto i = 1; i <= 30; ++i) {
        scene.inspector.setCursorX(5 + i * 0.01);
        QTest::qWait(16);
    }
    // Delegates are reused and their rows never share a position in a rendered frame.
    QCOMPARE(scene.tooltip->property("created").toInt(), 3);
    QVERIFY(frames > 5);
    QCOMPARE(overlapping, 0);
}

void TestPlotInspector::tooltipPlacement()
{
    auto scene = TooltipScene{};
    QVERIFY2(scene.tooltip != nullptr, qPrintable(scene.error));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
    scene.inspector.setCursorX(5);
    QTRY_VERIFY(scene.tooltip->isVisible());
    QTRY_VERIFY(scene.tooltip->x() > scene.inspector.position().x());

    // Near the right edge the tooltip flips to the other side of the cursor instead of covering it.
    scene.inspector.setCursorX(9.7);
    QTRY_VERIFY(scene.tooltip->x() + scene.tooltip->width() < scene.inspector.position().x());

    // Beyond every series' last sample nothing matches and the tooltip hides.
    scene.inspector.setRadius(2);
    scene.inspector.setCursorX(9.97);
    QTRY_COMPARE(scene.inspector.validCount(), 0);
    QVERIFY(scene.inspector.active());
    QTRY_VERIFY(!scene.tooltip->isVisible());
}

QTEST_MAIN(TestPlotInspector)
#include "tst_plot_inspector.moc"
