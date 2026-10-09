//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/materials/BarMaterial.hpp"
#include "QAccelPlot/series/BarSeries.hpp"

#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QtTest/QtTest>

#include <array>
#include <limits>
#include <memory>
#include <thread>
#include <vector>

namespace QAccelPlot {

namespace {

constexpr auto kInf = std::numeric_limits<double>::infinity();
constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();

class RenderableBars final : public BarSeries {
public:
    using BarSeries::updatePaintNode;
};

QVariantMap bar(const QVariant& position, const QVariant& value)
{
    auto map = QVariantMap{};
    if (position.isValid()) {
        map.insert(QStringLiteral("position"), position);
    }
    if (value.isValid()) {
        map.insert(QStringLiteral("value"), value);
    }
    return map;
}

}

class BarSeriesDataTest : public QObject {
    Q_OBJECT

private slots:
    void defaultsMatchDocumentation();
    void variantListNumbersUseIndexPositions();
    void variantListObjectsReadPositionValueAndCategory();
    void invalidRawArgumentsAreRejected();
    void movedDataWithWrongSizeIsRejected();
    void floatDataIsApplied();
    void postedDataIsAppliedFromWorkerThread();
    void postedFloatDataIsAppliedFromWorkerThread();
    void clearDataRemovesBars();
    void barAtReturnsPositionValueAndCategory();
    void mismatchedCategoriesAreRejected();
    void rangesIncludeBarWidthOffsetAndBaseline();
    void negativeValuesExtendTheValueRange();
    void invalidBarsAreExcludedFromRanges();
    void horizontalBarsSwapTheAxes();
    void logValueAxisExcludesNonPositiveValues();
    void dataBoundsReplaceTheRangeScan();
    void rawDoubleDataIsCopiedAndNullIsRejected();
    void propertiesClampAndNotify();
    void epochPositionsAreUploadedRelativeToAnOrigin();
    void geometryIsPassedToTheMaterial();
    void variantListObjectsWithFromAndToAreRanged();
    void rangedDataIsAppliedAndValidated();
    void postedRangedDataIsAppliedFromWorkerThread();
    void rangedBarsReportTheirEdgesAsRange();
    void rangedBarsOnALogPositionAxisSkipNonPositiveEdges();
    void rangedBarsAreUploadedAsTriples();
};

void BarSeriesDataTest::defaultsMatchDocumentation()
{
    const auto bars = BarSeries{};
    QCOMPARE(bars.orientation(), Qt::Vertical);
    QCOMPARE(bars.barWidth(), 0.8);
    QCOMPARE(bars.barOffset(), 0.0);
    QCOMPARE(bars.baselineValue(), 0.0);
    QCOMPARE(bars.minimumWidth(), 1.0);
    QVERIFY(!bars.hoverColor().isValid());
    QCOMPARE(bars.legendSymbol(), PlotSeries::LegendSymbol::Fill);
    QCOMPARE(bars.count(), 0);
    QCOMPARE(bars.hoveredIndex(), -1);
}

void BarSeriesDataTest::variantListNumbersUseIndexPositions()
{
    auto bars = BarSeries{};
    bars.setData(QVariantList{3.0, 5, QVariant{}});
    QCOMPARE(bars.count(), 3);
    QCOMPARE(bars.barAt(0).value(QStringLiteral("position")).toDouble(), 0.0);
    QCOMPARE(bars.barAt(0).value(QStringLiteral("value")).toDouble(), 3.0);
    QCOMPARE(bars.barAt(1).value(QStringLiteral("position")).toDouble(), 1.0);
    QCOMPARE(bars.barAt(1).value(QStringLiteral("value")).toDouble(), 5.0);
    QVERIFY(std::isnan(bars.barAt(2).value(QStringLiteral("value")).toDouble()));
}

void BarSeriesDataTest::variantListObjectsReadPositionValueAndCategory()
{
    auto categorized = bar(10.0, 2.0);
    categorized.insert(QStringLiteral("category"), 1);
    auto bars = BarSeries{};
    bars.setData(QVariantList{categorized, bar({}, 4.0), bar(7.0, {})});

    QCOMPARE(bars.count(), 3);
    QCOMPARE(bars.barAt(0).value(QStringLiteral("position")).toDouble(), 10.0);
    QCOMPARE(bars.barAt(0).value(QStringLiteral("category")).toInt(), 1);
    QCOMPARE(bars.barAt(1).value(QStringLiteral("position")).toDouble(), 1.0);
    QCOMPARE(bars.barAt(1).value(QStringLiteral("value")).toDouble(), 4.0);
    QCOMPARE(bars.barAt(1).value(QStringLiteral("category")).toInt(), -1);
    QVERIFY(std::isnan(bars.barAt(2).value(QStringLiteral("value")).toDouble()));

    bars.setData(QVariantList{1.0, 2.0});
    QVERIFY(!bars.barAt(0).contains(QStringLiteral("category")));
}

void BarSeriesDataTest::invalidRawArgumentsAreRejected()
{
    auto bars = BarSeries{};
    const auto data = std::array<double, 2>{0.0, 1.0};
    const auto floatData = std::array<float, 2>{0.0f, 1.0f};
    bars.setData(data.data(), 1);
    auto countSpy = QSignalSpy{&bars, &BarSeries::countChanged};

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BarSeries received a null data pointer.*"));
    bars.setData(static_cast<const double*>(nullptr), 1);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BarSeries data bar count cannot be negative.*"));
    bars.setData(data.data(), -1);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BarSeries data bar count cannot be negative.*"));
    bars.setDataF(floatData.data(), -1);

    QCOMPARE(bars.count(), 1);
    QCOMPARE(countSpy.count(), 0);
}

void BarSeriesDataTest::movedDataWithWrongSizeIsRejected()
{
    auto bars = BarSeries{};
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BarSeries received 3 values for 2 bars.*"));
    bars.setData(std::vector<double>{0.0, 1.0, 2.0}, 2);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BarSeries received 1 categories for 2 bars.*"));
    bars.setDataF(std::vector<float>{0.0f, 1.0f, 2.0f, 3.0f}, std::vector<int>{0}, 2);
    QCOMPARE(bars.count(), 0);
}

void BarSeriesDataTest::floatDataIsApplied()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto bars = BarSeries{};
    bars.setXAxis(&xAxis);
    bars.setYAxis(&yAxis);
    bars.setDataF(std::vector<float>{1.0f, 4.0f, 2.0f, 6.0f}, std::vector<int>{1, 0}, 2);

    QCOMPARE(bars.count(), 2);
    QCOMPARE(bars.barAt(1).value(QStringLiteral("value")).toDouble(), 6.0);
    QCOMPARE(bars.barAt(0).value(QStringLiteral("category")).toInt(), 1);
    QCOMPARE(yAxis.dataMax(), 6.0);
}

void BarSeriesDataTest::postedDataIsAppliedFromWorkerThread()
{
    auto bars = BarSeries{};
    auto worker = std::thread{[&bars]() {
        bars.postData(std::vector<double>{1.0, 2.0}, 1);
        bars.postData(std::vector<double>{5.0, 6.0, 7.0, 8.0}, std::vector<int>{0, 1}, 2);
    }};
    worker.join();

    QCOMPARE(bars.count(), 0);
    QTRY_COMPARE(bars.count(), 2);
    QCOMPARE(bars.barAt(1).value(QStringLiteral("position")).toDouble(), 7.0);
    QCOMPARE(bars.barAt(1).value(QStringLiteral("category")).toInt(), 1);
}

void BarSeriesDataTest::postedFloatDataIsAppliedFromWorkerThread()
{
    auto bars = BarSeries{};
    auto worker = std::thread{[&bars]() { bars.postData(std::vector<float>{3.0f, 4.0f, 5.0f, 6.0f}, 2); }};
    worker.join();

    QTRY_COMPARE(bars.count(), 2);
    QCOMPARE(bars.barAt(0).value(QStringLiteral("value")).toDouble(), 4.0);
}

void BarSeriesDataTest::clearDataRemovesBars()
{
    auto xAxis = Axis{};
    auto remaining = BarSeries{};
    remaining.setXAxis(&xAxis);
    remaining.setData(std::vector<double>{10.0, 1.0}, 1);
    auto bars = BarSeries{};
    bars.setXAxis(&xAxis);
    bars.setData(std::vector<double>{1.0, 2.0}, 1);
    QVERIFY(xAxis.dataMin() < 1.0);
    auto countSpy = QSignalSpy{&bars, &BarSeries::countChanged};

    bars.clearData();
    QCOMPARE(bars.count(), 0);
    QCOMPARE(countSpy.count(), 1);
    QCOMPARE(xAxis.dataMin(), 9.6);
}

void BarSeriesDataTest::barAtReturnsPositionValueAndCategory()
{
    auto bars = BarSeries{};
    bars.setData(std::vector<double>{1.5, -2.0}, std::vector<int>{3}, 1);
    const auto first = bars.barAt(0);
    QCOMPARE(first.value(QStringLiteral("position")).toDouble(), 1.5);
    QCOMPARE(first.value(QStringLiteral("value")).toDouble(), -2.0);
    QCOMPARE(first.value(QStringLiteral("category")).toInt(), 3);
    QVERIFY(bars.barAt(-1).isEmpty());
    QVERIFY(bars.barAt(1).isEmpty());
}

void BarSeriesDataTest::mismatchedCategoriesAreRejected()
{
    auto bars = BarSeries{};
    bars.setData(QVariantList{1.0, 2.0});
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BarSeries received 1 categories for 2 bars"));
    bars.setCategories({0});
    QVERIFY(!bars.barAt(0).contains(QStringLiteral("category")));

    bars.setCategories({0, 1});
    QCOMPARE(bars.barAt(1).value(QStringLiteral("category")).toInt(), 1);
    bars.setCategories({});
    QVERIFY(!bars.barAt(1).contains(QStringLiteral("category")));
}

void BarSeriesDataTest::rangesIncludeBarWidthOffsetAndBaseline()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto bars = BarSeries{};
    bars.setXAxis(&xAxis);
    bars.setYAxis(&yAxis);
    bars.setData(QVariantList{3.0, 5.0, 4.0});

    QCOMPARE(xAxis.dataMin(), -0.4);
    QCOMPARE(xAxis.dataMax(), 2.4);
    QCOMPARE(yAxis.dataMin(), 0.0);
    QCOMPARE(yAxis.dataMax(), 5.0);

    bars.setBarWidth(0.5);
    bars.setBarOffset(1.0);
    QCOMPARE(xAxis.dataMin(), 0.75);
    QCOMPARE(xAxis.dataMax(), 3.25);

    bars.setBaselineValue(2.0);
    QCOMPARE(yAxis.dataMin(), 2.0);
    bars.setBaselineValue(-kInf);
    QCOMPARE(yAxis.dataMin(), 3.0);
}

void BarSeriesDataTest::negativeValuesExtendTheValueRange()
{
    auto yAxis = Axis{};
    auto bars = BarSeries{};
    bars.setYAxis(&yAxis);
    bars.setData(QVariantList{-3.0, 2.0});
    QCOMPARE(yAxis.dataMin(), -3.0);
    QCOMPARE(yAxis.dataMax(), 2.0);
}

void BarSeriesDataTest::invalidBarsAreExcludedFromRanges()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto bars = BarSeries{};
    bars.setXAxis(&xAxis);
    bars.setYAxis(&yAxis);
    bars.setBarWidth(0.0);
    bars.setData(std::vector<double>{1.0, 2.0, kNaN, 50.0, kInf, 60.0, 20.0, kNaN, 5.0, kInf, 3.0, 4.0}, 6);

    QCOMPARE(xAxis.dataMin(), 1.0);
    QCOMPARE(xAxis.dataMax(), 5.0);
    QCOMPARE(yAxis.dataMin(), 0.0);
    QCOMPARE(yAxis.dataMax(), 4.0);
}

void BarSeriesDataTest::horizontalBarsSwapTheAxes()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto bars = BarSeries{};
    bars.setXAxis(&xAxis);
    bars.setYAxis(&yAxis);
    bars.setBarWidth(1.0);
    bars.setData(QVariantList{3.0, 7.0});
    QCOMPARE(yAxis.dataMax(), 7.0);

    auto spy = QSignalSpy{&bars, &BarSeries::orientationChanged};
    bars.setOrientation(Qt::Horizontal);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(xAxis.dataMin(), 0.0);
    QCOMPARE(xAxis.dataMax(), 7.0);
    QCOMPARE(yAxis.dataMin(), -0.5);
    QCOMPARE(yAxis.dataMax(), 1.5);
}

void BarSeriesDataTest::logValueAxisExcludesNonPositiveValues()
{
    auto yAxis = Axis{};
    yAxis.setViewportMin(1.0);
    yAxis.setViewportMax(1000.0);
    yAxis.setLogScale(true);
    auto bars = BarSeries{};
    bars.setYAxis(&yAxis);
    bars.setData(QVariantList{10.0, 500.0, -5.0});
    QCOMPARE(yAxis.dataMin(), 10.0);
    QCOMPARE(yAxis.dataMax(), 500.0);

    bars.setBaselineValue(2.0);
    QCOMPARE(yAxis.dataMin(), 2.0);

    yAxis.setLogScale(false);
    QCOMPARE(yAxis.dataMin(), -5.0);
}

void BarSeriesDataTest::dataBoundsReplaceTheRangeScan()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto bars = BarSeries{};
    bars.setXAxis(&xAxis);
    bars.setYAxis(&yAxis);

    bars.setData(std::vector<double>{1.0, 2.0}, 1, PlotSeries::DataBounds{-10.0, 10.0, -20.0, 20.0});
    QCOMPARE(bars.count(), 1);
    QCOMPARE(xAxis.dataMin(), -10.0);
    QCOMPARE(xAxis.dataMax(), 10.0);
    QCOMPARE(yAxis.dataMin(), -20.0);
    QCOMPARE(yAxis.dataMax(), 20.0);

    // The bounds described the bars as they were drawn; a geometry change scans them again.
    bars.setBarWidth(2.0);
    QCOMPARE(xAxis.dataMin(), 0.0);
    QCOMPARE(xAxis.dataMax(), 2.0);
    QCOMPARE(yAxis.dataMin(), 0.0);
    QCOMPARE(yAxis.dataMax(), 2.0);

    bars.setDataF(std::vector<float>{3.0f, 4.0f}, 1, PlotSeries::DataBounds{-1.0, 1.0, -2.0, 2.0});
    QCOMPARE(xAxis.dataMax(), 1.0);
    QCOMPARE(yAxis.dataMin(), -2.0);
}

void BarSeriesDataTest::rawDoubleDataIsCopiedAndNullIsRejected()
{
    auto xAxis = Axis{};
    auto bars = BarSeries{};
    bars.setXAxis(&xAxis);
    bars.setBarWidth(0.0);
    bars.setData(std::vector<double>{3.0, 4.0}, 1);
    QCOMPARE(xAxis.dataMax(), 3.0);
    const auto raw = std::array<double, 2>{10.0, 20.0};

    bars.setData(raw.data(), 1);
    QCOMPARE(bars.barAt(0).value(QStringLiteral("position")).toDouble(), 10.0);
    QCOMPARE(xAxis.dataMax(), 10.0);

    bars.setData(static_cast<const double*>(nullptr), 0);
    QCOMPARE(bars.count(), 0);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BarSeries received a null data pointer.*"));
    bars.setData(static_cast<const double*>(nullptr), 1);
    QCOMPARE(bars.count(), 0);
}

void BarSeriesDataTest::propertiesClampAndNotify()
{
    auto bars = BarSeries{};
    auto widthSpy = QSignalSpy{&bars, &BarSeries::barWidthChanged};
    auto minimumSpy = QSignalSpy{&bars, &BarSeries::minimumWidthChanged};
    auto baselineSpy = QSignalSpy{&bars, &BarSeries::baselineValueChanged};

    bars.setBarWidth(-1.0);
    QCOMPARE(bars.barWidth(), 0.0);
    bars.setBarWidth(-2.0);
    QCOMPARE(widthSpy.count(), 1);

    bars.setMinimumWidth(-3.0);
    QCOMPARE(bars.minimumWidth(), 0.0);
    QCOMPARE(minimumSpy.count(), 1);

    bars.setBaselineValue(kNaN);
    QCOMPARE(bars.baselineValue(), 0.0);
    bars.setBaselineValue(-kInf);
    bars.setBaselineValue(-kInf);
    QCOMPARE(baselineSpy.count(), 1);
}

void BarSeriesDataTest::epochPositionsAreUploadedRelativeToAnOrigin()
{
    constexpr auto epoch = 1'789'032'600'000.0;
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    xAxis.setViewportMin(epoch);
    xAxis.setViewportMax(epoch + 10.0);
    auto window = QQuickWindow{};
    auto bars = RenderableBars{};
    bars.setParentItem(window.contentItem());
    bars.setXAxis(&xAxis);
    bars.setYAxis(&yAxis);
    bars.setPlotRect({0, 0, 100, 100});
    bars.setBaselineValue(-2.0);
    bars.setData(std::vector<double>{epoch + 1.0, 5.0, epoch + 2.0, 6.0}, 2);

    auto node = std::unique_ptr<QSGNode>{bars.updatePaintNode(nullptr, nullptr)};
    QVERIFY(node);
    const auto* material = static_cast<BarMaterial*>(static_cast<QSGGeometryNode*>(node.get())->material());
    QCOMPARE(material->domainMin.x(), -1.0f);
    QCOMPARE(material->domainMax.x(), 9.0f);
    QCOMPARE(material->baseline, -7.0f);
}

void BarSeriesDataTest::geometryIsPassedToTheMaterial()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto window = QQuickWindow{};
    auto bars = RenderableBars{};
    bars.setParentItem(window.contentItem());
    bars.setXAxis(&xAxis);
    bars.setYAxis(&yAxis);
    bars.setPlotRect({0, 0, 100, 100});
    bars.setDataF(std::vector<float>{1.0f, 5.0f}, 1);
    bars.setBarWidth(0.5);
    bars.setBarOffset(0.25);
    bars.setMinimumWidth(3.0);
    bars.setOrientation(Qt::Horizontal);

    auto node = std::unique_ptr<QSGNode>{bars.updatePaintNode(nullptr, nullptr)};
    QVERIFY(node);
    const auto* geometryNode = static_cast<QSGGeometryNode*>(node.get());
    const auto* material = static_cast<BarMaterial*>(geometryNode->material());
    QCOMPARE(geometryNode->geometry()->vertexCount(), 6);
    QCOMPARE(material->barWidth, 0.5f);
    QCOMPARE(material->barOffset, 0.25f);
    QCOMPARE(material->horizontal, 1.0f);
    QCOMPARE(material->minimumSize, QVector2D(0.0f, 3.0f));

    bars.setOrientation(Qt::Vertical);
    node.reset(bars.updatePaintNode(node.release(), nullptr));
    QCOMPARE(material->horizontal, 0.0f);
    QCOMPARE(material->minimumSize, QVector2D(3.0f, 0.0f));
}

void BarSeriesDataTest::variantListObjectsWithFromAndToAreRanged()
{
    auto bars = BarSeries{};
    bars.setData(QVariantList{
        QVariantMap{{QStringLiteral("from"), 0.0}, {QStringLiteral("to"), 1.0}, {QStringLiteral("value"), 3.0}},
        QVariantMap{{QStringLiteral("from"), 1.0}, {QStringLiteral("to"), 4.0}, {QStringLiteral("value"), 5.0}, {QStringLiteral("category"), 1}},
        bar(7.0, 2.0),
    });
    QCOMPARE(bars.count(), 3);
    const auto ranged = bars.barAt(1);
    QCOMPARE(ranged.value(QStringLiteral("from")).toDouble(), 1.0);
    QCOMPARE(ranged.value(QStringLiteral("to")).toDouble(), 4.0);
    QCOMPARE(ranged.value(QStringLiteral("value")).toDouble(), 5.0);
    QCOMPARE(ranged.value(QStringLiteral("category")).toInt(), 1);
    QVERIFY(!ranged.contains(QStringLiteral("position")));
    // In a ranged list, a bar without from and to has no extent.
    QVERIFY(std::isnan(bars.barAt(2).value(QStringLiteral("from")).toDouble()));
    QCOMPARE(bars.barAt(2).value(QStringLiteral("value")).toDouble(), 2.0);

    bars.setData(QVariantList{3.0});
    QCOMPARE(bars.barAt(0).value(QStringLiteral("position")).toDouble(), 0.0);
    QVERIFY(!bars.barAt(0).contains(QStringLiteral("from")));
}

void BarSeriesDataTest::rangedDataIsAppliedAndValidated()
{
    auto bars = BarSeries{};
    bars.setRangedData(std::vector<double>{0.0, 1.0, 3.0, 1.0, 4.0, 5.0}, std::vector<int>{0, 1}, 2);
    QCOMPARE(bars.count(), 2);
    QCOMPARE(bars.barAt(1).value(QStringLiteral("to")).toDouble(), 4.0);
    QCOMPARE(bars.barAt(1).value(QStringLiteral("category")).toInt(), 1);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BarSeries received 4 values for 2 bars; expected 6"));
    bars.setRangedData(std::vector<double>{0.0, 1.0, 3.0, 5.0}, 2);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BarSeries received 1 categories for 2 bars"));
    bars.setRangedData(std::vector<double>{0.0, 1.0, 3.0, 1.0, 4.0, 5.0}, std::vector<int>{0}, 2);
    QCOMPARE(bars.barAt(1).value(QStringLiteral("to")).toDouble(), 4.0);

    // Position data replaces the ranged bars, from the double and the float setters.
    bars.setData(std::vector<double>{7.0, 8.0}, 1);
    QCOMPARE(bars.barAt(0).value(QStringLiteral("position")).toDouble(), 7.0);
    bars.setRangedData(std::vector<double>{0.0, 1.0, 3.0}, 1);
    QCOMPARE(bars.barAt(0).value(QStringLiteral("to")).toDouble(), 1.0);
    bars.setDataF(std::vector<float>{7.0f, 8.0f}, 1);
    QCOMPARE(bars.barAt(0).value(QStringLiteral("position")).toDouble(), 7.0);
    QCOMPARE(bars.barAt(0).value(QStringLiteral("value")).toDouble(), 8.0);
}

void BarSeriesDataTest::postedRangedDataIsAppliedFromWorkerThread()
{
    auto bars = BarSeries{};
    auto worker = std::thread{[&bars]() { bars.postRangedData(std::vector<double>{0.0, 1.0, 3.0, 1.0, 4.0, 5.0}, 2); }};
    worker.join();

    QCOMPARE(bars.count(), 0);
    QTRY_COMPARE(bars.count(), 2);
    QCOMPARE(bars.barAt(1).value(QStringLiteral("to")).toDouble(), 4.0);
}

void BarSeriesDataTest::rangedBarsReportTheirEdgesAsRange()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto bars = BarSeries{};
    bars.setXAxis(&xAxis);
    bars.setYAxis(&yAxis);
    // The width and offset of position bars do not widen or move ranged bars.
    bars.setBarWidth(2.0);
    bars.setBarOffset(1.0);
    // The bar with a NaN edge is not drawn. The infinite edge reaches the plot edge and has no place in the range.
    bars.setRangedData(std::vector<double>{2.0, 3.0, 3.0, 3.0, 6.0, 5.0, kNaN, 9.0, 8.0, 7.0, kInf, 4.0}, 4);

    QCOMPARE(xAxis.dataMin(), 2.0);
    QCOMPARE(xAxis.dataMax(), 7.0);
    QCOMPARE(yAxis.dataMin(), 0.0);
    QCOMPARE(yAxis.dataMax(), 5.0);

    bars.setOrientation(Qt::Horizontal);
    QCOMPARE(yAxis.dataMin(), 2.0);
    QCOMPARE(yAxis.dataMax(), 7.0);
    QCOMPARE(xAxis.dataMin(), 0.0);
    QCOMPARE(xAxis.dataMax(), 5.0);
}

void BarSeriesDataTest::rangedBarsOnALogPositionAxisSkipNonPositiveEdges()
{
    auto xAxis = Axis{};
    xAxis.setViewportMin(1.0);
    xAxis.setViewportMax(1000.0);
    xAxis.setLogScale(true);
    auto bars = BarSeries{};
    bars.setXAxis(&xAxis);
    bars.setRangedData(std::vector<double>{0.0, 1.0, 3.0, 1.0, 10.0, 5.0, 10.0, 100.0, 2.0}, 3);
    QCOMPARE(xAxis.dataMin(), 1.0);
    QCOMPARE(xAxis.dataMax(), 100.0);
}

void BarSeriesDataTest::rangedBarsAreUploadedAsTriples()
{
    constexpr auto epoch = 1'789'032'600'000.0;
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    xAxis.setViewportMin(epoch);
    xAxis.setViewportMax(epoch + 10.0);
    auto window = QQuickWindow{};
    auto bars = RenderableBars{};
    bars.setParentItem(window.contentItem());
    bars.setXAxis(&xAxis);
    bars.setYAxis(&yAxis);
    bars.setPlotRect({0, 0, 100, 100});
    bars.setRangedData(std::vector<double>{epoch + 1.0, epoch + 3.0, 5.0, epoch + 3.0, epoch + 4.0, 6.0}, 2);

    auto node = std::unique_ptr<QSGNode>{bars.updatePaintNode(nullptr, nullptr)};
    QVERIFY(node);
    const auto* geometryNode = static_cast<QSGGeometryNode*>(node.get());
    const auto* material = static_cast<BarMaterial*>(geometryNode->material());
    QCOMPARE(geometryNode->geometry()->vertexCount(), 12);
    QCOMPARE(material->ranged, 1.0f);
    // The edges are uploaded relative to the first one.
    QCOMPARE(material->domainMin.x(), -1.0f);
    QCOMPARE(material->domainMax.x(), 9.0f);

    bars.setData(std::vector<double>{epoch + 1.0, 5.0}, 1);
    node.reset(bars.updatePaintNode(node.release(), nullptr));
    QCOMPARE(material->ranged, 0.0f);
}

} // namespace QAccelPlot

using QAccelPlot::BarSeriesDataTest;
QTEST_MAIN(BarSeriesDataTest)
#include "tst_bar_series_data.moc"
