//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"
#include "WindowPlacement.hpp"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QtTest/QtTest>

#include <memory>

namespace QAccelPlot {

namespace {

// A rectangle at data (2, 2)–(8, 8) beneath a horizontal line at y = 5.
constexpr auto kScene = R"(
import QtQuick
import QAccelPlot

PlotView {
    id: plot
    width: 400
    height: 300

    xAxis: Axis { viewportMin: 0; viewportMax: 10 }
    yAxis: Axis { viewportMin: 0; viewportMax: 10 }

    RectangleSeries {
        objectName: "band"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        Component.onCompleted: setData([{ x1: 2, y1: 2, x2: 8, y2: 8 }])
    }

    LineCurve {
        objectName: "line"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        Component.onCompleted: setData([Qt.point(0, 5), Qt.point(10, 5)])
    }
}
)";

// One series with a record at data (5, 5) and hoverRadius 4.
constexpr auto kRadiusScene = R"(
import QtQuick
import QAccelPlot

PlotView {
    id: plot
    width: 400
    height: 300

    xAxis: Axis { viewportMin: 0; viewportMax: 10 }
    yAxis: Axis { viewportMin: 0; viewportMax: 10 }

    %1 {
        objectName: "series"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        hoverRadius: 4
        Component.onCompleted: %2
    }
}
)";

std::unique_ptr<QObject> createScene(QQmlEngine& engine, const QByteArray& source, QQuickWindow& window)
{
    auto component = QQmlComponent{&engine};
    component.setData(source, QUrl{});
    auto root = std::unique_ptr<QObject>{component.create()};
    auto* plot = qobject_cast<QAccelPlot*>(root.get());
    if (!plot) {
        qWarning() << component.errorString();
        return nullptr;
    }
    plot->setParentItem(window.contentItem());
    QAccelPlotTest::moveAwayFromCursor(window);
    window.show();
    if (!QTest::qWaitForWindowExposed(&window)) {
        return nullptr;
    }
    return root;
}

} // namespace

class LineCurveHoverTest : public QObject {
    Q_OBJECT

private slots:
    void zeroHoverRadiusPassesHoverToSeriesBeneath();
    void hoverRadiusChangeUpdatesHoveredIndex_data();
    void hoverRadiusChangeUpdatesHoveredIndex();
};

void LineCurveHoverTest::zeroHoverRadiusPassesHoverToSeriesBeneath()
{
    auto window = QQuickWindow{};
    window.resize(400, 300);
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(kScene, QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    auto* plot = qobject_cast<QAccelPlot*>(root.get());
    QVERIFY2(plot, qPrintable(component.errorString()));
    plot->setParentItem(window.contentItem());
    QAccelPlotTest::moveAwayFromCursor(window);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* band = plot->findChild<QQuickItem*>(QStringLiteral("band"));
    auto* line = plot->findChild<QQuickItem*>(QStringLiteral("line"));
    QVERIFY(band);
    QVERIFY(line);
    const auto pixel = [plot](const qreal x, const qreal y) { return QPoint{qRound(plot->dataToPixelX(x)), qRound(plot->dataToPixelY(y))}; };

    QTest::mouseMove(&window, pixel(5.0, 5.0));
    QVERIFY(line->property("hovered").toBool());
    QCOMPARE(band->property("hoveredIndex").toInt(), -1);

    // The cursor stays put: the change alone moves hover to the rectangle.
    line->setProperty("hoverRadius", 0.0);
    QTRY_VERIFY(!line->property("hovered").toBool());
    QCOMPARE(band->property("hoveredIndex").toInt(), 0);

    QTest::mouseMove(&window, pixel(3.0, 5.0));
    QVERIFY(!line->property("hovered").toBool());
    QCOMPARE(band->property("hoveredIndex").toInt(), 0);

    line->setProperty("hoverRadius", 10.0);
    QTRY_VERIFY(line->property("hovered").toBool());
    QCOMPARE(band->property("hoveredIndex").toInt(), -1);
}

void LineCurveHoverTest::hoverRadiusChangeUpdatesHoveredIndex_data()
{
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("setData");
    QTest::newRow("PointCloud") << QStringLiteral("PointCloud") << QStringLiteral("setData([Qt.point(5, 5)])");
    QTest::newRow("ErrorBarSeries") << QStringLiteral("ErrorBarSeries") << QStringLiteral("setData([5], [5], [3], [7])");
}

void LineCurveHoverTest::hoverRadiusChangeUpdatesHoveredIndex()
{
    QFETCH(QString, type);
    QFETCH(QString, setData);
    auto window = QQuickWindow{};
    window.resize(400, 300);
    auto engine = QQmlEngine{};
    const auto root = createScene(engine, QString::fromLatin1(kRadiusScene).arg(type, setData).toUtf8(), window);
    QVERIFY(root);
    auto* plot = qobject_cast<QAccelPlot*>(root.get());
    auto* series = plot->findChild<QQuickItem*>(QStringLiteral("series"));
    QVERIFY(series);

    // Qt drops a move to the global position of the previous row's last move.
    QTest::mouseMove(&window, QPoint{1, 1});
    // 10 pixels right of the record.
    QTest::mouseMove(&window, QPoint{qRound(plot->dataToPixelX(5.0)) + 10, qRound(plot->dataToPixelY(5.0))});
    QCOMPARE(series->property("hoveredIndex").toInt(), -1);

    // The cursor stays put: the change alone picks the record.
    series->setProperty("hoverRadius", 14.0);
    QTRY_COMPARE(series->property("hoveredIndex").toInt(), 0);
    QVERIFY(series->property("hovered").toBool());

    series->setProperty("hoverRadius", 4.0);
    QTRY_COMPARE(series->property("hoveredIndex").toInt(), -1);
    QVERIFY(!series->property("hovered").toBool());
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::LineCurveHoverTest)
#include "tst_line_curve_hover.moc"
