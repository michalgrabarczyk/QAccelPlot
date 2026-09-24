//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/annotations/DataAnchor.hpp"
#include "QAccelPlot/axis/Axis.hpp"

#include <QtTest/QtTest>

namespace QAccelPlot {

class DataAnchorTest : public QObject {
    Q_OBJECT

private slots:
    void destroyedAxisIsClearedInsteadOfDangling();
    void geometryFollowsDataCoordinates();
    void replacedAxesNoLongerMoveTheAnchor();
    void containsExpandsByHoverThreshold();
};

namespace {

void setRange(Axis& axis, const qreal min, const qreal max)
{
    axis.setViewportMin(min);
    axis.setViewportMax(max);
}

} // namespace

void DataAnchorTest::destroyedAxisIsClearedInsteadOfDangling()
{
    auto anchor = DataAnchor{};
    auto* xAxis = new Axis{};
    auto* yAxis = new Axis{&anchor};
    xAxis->setViewportMin(0.0);
    xAxis->setViewportMax(100.0);
    yAxis->setViewportMin(0.0);
    yAxis->setViewportMax(100.0);

    anchor.setXAxis(xAxis);
    anchor.setYAxis(yAxis);
    anchor.setPlotRect(QRectF(0, 0, 200, 100));
    anchor.setDataX1(10.0);
    anchor.setDataY1(10.0);
    anchor.setDataX2(20.0);
    anchor.setDataY2(20.0);

    delete xAxis;

    QCOMPARE(anchor.xAxis(), nullptr);

    // Must not dereference the destroyed axis.
    anchor.setDataX2(30.0);
}

void DataAnchorTest::geometryFollowsDataCoordinates()
{
    auto xAxis = Axis{nullptr, Axis::Bottom};
    auto yAxis = Axis{nullptr, Axis::Left};
    setRange(xAxis, 0.0, 100.0);
    setRange(yAxis, 0.0, 100.0);
    auto anchor = DataAnchor{};
    anchor.setXAxis(&xAxis);
    anchor.setYAxis(&yAxis);
    anchor.setPlotRect(QRectF{10, 20, 200, 100});
    anchor.setDataX1(10.0);
    anchor.setDataY1(10.0);
    anchor.setDataX2(60.0);
    anchor.setDataY2(50.0);

    QCOMPARE(anchor.plotRect(), QRectF(10, 20, 200, 100));
    QCOMPARE(QRectF(anchor.position(), anchor.size()), QRectF(30, 70, 100, 40));

    setRange(xAxis, 0.0, 200.0);
    QCOMPARE(QRectF(anchor.position(), anchor.size()), QRectF(20, 70, 50, 40));
}

void DataAnchorTest::replacedAxesNoLongerMoveTheAnchor()
{
    auto oldXAxis = Axis{nullptr, Axis::Bottom};
    auto oldYAxis = Axis{nullptr, Axis::Left};
    auto newXAxis = Axis{nullptr, Axis::Bottom};
    auto newYAxis = Axis{nullptr, Axis::Left};
    for (auto* axis : {&oldXAxis, &oldYAxis, &newXAxis, &newYAxis}) {
        setRange(*axis, 0.0, 100.0);
    }
    auto anchor = DataAnchor{};
    anchor.setXAxis(&oldXAxis);
    anchor.setYAxis(&oldYAxis);
    anchor.setXAxis(&newXAxis);
    anchor.setYAxis(&newYAxis);
    QCOMPARE(anchor.xAxis(), &newXAxis);
    QCOMPARE(anchor.yAxis(), &newYAxis);
    anchor.setPlotRect(QRectF{0, 0, 100, 100});
    anchor.setDataX2(50.0);
    anchor.setDataY2(50.0);
    const auto geometry = QRectF(anchor.position(), anchor.size());

    setRange(oldXAxis, 0.0, 10.0);
    setRange(oldYAxis, 0.0, 10.0);

    QCOMPARE(QRectF(anchor.position(), anchor.size()), geometry);
}

void DataAnchorTest::containsExpandsByHoverThreshold()
{
    auto anchor = DataAnchor{};
    anchor.setSize(QSizeF{20, 10});
    anchor.setHoverThreshold(4.0);
    const auto& item = static_cast<const QQuickItem&>(anchor);

    QVERIFY(item.contains(QPointF{10, 5}));
    QVERIFY(item.contains(QPointF{-3, 13}));
    QVERIFY(!item.contains(QPointF{-5, 5}));
    QVERIFY(!item.contains(QPointF{10, 15}));
}

} // namespace QAccelPlot

using QAccelPlot::DataAnchorTest;
QTEST_MAIN(DataAnchorTest)
#include "tst_data_anchor.moc"
