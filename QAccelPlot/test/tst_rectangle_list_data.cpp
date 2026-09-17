//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/materials/RectMaterial.hpp"
#include "QAccelPlot/shapes/RectangleList.hpp"

#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QtTest/QtTest>

#include <array>

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
    const auto data = std::array<float, 4>{0.0f, 1.0f, 2.0f, 3.0f};
    auto countSpy = QSignalSpy{&rectangles, &RectangleList::countChanged};

    rectangles.setData(data.data(), 1);
    QCOMPARE(rectangles.count(), 1);
    QCOMPARE(countSpy.count(), 1);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList received a null data pointer.*"));
    rectangles.setData(static_cast<const float*>(nullptr), 1);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList data rectangle count cannot be negative.*"));
    rectangles.setData(data.data(), -1);

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
