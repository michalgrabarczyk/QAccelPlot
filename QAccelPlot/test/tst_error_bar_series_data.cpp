//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "HoverEvents.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/inspection/InspectionResult.hpp"
#include "QAccelPlot/inspection/SeriesInspection.hpp"
#include "QAccelPlot/materials/ErrorBarMaterial.hpp"
#include "QAccelPlot/series/ErrorBarSeries.hpp"

#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QtTest/QtTest>

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();
constexpr auto kInf = std::numeric_limits<double>::infinity();

class TestableErrorBars final : public ErrorBarSeries {
public:
    using ErrorBarSeries::hoverEnterEvent;
    using ErrorBarSeries::hoverLeaveEvent;
    using ErrorBarSeries::hoverMoveEvent;
    using ErrorBarSeries::updatePaintNode;
};

// Error bars bound to axes spanning 0..10 in a 100 × 100 px item, so one data unit is 10 px.
struct ErrorBarFixture {
    ErrorBarFixture()
    {
        for (auto* axis : {&xAxis, &yAxis}) {
            axis->setViewportMin(0.0);
            axis->setViewportMax(10.0);
        }
        yAxis.setOrientation(Qt::Vertical);
        bars.setParentItem(window.contentItem());
        bars.setSize({100.0, 100.0});
        bars.setPlotRect({0.0, 0.0, 100.0, 100.0});
        bars.setXAxis(&xAxis);
        bars.setYAxis(&yAxis);
    }

    QPointF pixel(const double x, const double y) const
    {
        return {xAxis.coordToPixel(x, 100.0), yAxis.coordToPixel(y, 100.0)};
    }

    void hover(const QEvent::Type type, const QPointF& position)
    {
        auto event = QAccelPlotTest::hoverEvent(type, position, position);
        if (type == QEvent::HoverEnter) {
            bars.hoverEnterEvent(&event);
        } else if (type == QEvent::HoverMove) {
            bars.hoverMoveEvent(&event);
        } else {
            bars.hoverLeaveEvent(&event);
        }
    }

    QSGGeometryNode* paint()
    {
        node.reset(bars.updatePaintNode(node.release(), nullptr));
        return static_cast<QSGGeometryNode*>(node.get());
    }

    QQuickWindow window;
    Axis xAxis;
    Axis yAxis;
    TestableErrorBars bars;
    std::unique_ptr<QSGNode> node;
};

bool hasErrorBar(const QVariantMap& bar, const double x, const double y, const double low, const double high)
{
    const auto same = [&bar](const char* key, const double expected) {
        const auto actual = bar.value(QString::fromLatin1(key)).toDouble();
        return std::isnan(expected) ? std::isnan(actual) : qFuzzyCompare(actual, expected);
    };
    return !bar.isEmpty() && same("x", x) && same("y", y) && same("low", low) && same("high", high);
}

} // namespace

class ErrorBarSeriesDataTest : public QObject {
    Q_OBJECT

private slots:
    void setDataStoresQuadruplesUpToTheShortestList();
    void errorDataGivesBoundsAroundTheValue();
    void appendingAddsOneDataPoint();
    void dataRangesCoverValuesAndBounds();
    void hitTestFindsLinesAndCaps();
    void nearestErrorBarWinsAndTiesGoToHigherIndex();
    void invalidAndUnboundedBounds();
    void logAxisClipsNonPositiveBounds();
    void offsetShiftsErrorBars();
    void horizontalErrorBarsLieAlongX();
    void hoverEventsTrackErrorBarUnderCursor();
    void inspectionReturnsRecords();
    void paintReservesVerticesAndSetsUniforms();
};

void ErrorBarSeriesDataTest::setDataStoresQuadruplesUpToTheShortestList()
{
    auto bars = ErrorBarSeries{};
    auto spy = QSignalSpy{&bars, &ErrorBarSeries::countChanged};
    bars.setData(QList<qreal>{1, 2, 3}, QList<qreal>{4, 5, 6}, QList<qreal>{3, 4}, QList<qreal>{5, 6, 7});

    QCOMPARE(bars.count(), 2);
    QCOMPARE(spy.count(), 1);
    QVERIFY(hasErrorBar(bars.errorBarAt(1), 2, 5, 4, 6));
    QVERIFY(bars.errorBarAt(2).isEmpty());
    QVERIFY(bars.errorBarAt(-1).isEmpty());

    bars.setData(std::vector<double>{1, 2, 3, 4, 5, 6, 7, 8}, 2);
    QVERIFY(hasErrorBar(bars.errorBarAt(1), 5, 6, 7, 8));

    bars.clearData();
    QCOMPARE(bars.count(), 0);
}

void ErrorBarSeriesDataTest::errorDataGivesBoundsAroundTheValue()
{
    auto bars = ErrorBarSeries{};
    bars.setErrorData(QList<qreal>{1, 2}, QList<qreal>{10, 20}, QList<qreal>{0.5, -2});
    QVERIFY(hasErrorBar(bars.errorBarAt(0), 1, 10, 9.5, 10.5));
    QVERIFY(hasErrorBar(bars.errorBarAt(1), 2, 20, 18, 22));

    bars.setErrorData(std::vector<double>{1}, std::vector<double>{10}, std::vector<double>{1}, std::vector<double>{3});
    QVERIFY(hasErrorBar(bars.errorBarAt(0), 1, 10, 9, 13));

    bars.setErrorData(QList<qreal>{1}, QList<qreal>{10}, QList<qreal>{kNaN}, QList<qreal>{2});
    QVERIFY(hasErrorBar(bars.errorBarAt(0), 1, 10, kNaN, 12));

    // Horizontal error bars have their bounds around x.
    bars.setOrientation(Qt::Horizontal);
    bars.setErrorData(std::vector<double>{1}, std::vector<double>{10}, std::vector<double>{0.25});
    QVERIFY(hasErrorBar(bars.errorBarAt(0), 1, 10, 0.75, 1.25));
}

void ErrorBarSeriesDataTest::appendingAddsOneDataPoint()
{
    auto bars = ErrorBarSeries{};
    auto spy = QSignalSpy{&bars, &ErrorBarSeries::countChanged};
    bars.appendData(1, 2, 1.5, 3);
    bars.appendErrorData(2, 4, 1);
    bars.appendErrorData(3, 6, 1, -2);

    QCOMPARE(bars.count(), 3);
    QCOMPARE(spy.count(), 3);
    QVERIFY(hasErrorBar(bars.errorBarAt(0), 1, 2, 1.5, 3));
    QVERIFY(hasErrorBar(bars.errorBarAt(1), 2, 4, 3, 5));
    QVERIFY(hasErrorBar(bars.errorBarAt(2), 3, 6, 5, 8));
}

void ErrorBarSeriesDataTest::dataRangesCoverValuesAndBounds()
{
    auto fixture = ErrorBarFixture{};
    auto& bars = fixture.bars;
    bars.setData(QList<qreal>{1, 4}, QList<qreal>{5, 6}, QList<qreal>{2, kNaN}, QList<qreal>{8, 9});
    QCOMPARE(bars.xDataRange()->min, 1.0);
    QCOMPARE(bars.xDataRange()->max, 4.0);
    QCOMPARE(bars.yDataRange()->min, 2.0);
    QCOMPARE(bars.yDataRange()->max, 9.0);

    // Appending widens the ranges without a rescan.
    bars.appendData(-1, 5, -3, 5);
    QCOMPARE(bars.xDataRange()->min, -1.0);
    QCOMPARE(bars.yDataRange()->min, -3.0);

    // Horizontal error bars report their bounds on X.
    bars.setOrientation(Qt::Horizontal);
    QCOMPARE(bars.xDataRange()->min, -3.0);
    QCOMPARE(bars.xDataRange()->max, 9.0);
    QCOMPARE(bars.yDataRange()->min, 5.0);
    QCOMPARE(bars.yDataRange()->max, 6.0);
}

void ErrorBarSeriesDataTest::hitTestFindsLinesAndCaps()
{
    auto fixture = ErrorBarFixture{};
    auto& bars = fixture.bars;
    bars.setLineWidth(2);
    bars.setCapLength(10);
    bars.setHoverRadius(0);
    bars.setData(QList<qreal>{5}, QList<qreal>{5}, QList<qreal>{3}, QList<qreal>{7});

    const auto center = fixture.pixel(5, 5);
    const auto top = fixture.pixel(5, 7);
    QCOMPARE(bars.errorBarIndexAt(center), 0);
    QCOMPARE(bars.errorBarIndexAt(center + QPointF{0.9, 0}), 0);
    QCOMPARE(bars.errorBarIndexAt(center + QPointF{1.5, 0}), -1);
    QCOMPARE(bars.errorBarIndexAt(top + QPointF{4.5, 0}), 0);
    QCOMPARE(bars.errorBarIndexAt(top + QPointF{5.5, 0}), -1);
    QCOMPARE(bars.errorBarIndexAt(top - QPointF{0, 2}), -1);
    QVERIFY(bars.contains(center));
    QVERIFY(!bars.contains(center + QPointF{3, 0}));

    bars.setHoverRadius(6);
    QCOMPARE(bars.errorBarIndexAt(center + QPointF{6.5, 0}), 0);
    QCOMPARE(bars.errorBarIndexAt(center + QPointF{7.5, 0}), -1);
    QCOMPARE(bars.errorBarIndexAt(top - QPointF{0, 6.5}), 0);

    // Without caps only the line is hit.
    bars.setHoverRadius(0);
    bars.setCapLength(0);
    QCOMPARE(bars.errorBarIndexAt(top + QPointF{4.5, 0}), -1);
    QCOMPARE(bars.errorBarIndexAt(top), 0);

    bars.setLineWidth(0);
    QCOMPARE(bars.errorBarIndexAt(center), -1);
}

void ErrorBarSeriesDataTest::nearestErrorBarWinsAndTiesGoToHigherIndex()
{
    auto fixture = ErrorBarFixture{};
    auto& bars = fixture.bars;
    bars.setHoverRadius(20);
    bars.setData(QList<qreal>{4, 6}, QList<qreal>{5, 5}, QList<qreal>{3, 3}, QList<qreal>{7, 7});

    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(4.8, 5)), 0);
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(5.2, 5)), 1);
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(5, 5)), 1);
}

void ErrorBarSeriesDataTest::invalidAndUnboundedBounds()
{
    auto fixture = ErrorBarFixture{};
    auto& bars = fixture.bars;
    bars.setHoverRadius(0);
    bars.setCapLength(10);
    // A NaN low leaves out the lower half, an infinite high reaches past the plot edge, and the
    // last two are not drawn: a NaN position and a NaN value.
    bars.setData(QList<qreal>{2, 5, kNaN, 8}, QList<qreal>{5, 5, 5, kNaN}, QList<qreal>{kNaN, 3, 3, 3}, QList<qreal>{7, kInf, 7, 7});

    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(2, 6)), 0);
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(2, 4)), -1);
    // The line runs from the value without a cap there.
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(2, 5) + QPointF{4, 0}), -1);
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(2, 7) + QPointF{4, 0}), 0);

    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(5, 9.9)), 1);
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(8, 5)), -1);
    QCOMPARE(bars.inspection()->recordAt(2).status, InspectionStatus::NoMatch);
    QCOMPARE(bars.inspection()->recordAt(3).status, InspectionStatus::NoMatch);
}

void ErrorBarSeriesDataTest::logAxisClipsNonPositiveBounds()
{
    auto fixture = ErrorBarFixture{};
    auto& bars = fixture.bars;
    fixture.yAxis.setViewportMin(1);
    fixture.yAxis.setViewportMax(100);
    fixture.yAxis.setLogScale(true);
    bars.setHoverRadius(0);
    bars.setCapLength(10);
    bars.setData(QList<qreal>{5, 8}, QList<qreal>{10, -1}, QList<qreal>{-5, 2}, QList<qreal>{50, 20});

    // The non-positive low reaches past the bottom edge.
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(5, 1.1)), 0);
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(5, 50) + QPointF{4, 0}), 0);
    QVERIFY(fixture.bars.yDataRange());
    QCOMPARE(bars.yDataRange()->min, 2.0);
    // A non-positive value still draws its bounds.
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(8, 5)), 1);
}

void ErrorBarSeriesDataTest::offsetShiftsErrorBars()
{
    auto fixture = ErrorBarFixture{};
    auto& bars = fixture.bars;
    bars.setHoverRadius(0);
    bars.setData(QList<qreal>{5}, QList<qreal>{5}, QList<qreal>{3}, QList<qreal>{7});
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(5, 5)), 0);

    bars.setOffset(1);
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(5, 5)), -1);
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(6, 5)), 0);
    // Auto-ranging ignores the offset.
    QCOMPARE(bars.xDataRange()->max, 5.0);
}

void ErrorBarSeriesDataTest::horizontalErrorBarsLieAlongX()
{
    auto fixture = ErrorBarFixture{};
    auto& bars = fixture.bars;
    bars.setOrientation(Qt::Horizontal);
    bars.setHoverRadius(0);
    bars.setCapLength(10);
    bars.setOffset(1);
    bars.setData(QList<qreal>{5}, QList<qreal>{5}, QList<qreal>{3}, QList<qreal>{7});

    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(4, 6)), 0);
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(4, 5)), -1);
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(3, 6) + QPointF{0, 4}), 0);
    QCOMPARE(bars.errorBarIndexAt(fixture.pixel(2, 6)), -1);
}

void ErrorBarSeriesDataTest::hoverEventsTrackErrorBarUnderCursor()
{
    auto fixture = ErrorBarFixture{};
    auto& bars = fixture.bars;
    bars.setData(QList<qreal>{2, 8}, QList<qreal>{5, 5}, QList<qreal>{3, 3}, QList<qreal>{7, 7});
    auto spy = QSignalSpy{&bars, &ErrorBarSeries::hoveredIndexChanged};
    auto hoveredSpy = QSignalSpy{&bars, &ErrorBarSeries::hoveredChanged};

    fixture.hover(QEvent::HoverEnter, fixture.pixel(2, 5));
    QCOMPARE(bars.hoveredIndex(), 0);
    QVERIFY(bars.hovered());
    fixture.hover(QEvent::HoverMove, fixture.pixel(8, 4));
    QCOMPARE(bars.hoveredIndex(), 1);
    fixture.hover(QEvent::HoverMove, fixture.pixel(5, 5));
    QCOMPARE(bars.hoveredIndex(), -1);
    QVERIFY(!bars.hovered());
    fixture.hover(QEvent::HoverMove, fixture.pixel(8, 4));
    fixture.hover(QEvent::HoverLeave, fixture.pixel(8, 4));
    QCOMPARE(bars.hoveredIndex(), -1);
    QCOMPARE(spy.count(), 5);
    QCOMPARE(hoveredSpy.count(), 4);

    fixture.hover(QEvent::HoverEnter, fixture.pixel(8, 4));
    bars.setData(QList<qreal>{2}, QList<qreal>{5}, QList<qreal>{3}, QList<qreal>{7});
    QCOMPARE(bars.hoveredIndex(), -1);
    QVERIFY(!bars.hovered());

    fixture.hover(QEvent::HoverEnter, fixture.pixel(2, 5));
    QCOMPARE(bars.hoveredIndex(), 0);
    bars.setHoverEnabled(false);
    QCOMPARE(bars.hoveredIndex(), -1);
    QVERIFY(!bars.hovered());
    QVERIFY(!bars.acceptHoverEvents());
}

void ErrorBarSeriesDataTest::inspectionReturnsRecords()
{
    auto fixture = ErrorBarFixture{};
    auto& bars = fixture.bars;
    bars.setData(QList<qreal>{2, 8}, QList<qreal>{5, 5}, QList<qreal>{3, 4}, QList<qreal>{7, 6});

    const auto record = bars.inspection()->recordAt(1);
    QCOMPARE(record.status, InspectionStatus::Ready);
    QCOMPARE(record.index, 1);
    QVERIFY(hasErrorBar(record.fields, 8, 5, 4, 6));
    QCOMPARE(bars.inspection()->recordAt(2).status, InspectionStatus::InvalidArgument);

    const auto hit = bars.inspection()->recordAtPosition(fixture.pixel(2, 6));
    QCOMPARE(hit.status, InspectionStatus::Ready);
    QCOMPARE(hit.index, 0);
    QVERIFY(!bars.inspection()->recordAtPosition(fixture.pixel(5, 6)).valid());
}

void ErrorBarSeriesDataTest::paintReservesVerticesAndSetsUniforms()
{
    auto fixture = ErrorBarFixture{};
    auto& bars = fixture.bars;
    bars.setData(QList<qreal>{2, 8}, QList<qreal>{5, 5}, QList<qreal>{3, 4}, QList<qreal>{7, 6});
    bars.setOrientation(Qt::Horizontal);
    bars.setLineWidth(3);
    bars.setCapLength(12);
    bars.setOffset(0.5);
    bars.setColor(Qt::red);
    bars.setHoverColor(QColor{});
    auto* node = fixture.paint();
    if (!node) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }

    const auto* material = static_cast<ErrorBarMaterial*>(node->material());
    QCOMPARE(node->geometry()->vertexCount(), 256 * ErrorBarMaterial::kVerticesPerErrorBar);
    QCOMPARE(material->errorBarCount, 2.0f);
    QCOMPARE(material->lineWidth, 3.0f);
    QCOMPARE(material->capLength, 12.0f);
    QCOMPARE(material->offset, 0.5f);
    QCOMPARE(material->horizontal, 1.0f);
    // An invalid hover color draws the hovered error bar in the line color.
    QCOMPARE(material->hoverColor, QColor{Qt::red});
    QVERIFY(material->dataTexture);

    for (auto i = 0; i < 300; ++i) {
        bars.appendData(i, i, i - 1, i + 1);
    }
    node = fixture.paint();
    QCOMPARE(node->geometry()->vertexCount(), 512 * ErrorBarMaterial::kVerticesPerErrorBar);
    QCOMPARE(static_cast<ErrorBarMaterial*>(node->material())->errorBarCount, 302.0f);

    bars.clearData();
    QVERIFY(!fixture.paint());
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::ErrorBarSeriesDataTest)
#include "tst_error_bar_series_data.moc"
