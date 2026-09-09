//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "RectangleList.hpp"

#include <QtTest/QtTest>

#include <array>

namespace QAccelPlot {

class RectangleListDataTest : public QObject {
    Q_OBJECT

private slots:
    void invalidRawArgumentsAreRejected();
};

void RectangleListDataTest::invalidRawArgumentsAreRejected()
{
    auto rectangles = RectangleList{};
    const auto data = std::array<float, 4>{0.0f, 1.0f, 2.0f, 3.0f};
    auto countSpy = QSignalSpy{&rectangles, &RectangleList::countChanged};

    rectangles.setRawData(data.data(), 1);
    QCOMPARE(rectangles.count(), 1);
    QCOMPARE(countSpy.count(), 1);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList received a null data pointer.*"));
    rectangles.setRawData(nullptr, 1);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleList data rectangle count cannot be negative.*"));
    rectangles.setRawData(data.data(), -1);

    QCOMPARE(rectangles.count(), 1);
    QCOMPARE(countSpy.count(), 1);
}

} // namespace QAccelPlot

using QAccelPlot::RectangleListDataTest;
QTEST_MAIN(RectangleListDataTest)
#include "tst_rectangle_list_data.moc"
