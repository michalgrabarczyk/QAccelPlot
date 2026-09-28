//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/materials/RectMaterial.hpp"
#include "QAccelPlot/series/RectangleList.hpp"

#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QtTest/QtTest>

#include <array>
#include <limits>
#include <thread>
#include <vector>

namespace QAccelPlot {

namespace {
class RenderableRectangles final : public RectangleList {
public:
    using RectangleList::updatePaintNode;
};
}

class RectangleListDataTest : public QObject {
    Q_OBJECT

private slots:
    void hoverEnvironmentControlsAcceptance();
    void invalidRawArgumentsAreRejected();
    void rawDoubleDataPreservesModernEpochPrecision();
    void variantListDataPreservesModernEpochPrecision();
    void movedDataIsApplied();
    void movedDataWithWrongSizeIsRejected();
    void noRangeDataDoesNotReportRanges();
    void floatDataIsApplied();
    void floatNoRangeDataDoesNotReportRanges();
    void floatDataIsUploadedWithoutOrigin();
    void postedFloatDataIsAppliedFromWorkerThread();
    void postedDataIsAppliedFromWorkerThread();
    void clearDataRemovesRectangles();
    void countChangedOnlyWhenCountChanges();
    void rectangleAtReturnsBounds();
    void missingEdgesAreUnbounded();
    void unboundedAndInvalidEdgesAreExcludedFromRanges();
    void variantListCategoriesAreStored();
    void categoriesFollowTheData();
    void mismatchedCategoriesAreRejected();
    void borderSettingsClampAndNotify();
    void scaleChangesRefreshRenderCoordinates_data();
    void scaleChangesRefreshRenderCoordinates();
};

void RectangleListDataTest::hoverEnvironmentControlsAcceptance()
{
    constexpr auto variableName = "QACCELPLOT_HOVER_ENABLED";
    const auto wasSet = qEnvironmentVariableIsSet(variableName);
    const auto previousValue = qgetenv(variableName);

    qunsetenv(variableName);
    const auto defaultRectangles = RectangleList{};
    qputenv(variableName, "0");
    const auto hoverDisabledRectangles = RectangleList{};

    if (wasSet) {
        qputenv(variableName, previousValue);
    } else {
        qunsetenv(variableName);
    }

    QVERIFY(defaultRectangles.acceptHoverEvents());
    QVERIFY(!hoverDisabledRectangles.acceptHoverEvents());
}

void RectangleListDataTest::invalidRawArgumentsAreRejected()
{
    auto rectangles = RectangleList{};
    const auto data = std::array<double, 4>{0.0, 1.0, 2.0, 3.0};
    const auto floatData = std::array<float, 4>{0.0f, 1.0f, 2.0f, 3.0f};
    auto countSpy = QSignalSpy{&rectangles, &RectangleList::countChanged};

    rectangles.setData(data.data(), 1);
    QCOMPARE(rectangles.count(), 1);
    QCOMPARE(countSpy.count(), 1);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList received a null data pointer.*"));
    rectangles.setData(static_cast<const double*>(nullptr), 1);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList data rectangle count cannot be negative.*"));
    rectangles.setData(data.data(), -1);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList data rectangle count cannot be negative.*"));
    rectangles.setDataF(floatData.data(), -1);

    QCOMPARE(rectangles.count(), 1);
    QCOMPARE(countSpy.count(), 1);
}

void RectangleListDataTest::rawDoubleDataPreservesModernEpochPrecision()
{
    constexpr auto epochMilliseconds = double{1'789'032'600'000.0};
    auto xAxis = Axis{};
    auto rectangles = RectangleList{};
    rectangles.setXAxis(&xAxis);

    const auto data = std::array<double, 8>{
        epochMilliseconds,
        0.0,
        epochMilliseconds + 1.0,
        1.0,
        epochMilliseconds + 2.0,
        0.0,
        epochMilliseconds + 3.0,
        1.0,
    };
    rectangles.setData(data.data(), 2);

    QCOMPARE(xAxis.dataMin(), epochMilliseconds);
    QCOMPARE(xAxis.dataMax(), epochMilliseconds + 3.0);
}

void RectangleListDataTest::variantListDataPreservesModernEpochPrecision()
{
    constexpr auto epochMilliseconds = double{1'789'032'600'000.0};
    auto xAxis = Axis{};
    auto rectangles = RectangleList{};
    rectangles.setXAxis(&xAxis);

    auto rectMap = QVariantMap{};
    rectMap.insert(QStringLiteral("x1"), epochMilliseconds);
    rectMap.insert(QStringLiteral("y1"), 0.0);
    rectMap.insert(QStringLiteral("x2"), epochMilliseconds + 1.0);
    rectMap.insert(QStringLiteral("y2"), 1.0);
    rectangles.setData(QVariantList{rectMap});

    QCOMPARE(xAxis.dataMin(), epochMilliseconds);
    QCOMPARE(xAxis.dataMax(), epochMilliseconds + 1.0);
}

void RectangleListDataTest::movedDataIsApplied()
{
    auto xAxis = Axis{};
    auto rectangles = RectangleList{};
    rectangles.setXAxis(&xAxis);

    rectangles.setData(std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0}, 2);

    QCOMPARE(rectangles.count(), 2);
    QCOMPARE(rectangles.rectangleAt(1).value(QStringLiteral("x2")).toDouble(), 7.0);
    QCOMPARE(xAxis.dataMin(), 1.0);
    QCOMPARE(xAxis.dataMax(), 7.0);
}

void RectangleListDataTest::movedDataWithWrongSizeIsRejected()
{
    auto rectangles = RectangleList{};
    rectangles.setData(std::vector<double>{1.0, 2.0, 3.0, 4.0}, 1);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList received 3 coordinates for 1 rectangles; expected 4"));
    rectangles.setData(std::vector<double>{1.0, 2.0, 3.0}, 1);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList data rectangle count cannot be negative.*"));
    rectangles.setData(std::vector<double>{}, -1);

    QCOMPARE(rectangles.count(), 1);
    QCOMPARE(rectangles.rectangleAt(0).value(QStringLiteral("y2")).toDouble(), 4.0);
}

void RectangleListDataTest::noRangeDataDoesNotReportRanges()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    xAxis.setViewportMin(0);
    xAxis.setViewportMax(100);
    yAxis.setViewportMin(0);
    yAxis.setViewportMax(100);
    auto window = QQuickWindow{};
    auto rectangles = RenderableRectangles{};
    rectangles.setParentItem(window.contentItem());
    rectangles.setXAxis(&xAxis);
    rectangles.setYAxis(&yAxis);
    rectangles.setPlotRect({0, 0, 100, 100});
    rectangles.setData(std::vector<double>{1.0, 2.0, 3.0, 4.0}, 1);
    auto node = std::unique_ptr<QSGNode>{rectangles.updatePaintNode(nullptr, nullptr)};
    QVERIFY(node);
    auto xRangeSpy = QSignalSpy{&rectangles, &RectangleList::xDataRangeChanged};
    auto yRangeSpy = QSignalSpy{&rectangles, &RectangleList::yDataRangeChanged};

    rectangles.setDataNoRange(std::vector<double>{10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0}, std::vector<int>{1, 0}, 2);

    QCOMPARE(rectangles.count(), 2);
    QCOMPARE(rectangles.rectangleAt(1).value(QStringLiteral("x1")).toDouble(), 50.0);
    QCOMPARE(rectangles.rectangleAt(1).value(QStringLiteral("category")).toInt(), 0);
    QCOMPARE(xRangeSpy.count(), 0);
    QCOMPARE(yRangeSpy.count(), 0);
    QCOMPARE(xAxis.dataMin(), 1.0);
    QCOMPARE(xAxis.dataMax(), 3.0);
    QCOMPARE(yAxis.dataMin(), 2.0);
    QCOMPARE(yAxis.dataMax(), 4.0);

    node.reset(rectangles.updatePaintNode(node.release(), nullptr));
    const auto* material = static_cast<RectMaterial*>(static_cast<QSGGeometryNode*>(node.get())->material());
    QCOMPARE(material->rectCount, 2.0f);
    QCOMPARE(material->useVertexColor, 1.0f);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList received 3 coordinates for 1 rectangles; expected 4"));
    rectangles.setDataNoRange(std::vector<double>{1.0, 2.0, 3.0}, {}, 1);
    QCOMPARE(rectangles.count(), 2);

    rectangles.setDataNoRange(std::vector<double>{90.0, 91.0, 92.0, 93.0}, 1);
    QCOMPARE(rectangles.count(), 1);
    QVERIFY(!rectangles.rectangleAt(0).contains(QStringLiteral("category")));
    QCOMPARE(xRangeSpy.count(), 0);
    QCOMPARE(xAxis.dataMax(), 3.0);
}

void RectangleListDataTest::floatDataIsApplied()
{
    constexpr auto kInf = std::numeric_limits<float>::infinity();
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto rectangles = RectangleList{};
    rectangles.setXAxis(&xAxis);
    rectangles.setYAxis(&yAxis);

    rectangles.setDataF(std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, -kInf, 7.0f, kInf}, 2);
    QCOMPARE(rectangles.count(), 2);
    QCOMPARE(rectangles.rectangleAt(1).value(QStringLiteral("x2")).toDouble(), 7.0);
    QCOMPARE(rectangles.rectangleAt(1).value(QStringLiteral("y1")).toDouble(), -std::numeric_limits<double>::infinity());
    QCOMPARE(xAxis.dataMin(), 1.0);
    QCOMPARE(xAxis.dataMax(), 7.0);
    QCOMPARE(yAxis.dataMin(), 2.0);
    QCOMPARE(yAxis.dataMax(), 4.0);

    const auto raw = std::array<float, 4>{10.0f, 20.0f, 30.0f, 40.0f};
    rectangles.setDataF(raw.data(), 1);
    QCOMPARE(rectangles.count(), 1);
    QCOMPARE(rectangles.rectangleAt(0).value(QStringLiteral("y2")).toDouble(), 40.0);
    QCOMPARE(xAxis.dataMin(), 10.0);

    rectangles.setDataF(std::vector<float>{0.0f, 0.0f, 1.0f, 1.0f}, std::vector<int>{3}, 1);
    QCOMPARE(rectangles.rectangleAt(0).value(QStringLiteral("category")).toInt(), 3);

    // Switching back to double data reads the double storage again.
    rectangles.setData(std::vector<double>{100.0, 200.0, 300.0, 400.0}, 1);
    QCOMPARE(rectangles.rectangleAt(0).value(QStringLiteral("x1")).toDouble(), 100.0);
    QCOMPARE(xAxis.dataMin(), 100.0);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList received 3 coordinates for 1 rectangles; expected 4"));
    rectangles.setDataF(std::vector<float>{1.0f, 2.0f, 3.0f}, 1);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList received a null data pointer for 1 rectangles"));
    rectangles.setDataF(nullptr, 1);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList received 2 categories for 1 rectangles"));
    rectangles.setDataF(std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f}, std::vector<int>{1, 2}, 1);
    QCOMPARE(rectangles.rectangleAt(0).value(QStringLiteral("x1")).toDouble(), 100.0);
}

void RectangleListDataTest::floatNoRangeDataDoesNotReportRanges()
{
    auto xAxis = Axis{};
    auto rectangles = RectangleList{};
    rectangles.setXAxis(&xAxis);
    rectangles.setDataF(std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f}, 1);
    auto xRangeSpy = QSignalSpy{&rectangles, &RectangleList::xDataRangeChanged};

    rectangles.setDataFNoRange(std::vector<float>{10.0f, 20.0f, 30.0f, 40.0f}, 1);
    QCOMPARE(rectangles.rectangleAt(0).value(QStringLiteral("x1")).toDouble(), 10.0);

    rectangles.setDataFNoRange(std::vector<float>{50.0f, 60.0f, 70.0f, 80.0f, 90.0f, 91.0f, 92.0f, 93.0f}, std::vector<int>{0, 1}, 2);
    QCOMPARE(rectangles.count(), 2);
    QCOMPARE(rectangles.rectangleAt(1).value(QStringLiteral("category")).toInt(), 1);

    const auto raw = std::array<float, 4>{-5.0f, -6.0f, -7.0f, -8.0f};
    rectangles.setDataFNoRange(raw.data(), 1);
    QCOMPARE(rectangles.count(), 1);
    QCOMPARE(rectangles.rectangleAt(0).value(QStringLiteral("y2")).toDouble(), -8.0);
    QVERIFY(!rectangles.rectangleAt(0).contains(QStringLiteral("category")));

    QCOMPARE(xRangeSpy.count(), 0);
    QCOMPARE(xAxis.dataMin(), 1.0);
    QCOMPARE(xAxis.dataMax(), 3.0);
}

void RectangleListDataTest::floatDataIsUploadedWithoutOrigin()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    xAxis.setViewportMin(1);
    xAxis.setViewportMax(1000);
    yAxis.setViewportMin(1);
    yAxis.setViewportMax(1000);
    auto window = QQuickWindow{};
    auto rectangles = RenderableRectangles{};
    rectangles.setParentItem(window.contentItem());
    rectangles.setXAxis(&xAxis);
    rectangles.setYAxis(&yAxis);
    rectangles.setPlotRect({0, 0, 100, 100});
    auto node = std::unique_ptr<QSGNode>{};
    const auto paintedDomainMin = [&rectangles, &node]() {
        node.reset(rectangles.updatePaintNode(node.release(), nullptr));
        return static_cast<RectMaterial*>(static_cast<QSGGeometryNode*>(node.get())->material())->domainMin;
    };

    rectangles.setData(std::vector<double>{10, 20, 100, 200}, 1);
    QCOMPARE(paintedDomainMin(), QVector2D(1 - 10, 1 - 20));

    rectangles.setDataF(std::vector<float>{10, 20, 100, 200}, 1);
    QCOMPARE(paintedDomainMin(), QVector2D(1, 1));

    xAxis.setLogScale(true);
    QCOMPARE(paintedDomainMin(), QVector2D(1, 1));

    rectangles.setData(std::vector<double>{10, 20, 100, 200}, 1);
    QCOMPARE(paintedDomainMin(), QVector2D(1, 1 - 20));
}

void RectangleListDataTest::postedFloatDataIsAppliedFromWorkerThread()
{
    auto rectangles = RectangleList{};
    auto worker = std::thread{[&rectangles]() {
        rectangles.postData(std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f}, 1);
        rectangles.postData(std::vector<float>{5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f}, std::vector<int>{4, 5}, 2);
    }};
    worker.join();

    QCOMPARE(rectangles.count(), 0);
    QTRY_COMPARE(rectangles.count(), 2);
    QCOMPARE(rectangles.rectangleAt(1).value(QStringLiteral("x1")).toDouble(), 9.0);
    QCOMPARE(rectangles.rectangleAt(1).value(QStringLiteral("category")).toInt(), 5);
}

void RectangleListDataTest::postedDataIsAppliedFromWorkerThread()
{
    auto rectangles = RectangleList{};
    auto worker = std::thread{[&rectangles]() {
        rectangles.postData(std::vector<double>{1.0, 2.0, 3.0, 4.0}, 1);
        rectangles.postData(std::vector<double>{5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0}, 2);
    }};
    worker.join();

    // Queued: nothing is applied until the event loop runs.
    QCOMPARE(rectangles.count(), 0);
    QTRY_COMPARE(rectangles.count(), 2);
    QCOMPARE(rectangles.rectangleAt(1).value(QStringLiteral("x1")).toDouble(), 9.0);
}

void RectangleListDataTest::clearDataRemovesRectangles()
{
    auto xAxis = Axis{};
    auto remaining = RectangleList{};
    remaining.setXAxis(&xAxis);
    remaining.setData(std::vector<double>{10.0, 0.0, 20.0, 1.0}, 1);
    auto rectangles = RectangleList{};
    rectangles.setXAxis(&xAxis);
    rectangles.setData(std::vector<double>{1.0, 2.0, 3.0, 4.0}, 1);
    QCOMPARE(xAxis.dataMin(), 1.0);
    auto countSpy = QSignalSpy{&rectangles, &RectangleList::countChanged};

    rectangles.clearData();

    QCOMPARE(rectangles.count(), 0);
    QCOMPARE(countSpy.count(), 1);
    QVERIFY(rectangles.rectangleAt(0).isEmpty());
    QCOMPARE(xAxis.dataMin(), 10.0);
    QCOMPARE(xAxis.dataMax(), 20.0);
}

void RectangleListDataTest::countChangedOnlyWhenCountChanges()
{
    auto rectangles = RectangleList{};
    auto countSpy = QSignalSpy{&rectangles, &RectangleList::countChanged};

    rectangles.setData(std::vector<double>{1.0, 2.0, 3.0, 4.0}, 1);
    rectangles.setData(std::vector<double>{5.0, 6.0, 7.0, 8.0}, 1);
    QCOMPARE(countSpy.count(), 1);

    rectangles.setData(QVariantList{});
    QCOMPARE(countSpy.count(), 2);
}

void RectangleListDataTest::rectangleAtReturnsBounds()
{
    auto rectangles = RectangleList{};
    auto rectMap = QVariantMap{};
    rectMap.insert(QStringLiteral("x1"), 1.5);
    rectMap.insert(QStringLiteral("y1"), -2.0);
    rectMap.insert(QStringLiteral("x2"), 3.5);
    rectMap.insert(QStringLiteral("y2"), 4.0);
    rectangles.setData(QVariantList{rectMap});

    QCOMPARE(rectangles.rectangleAt(0), rectMap);
    QVERIFY(rectangles.rectangleAt(-1).isEmpty());
    QVERIFY(rectangles.rectangleAt(1).isEmpty());
}

void RectangleListDataTest::missingEdgesAreUnbounded()
{
    constexpr auto kInf = std::numeric_limits<double>::infinity();
    auto rectangles = RectangleList{};
    auto span = QVariantMap{};
    span.insert(QStringLiteral("x1"), 8.0);
    span.insert(QStringLiteral("x2"), 12.0);
    auto band = QVariantMap{};
    band.insert(QStringLiteral("x1"), QVariant::fromValue(nullptr));
    band.insert(QStringLiteral("y1"), 1.0);
    band.insert(QStringLiteral("y2"), 2.0);
    rectangles.setData(QVariantList{span, band});

    const auto first = rectangles.rectangleAt(0);
    QCOMPARE(first.value(QStringLiteral("x1")).toDouble(), 8.0);
    QCOMPARE(first.value(QStringLiteral("y1")).toDouble(), -kInf);
    QCOMPARE(first.value(QStringLiteral("x2")).toDouble(), 12.0);
    QCOMPARE(first.value(QStringLiteral("y2")).toDouble(), kInf);
    const auto second = rectangles.rectangleAt(1);
    QCOMPARE(second.value(QStringLiteral("x1")).toDouble(), -kInf);
    QCOMPARE(second.value(QStringLiteral("x2")).toDouble(), kInf);
}

void RectangleListDataTest::unboundedAndInvalidEdgesAreExcludedFromRanges()
{
    constexpr auto kInf = std::numeric_limits<double>::infinity();
    constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto other = RectangleList{};
    other.setYAxis(&yAxis);
    other.setData(std::vector<double>{0.0, 100.0, 1.0, 200.0}, 1);
    auto rectangles = RectangleList{};
    rectangles.setXAxis(&xAxis);
    rectangles.setYAxis(&yAxis);

    rectangles.setData(std::vector<double>{8.0, -kInf, 12.0, kInf, -kInf, 5.0, 20.0, 6.0, -50.0, -50.0, 50.0, kNaN}, 3);

    QCOMPARE(xAxis.dataMin(), 8.0);
    QCOMPARE(xAxis.dataMax(), 20.0);
    QCOMPARE(yAxis.dataMin(), 5.0);
    QCOMPARE(yAxis.dataMax(), 200.0);

    // Only full-height spans: the list stops contributing a Y range.
    rectangles.setData(std::vector<double>{8.0, -kInf, 12.0, kInf}, 1);
    QCOMPARE(yAxis.dataMin(), 100.0);
    QCOMPARE(yAxis.dataMax(), 200.0);
}

void RectangleListDataTest::variantListCategoriesAreStored()
{
    auto rectangles = RectangleList{};
    auto first = QVariantMap{};
    first.insert(QStringLiteral("x2"), 1.0);
    first.insert(QStringLiteral("category"), 2);
    auto second = QVariantMap{};
    second.insert(QStringLiteral("x2"), 2.0);
    rectangles.setData(QVariantList{first, second});

    QCOMPARE(rectangles.rectangleAt(0).value(QStringLiteral("category")).toInt(), 2);
    QCOMPARE(rectangles.rectangleAt(1).value(QStringLiteral("category")).toInt(), -1);

    rectangles.setData(QVariantList{second});
    QVERIFY(!rectangles.rectangleAt(0).contains(QStringLiteral("category")));
}

void RectangleListDataTest::categoriesFollowTheData()
{
    auto rectangles = RectangleList{};
    rectangles.setData(std::vector<double>{0.0, 0.0, 1.0, 1.0, 2.0, 2.0, 3.0, 3.0}, std::vector<int>{1, 0}, 2);
    QCOMPARE(rectangles.rectangleAt(1).value(QStringLiteral("category")).toInt(), 0);

    rectangles.setCategories({3, 4});
    QCOMPARE(rectangles.rectangleAt(0).value(QStringLiteral("category")).toInt(), 3);

    rectangles.setCategories({});
    QVERIFY(!rectangles.rectangleAt(0).contains(QStringLiteral("category")));

    rectangles.setCategories({5, 6});
    rectangles.setData(std::vector<double>{0.0, 0.0, 1.0, 1.0}, 1);
    QVERIFY(!rectangles.rectangleAt(0).contains(QStringLiteral("category")));

    auto worker = std::thread{[&rectangles]() { rectangles.postData(std::vector<double>{0.0, 0.0, 1.0, 1.0}, std::vector<int>{7}, 1); }};
    worker.join();
    QTRY_COMPARE(rectangles.rectangleAt(0).value(QStringLiteral("category")).toInt(), 7);
}

void RectangleListDataTest::mismatchedCategoriesAreRejected()
{
    auto rectangles = RectangleList{};
    rectangles.setData(std::vector<double>{0.0, 0.0, 1.0, 1.0}, std::vector<int>{1}, 1);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList received 2 categories for 1 rectangles"));
    rectangles.setData(std::vector<double>{5.0, 5.0, 6.0, 6.0}, std::vector<int>{1, 2}, 1);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList received 3 categories for 1 rectangles"));
    rectangles.setCategories({1, 2, 3});

    QCOMPARE(rectangles.rectangleAt(0).value(QStringLiteral("x1")).toDouble(), 0.0);
    QCOMPARE(rectangles.rectangleAt(0).value(QStringLiteral("category")).toInt(), 1);
}

void RectangleListDataTest::borderSettingsClampAndNotify()
{
    auto rectangles = RectangleList{};
    auto* border = rectangles.border();
    QCOMPARE(border->width(), 0.0);
    QCOMPARE(border->color(), QColor{Qt::black});
    auto widthSpy = QSignalSpy{border, &RectangleBorder::widthChanged};
    auto colorSpy = QSignalSpy{border, &RectangleBorder::colorChanged};

    border->setWidth(2.0);
    border->setWidth(2.0);
    border->setColor(Qt::white);
    QCOMPARE(widthSpy.count(), 1);
    QCOMPARE(colorSpy.count(), 1);

    border->setWidth(-1.0);
    QCOMPARE(border->width(), 0.0);
}

void RectangleListDataTest::scaleChangesRefreshRenderCoordinates_data()
{
    QTest::addColumn<bool>("horizontal");
    QTest::addColumn<bool>("logScale");
    QTest::newRow("x-linear-to-log") << true << true;
    QTest::newRow("x-log-to-linear") << true << false;
    QTest::newRow("y-linear-to-log") << false << true;
    QTest::newRow("y-log-to-linear") << false << false;
}

void RectangleListDataTest::scaleChangesRefreshRenderCoordinates()
{
    QFETCH(bool, horizontal);
    QFETCH(bool, logScale);
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    xAxis.setViewportMin(1);
    xAxis.setViewportMax(1000);
    yAxis.setViewportMin(1);
    yAxis.setViewportMax(1000);
    auto& changedAxis = horizontal ? xAxis : yAxis;
    changedAxis.setLogScale(!logScale);
    auto window = QQuickWindow{};
    auto rectangles = RenderableRectangles{};
    rectangles.setParentItem(window.contentItem());
    rectangles.setXAxis(&xAxis);
    rectangles.setYAxis(&yAxis);
    rectangles.setPlotRect({0, 0, 100, 100});
    const auto data = std::array<double, 4>{10, 20, 100, 200};
    rectangles.setData(data.data(), 1);
    auto node = std::unique_ptr<QSGNode>{rectangles.updatePaintNode(nullptr, nullptr)};
    QVERIFY(node);

    changedAxis.setLogScale(logScale);
    node.reset(rectangles.updatePaintNode(node.release(), nullptr));

    auto* geometry = static_cast<QSGGeometryNode*>(node.get());
    const auto* material = static_cast<RectMaterial*>(geometry->material());
    const auto expectedMin = QVector2D{float(1 - (xAxis.logScale() ? 0 : 10)), float(1 - (yAxis.logScale() ? 0 : 20))};
    QCOMPARE(material->domainMin, expectedMin);
    QCOMPARE(material->logScaleX, xAxis.logScale() ? 1.0f : 0.0f);
    QCOMPARE(material->logScaleY, yAxis.logScale() ? 1.0f : 0.0f);
}

} // namespace QAccelPlot

using QAccelPlot::RectangleListDataTest;
QTEST_MAIN(RectangleListDataTest)
#include "tst_rectangle_list_data.moc"
