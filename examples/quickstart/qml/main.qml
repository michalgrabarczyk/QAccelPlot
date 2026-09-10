//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QAccelPlot as QAccelPlot
Window {
    id: root

    readonly property QtObject colorPalette: QAccelPlot.Colors.dark

    width: 840
    height: 560
    visible: true
    title: "QAccelPlot Quickstart"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent

    Typography {
        id: typography
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Label {
            text: "Daily temperature"
            color: colorPalette.text
            font.bold: true
            font.pixelSize: 20
        }

        Label {
            text: "A simple temperature plot with two curves, labeled axes, and a legend."
            color: colorPalette.textSecondary
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            plotAreaColor: colorPalette.plotArea
            axesAreaColor: colorPalette.axesArea
            grid.gridColor: colorPalette.grid
            grid.subGridVisible: false
            legend: QAccelPlot.Legend {
                series: plot.series
                color: colorPalette.legendBackground
                textColor: colorPalette.text
                border.color: colorPalette.legendBorder
            }

            xAxis: QAccelPlot.Axis {
                viewportMin: 0
                viewportMax: 24
                dataMin: 0
                dataMax: 24
                side: QAccelPlot.Axis.Bottom
                label: "Time of day (h)"
                baselineColor: colorPalette.axisLine
                labelColor: colorPalette.axisLabel
                labelFont: typography.axisLabel
                hoverColor: colorPalette.hover
                ticker.tickColor: colorPalette.tick
                ticker.tickLabelColor: colorPalette.axisTickLabel
                ticker.tickLabelFont: typography.axisTickLabel
                ticker.subtickColor: colorPalette.subtick
                ticker.tickCount: 7
            }

            yAxis: QAccelPlot.Axis {
                viewportMin: 16
                viewportMax: 27
                dataMin: 16
                dataMax: 27
                side: QAccelPlot.Axis.Left
                axisTitlePadding: 40
                layoutSize: 60
                label: "Temperature (°C)"
                baselineColor: colorPalette.axisLine
                labelColor: colorPalette.axisLabel
                labelFont: typography.axisLabel
                hoverColor: colorPalette.hover
                ticker.tickColor: colorPalette.tick
                ticker.tickLabelColor: colorPalette.axisTickLabel
                ticker.tickLabelFont: typography.axisTickLabel
                ticker.subtickColor: colorPalette.subtick
            }

            QAccelPlot.LineCurve {
                objectName: "temperatureCurve"
                name: "Outdoor temperature"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: colorPalette.seriesPrimary
                lineWidth: 3
            }

            QAccelPlot.LineCurve {
                name: "Six-hour forecast"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: colorPalette.seriesSecondary
                lineWidth: 2
                lineStyle: QAccelPlot.DashLine {
                    pattern: [8, 5]
                }
                markerShape: QAccelPlot.LineCurve.Diamond
                markerSize: 5

                Component.onCompleted: setData([Qt.point(0, 18.0), Qt.point(6, 19.5), Qt.point(12, 25.0), Qt.point(18, 23.5), Qt.point(24, 18.0)])
            }
        }
    }
}
