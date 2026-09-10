//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QAccelPlot as QAccelPlot
Item {
    id: root
    required property var palette
    required property var typography

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Label {
            text: "TextTickLabelFormatter maps integer coordinates to names, allowing numeric series data to describe ordered categories."
            color: root.palette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            plotAreaColor: root.palette.plotArea
            axesAreaColor: root.palette.axesArea
            grid.gridColor: root.palette.grid
            grid.subGridVisible: false

            xAxis: QAccelPlot.Axis {
                viewportMin: 0
                viewportMax: 6
                dataMin: 0
                dataMax: 6
                side: QAccelPlot.Axis.Bottom
                label: "Day of week"
                baselineColor: root.palette.axisLine
                labelColor: root.palette.axisLabel
                labelFont: root.typography.axisLabel
                hoverColor: root.palette.hover
                ticker.tickCount: 6
                ticker.subtickCount: 0
                ticker.tickColor: root.palette.tick
                ticker.tickLabelColor: root.palette.axisTickLabel
                ticker.tickLabelFont: root.typography.axisTickLabel
                ticker.subtickColor: root.palette.subtick
                ticker.tickLabelFormatter: QAccelPlot.TextTickLabelFormatter {
                    labels: ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
                }
            }

            yAxis: QAccelPlot.Axis {
                viewportMin: 20
                viewportMax: 55
                dataMin: 20
                dataMax: 55
                side: QAccelPlot.Axis.Left
                axisTitlePadding: 40
                layoutSize: 60
                label: "Electricity use (kWh)"
                baselineColor: root.palette.axisLine
                labelColor: root.palette.axisLabel
                labelFont: root.typography.axisLabel
                hoverColor: root.palette.hover
                ticker.tickColor: root.palette.tick
                ticker.tickLabelColor: root.palette.axisTickLabel
                ticker.tickLabelFont: root.typography.axisTickLabel
                ticker.subtickColor: root.palette.subtick
            }

            QAccelPlot.LineCurve {
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.palette.seriesSecondary
                lineWidth: 3
                markerShape: QAccelPlot.LineCurve.Diamond
                markerSize: 8
                antialiasingEnabled: true

                Component.onCompleted: setData([Qt.point(0, 34), Qt.point(1, 32), Qt.point(2, 36), Qt.point(3, 35), Qt.point(4, 39), Qt.point(5, 51), Qt.point(6, 46)])
            }
        }
    }
}
