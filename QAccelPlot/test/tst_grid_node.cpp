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
    void disabledTicksProduceNoGrid_data();
    void disabledTicksProduceNoGrid();
    void hiddenGridClearsGeometry();
};

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

QTEST_MAIN(GridNodeTest)
#include "tst_grid_node.moc"
