//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/axis/AxisTickPainter.hpp"
#include "QAccelPlot/grid/Grid.hpp"
#include "QAccelPlot/grid/GridNode.hpp"

#include <QSGFlatColorMaterial>
#include <QSGGeometryNode>
#include <QtTest/QtTest>

#include <algorithm>
#include <cmath>

namespace {

// Returns the x centers of the vertical grid lines drawn in color.
QList<qreal> verticalLineCenters(const QSGNode& node, const QColor& color)
{
    static constexpr auto kVerticesPerRect = 6;
    auto centers = QList<qreal>{};
    for (auto* child = node.firstChild(); child != nullptr; child = child->nextSibling()) {
        const auto* geometryNode = static_cast<const QSGGeometryNode*>(child);
        if (static_cast<const QSGFlatColorMaterial*>(geometryNode->material())->color() != color) {
            continue;
        }
        const auto* geometry = geometryNode->geometry();
        const auto* vertices = geometry->vertexDataAsPoint2D();
        for (auto i = 0; i + 1 < geometry->vertexCount(); i += kVerticesPerRect) {
            centers.append((vertices[i].x + vertices[i + 1].x) / 2.0);
        }
    }
    std::sort(centers.begin(), centers.end());
    return centers;
}

QList<qreal> pixelPositions(const QAccelPlot::Axis& axis, const QList<qreal>& values, const QRectF& plotRect)
{
    auto pixels = QList<qreal>{};
    for (const auto value : values) {
        pixels.append(plotRect.x() + axis.coordToPixel(value, plotRect.width()));
    }
    std::sort(pixels.begin(), pixels.end());
    return pixels;
}

void comparePositions(const QList<qreal>& actual, const QList<qreal>& expected)
{
    static constexpr auto kPixelTolerance = 1e-3;
    QCOMPARE(actual.size(), expected.size());
    for (auto i = 0; i < actual.size(); ++i) {
        QVERIFY2(std::abs(actual.at(i) - expected.at(i)) < kPixelTolerance,
            qPrintable(QStringLiteral("line %1 at %2, tick at %3").arg(i).arg(actual.at(i)).arg(expected.at(i))));
    }
}

}

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
    void gridLinesMatchAxisTicks_data();
    void gridLinesMatchAxisTicks();
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

void GridNodeTest::gridLinesMatchAxisTicks_data()
{
    QTest::addColumn<bool>("logScale");
    QTest::addColumn<double>("viewportMin");
    QTest::addColumn<double>("viewportMax");
    QTest::addColumn<int>("tickCount");
    QTest::addColumn<int>("subtickCount");
    QTest::newRow("linear-ticks-on-both-edges") << false << 0.0 << 1.0 << 5 << 1;
    QTest::newRow("linear-edge-tick-rounds-past-viewport") << false << 0.0 << 0.6 << 5 << 4;
    QTest::newRow("linear-offset") << false << 0.13 << 0.87 << 5 << 3;
    QTest::newRow("linear-reversed") << false << 1.0 << -1.0 << 4 << 1;
    QTest::newRow("linear-no-subticks") << false << -3.0 << 7.0 << 5 << 0;
    QTest::newRow("log-decades") << true << 1.0 << 100.0 << 5 << 10;
    QTest::newRow("log-partial-decades") << true << 2.0 << 500.0 << 5 << 10;
    QTest::newRow("log-no-subticks") << true << 1.0 << 1000.0 << 5 << 0;
    QTest::newRow("log-collapsed") << true << 10.0 << 10.0 << 5 << 10;
}

void GridNodeTest::gridLinesMatchAxisTicks()
{
    QFETCH(bool, logScale);
    QFETCH(double, viewportMin);
    QFETCH(double, viewportMax);
    QFETCH(int, tickCount);
    QFETCH(int, subtickCount);
    auto axis = QAccelPlot::Axis{};
    axis.setLogScale(logScale);
    axis.setViewportMin(viewportMin);
    axis.setViewportMax(viewportMax);
    axis.ticker()->setTickCount(tickCount);
    axis.ticker()->setSubtickCount(subtickCount);
    auto grid = QAccelPlot::Grid{};
    grid.setGridColor(Qt::red);
    grid.setSubGridColor(Qt::blue);
    const auto plotRect = QRectF{10, 20, 1000, 100};
    auto node = QAccelPlot::GridNode{};

    node.update(&grid, &axis, nullptr, plotRect);

    const auto ticks = QAccelPlot::AxisTickPainter::computeTicks(axis.viewportMin(), axis.viewportMax(), axis.logScale(), axis.ticker());
    auto majorValues = QList<qreal>{};
    for (const auto& tick : ticks.majorTicks) {
        majorValues.append(tick.value);
    }
    comparePositions(verticalLineCenters(node, Qt::red), pixelPositions(axis, majorValues, plotRect));
    comparePositions(verticalLineCenters(node, Qt::blue), pixelPositions(axis, ticks.subtickValues, plotRect));
}

QTEST_MAIN(GridNodeTest)
#include "tst_grid_node.moc"
