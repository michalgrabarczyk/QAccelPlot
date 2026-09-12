//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

Window {
    width: 320
    height: 240
    visible: true
    color: "#101820"
    title: "QAccelPlot installed-package smoke test"

    QAccelPlot.Plot {
        id: plot
        anchors.fill: parent
        anchors.margins: 12
        plotAreaColor: "#101820"
        axesAreaColor: "#101820"
        legendVisible: false

        xAxis: QAccelPlot.Axis {
            visible: false
            viewportMin: 0
            viewportMax: 4
            dataMin: 0
            dataMax: 4
        }

        yAxis: QAccelPlot.Axis {
            visible: false
            viewportMin: 0
            viewportMax: 4
            dataMin: 0
            dataMax: 4
        }

        QAccelPlot.LineCurve {
            xAxis: plot.xAxis
            yAxis: plot.yAxis
            color: "#ff3b30"
            lineWidth: 8

            Component.onCompleted: setData([
                Qt.point(0, 0.5),
                Qt.point(1, 3.5),
                Qt.point(2, 0.5),
                Qt.point(3, 3.5),
                Qt.point(4, 0.5)
            ])
        }
    }
}
