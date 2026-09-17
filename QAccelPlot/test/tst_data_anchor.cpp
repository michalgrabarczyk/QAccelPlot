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
};

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

} // namespace QAccelPlot

using QAccelPlot::DataAnchorTest;
QTEST_MAIN(DataAnchorTest)
#include "tst_data_anchor.moc"
