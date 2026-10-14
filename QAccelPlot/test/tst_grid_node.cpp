//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/grid/Grid.hpp"
#include "QAccelPlot/grid/GridNode.hpp"

#include <QtTest/QtTest>

class GridNodeTest : public QObject {
    Q_OBJECT
private slots:
    void logTickControls_data();
    void logTickControls();
    void disabledTicksProduceNoGrid_data();
    void disabledTicksProduceNoGrid();
    void hiddenGridClearsGeometry();
    void collapsedRangeProducesNoGrid_data();
    void collapsedRangeProducesNoGrid();
};

void GridNodeTest::logTickControls_data()
{
    QTest::addColumn<int>("majorCount");
    QTest::addColumn<int>("minorCount");
    QTest::newRow("no-major") << 0 << 10;
    QTest::newRow("negative-major") << -1 << 10;
    QTest::newRow("no-minor") << 5 << 0;
    QTest::newRow("negative-minor") << 5 << -1;
    QTest::newRow("enabled") << 5 << 10;
}

void GridNodeTest::logTickControls()
{
    QFETCH(int, majorCount);
    QFETCH(int, minorCount);
    auto axis = QAccelPlot::Axis{};
    axis.setLogScale(true);
    axis.setViewportMin(1.0);
    axis.setViewportMax(100.0);
    axis.ticker()->setTickCount(majorCount);
    axis.ticker()->setSubtickCount(minorCount);
    auto grid = QAccelPlot::Grid{};
    auto node = QAccelPlot::GridNode{};
    node.update(&grid, &axis, nullptr, QRectF{0, 0, 100, 100});
    QCOMPARE(node.childCount(), majorCount <= 0 ? 0 : minorCount <= 0 ? 1 : 2);
}

void GridNodeTest::disabledTicksProduceNoGrid_data()
{
    QTest::addColumn<int>("tickCount");
    QTest::newRow("zero") << 0;
    QTest::newRow("negative") << -1;
}

void GridNodeTest::disabledTicksProduceNoGrid()
{
    QFETCH(int, tickCount);
    auto axis = QAccelPlot::Axis{};
    axis.setViewportMax(100.0);
    axis.ticker()->setTickCount(tickCount);
    axis.ticker()->setSubtickCount(4);
    auto grid = QAccelPlot::Grid{};
    auto node = QAccelPlot::GridNode{};

    node.update(&grid, &axis, nullptr, QRectF{0, 0, 100, 100});

    QCOMPARE(node.childCount(), 0);
    axis.ticker()->setTickCount(5);
    node.update(&grid, &axis, nullptr, QRectF{0, 0, 100, 100});
    QVERIFY(node.childCount() > 0);
}

void GridNodeTest::hiddenGridClearsGeometry()
{
    auto axis = QAccelPlot::Axis{};
    auto grid = QAccelPlot::Grid{};
    auto node = QAccelPlot::GridNode{};
    node.update(&grid, &axis, nullptr, QRectF{0, 0, 100, 100});
    QVERIFY(node.childCount() > 0);

    grid.setGridVisible(false);
    grid.setSubGridVisible(false);
    node.update(&grid, &axis, nullptr, QRectF{0, 0, 100, 100});

    QCOMPARE(node.childCount(), 0);
}

void GridNodeTest::collapsedRangeProducesNoGrid_data()
{
    QTest::addColumn<double>("coordinate");
    QTest::newRow("zero") << 0.0;
    QTest::newRow("large") << 1e30;
}

void GridNodeTest::collapsedRangeProducesNoGrid()
{
    QFETCH(double, coordinate);
    auto axis = QAccelPlot::Axis{};
    axis.setViewportMin(coordinate);
    axis.setViewportMax(coordinate);
    auto grid = QAccelPlot::Grid{};
    auto node = QAccelPlot::GridNode{};
    node.update(&grid, &axis, nullptr, QRectF{0, 0, 100, 100});
    QCOMPARE(node.childCount(), 0);
}

QTEST_MAIN(GridNodeTest)
#include "tst_grid_node.moc"
