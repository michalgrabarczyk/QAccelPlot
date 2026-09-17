//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/shapes/RectangleList.hpp"

#include <QtTest/QtTest>

#include <array>

namespace QAccelPlot {

class RectangleListDataTest : public QObject {
    Q_OBJECT

private slots:
    void hoverEnvironmentControlsAcceptance();
    void invalidRawArgumentsAreRejected();
    void rawDoubleDataPreservesModernEpochPrecision();
    void variantListDataPreservesModernEpochPrecision();
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

} // namespace QAccelPlot

using QAccelPlot::RectangleListDataTest;
QTEST_MAIN(RectangleListDataTest)
#include "tst_rectangle_list_data.moc"
