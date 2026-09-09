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
    id: window

    property int pointCount: 1001
    property real peakX: 24
    property real peakY: 6
    property real valleyX: 58
    property real valleyY: -5
    readonly property QtObject colorPalette: QAccelPlot.Colors.dark

    width: 1200
    height: 800
    visible: true
    title: "Annotations Example"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent

    Typography {
        id: typography
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        Label {
            text: "Process telemetry annotations"
            color: colorPalette.text
            font.bold: true
            font.pixelSize: 16
        }

        Label {
            text: "Peak, valley, event marker, and warning range stay attached to data while you pan or zoom. Hover the event marker or warning range."
            color: colorPalette.textSecondary
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        QAccelPlot.Plot {
            id: plot1

            Layout.fillWidth: true
            Layout.fillHeight: true
            plotAreaColor: colorPalette.plotArea
            axesAreaColor: colorPalette.axesArea
            border.color: colorPalette.plotBorder
            border.width: 3
            grid.gridColor: colorPalette.grid
            grid.subGridColor: colorPalette.subGrid
            legend: QAccelPlot.Legend {
                series: plot1.series
                color: colorPalette.legendBackground
                textColor: colorPalette.text
                border.color: colorPalette.legendBorder
            }

            xAxis: QAccelPlot.Axis {
                viewportMin: 0
                viewportMax: 100
                dataMin: 0
                dataMax: 100
                side: QAccelPlot.Axis.Bottom
                label: "Process time (s)"
                baselineColor: colorPalette.axisLine
                baselineWidth: 3
                labelColor: colorPalette.axisLabel
                labelFont: typography.axisLabel
                hoverColor: colorPalette.hover
                ticker.tickColor: colorPalette.tick
                ticker.tickLabelColor: colorPalette.axisTickLabel
                ticker.tickLabelFont: typography.axisTickLabel
                ticker.subtickColor: colorPalette.subtick
            }

            yAxis: QAccelPlot.Axis {
                viewportMin: -6
                viewportMax: 8
                dataMin: -6
                dataMax: 8
                side: QAccelPlot.Axis.Left
                label: "Pressure response (units)"
                baselineColor: colorPalette.axisLine
                baselineWidth: 3
                labelColor: colorPalette.axisLabel
                labelFont: typography.axisLabel
                hoverColor: colorPalette.hover
                ticker.tickColor: colorPalette.tick
                ticker.tickLabelColor: colorPalette.axisTickLabel
                ticker.tickLabelFont: typography.axisTickLabel
                ticker.subtickColor: colorPalette.subtick
            }

            Item {
                id: annotationLayer
                z: 1
                x: plot1.plotRect.x
                y: plot1.plotRect.y
                width: plot1.plotRect.width
                height: plot1.plotRect.height
                clip: true

                QAccelPlot.DataAnchor {
                    xAxis: plot1.xAxis
                    yAxis: plot1.yAxis
                    plotRect: Qt.rect(0, 0, annotationLayer.width, annotationLayer.height)
                    dataX1: 42
                    dataY1: plot1.yAxis.viewportMin
                    dataX2: 42
                    dataY2: plot1.yAxis.viewportMax

                    HoverHandler {
                        id: eventMarkerHover
                    }

                    Rectangle {
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.left
                        width: eventMarkerHover.hovered ? 5 : 3
                        color: colorPalette.annotationEvent
                    }

                    Rectangle {
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.topMargin: 10
                        anchors.leftMargin: 5
                        width: eventLabel.implicitWidth + 12
                        height: eventLabel.implicitHeight + 8
                        color: colorPalette.control
                        border.color: colorPalette.annotationEvent
                        radius: 4

                        Text {
                            id: eventLabel
                            anchors.centerIn: parent
                            color: colorPalette.text
                            text: "Valve opened · 42 s"
                            font.pixelSize: 11
                        }
                    }
                }

                QAccelPlot.DataAnchor {
                    xAxis: plot1.xAxis
                    yAxis: plot1.yAxis
                    plotRect: Qt.rect(0, 0, annotationLayer.width, annotationLayer.height)
                    dataX1: window.peakX
                    dataY1: window.peakY
                    dataX2: window.peakX
                    dataY2: window.peakY

                    Rectangle {
                        anchors.centerIn: parent
                        width: 10
                        height: 10
                        radius: 5
                        color: colorPalette.annotationPeak
                        border.color: colorPalette.annotationMarkerOutline
                        border.width: 1
                    }

                    Rectangle {
                        anchors.left: parent.right
                        anchors.bottom: parent.top
                        anchors.leftMargin: 6
                        anchors.bottomMargin: 4
                        width: peakLabel.implicitWidth + 10
                        height: peakLabel.implicitHeight + 6
                        color: colorPalette.control
                        border.color: colorPalette.annotationPeak
                        radius: 3

                        Text {
                            id: peakLabel
                            anchors.centerIn: parent
                            color: colorPalette.text
                            text: "Peak pressure · " + window.peakY.toFixed(1)
                            font.pixelSize: 12
                        }
                    }
                }

                QAccelPlot.DataAnchor {
                    xAxis: plot1.xAxis
                    yAxis: plot1.yAxis
                    plotRect: Qt.rect(0, 0, annotationLayer.width, annotationLayer.height)
                    dataX1: window.valleyX
                    dataY1: window.valleyY
                    dataX2: window.valleyX
                    dataY2: window.valleyY

                    Rectangle {
                        anchors.centerIn: parent
                        width: 10
                        height: 10
                        radius: 5
                        color: colorPalette.annotationValley
                        border.color: colorPalette.annotationMarkerOutline
                        border.width: 1
                    }

                    Rectangle {
                        anchors.left: parent.right
                        anchors.top: parent.bottom
                        anchors.leftMargin: 6
                        anchors.topMargin: 4
                        width: valleyLabel.implicitWidth + 10
                        height: valleyLabel.implicitHeight + 6
                        color: colorPalette.control
                        border.color: colorPalette.annotationValley
                        radius: 3

                        Text {
                            id: valleyLabel
                            anchors.centerIn: parent
                            color: colorPalette.text
                            text: "Pressure drop · " + window.valleyY.toFixed(1)
                            font.pixelSize: 12
                        }
                    }
                }

                QAccelPlot.DataAnchor {
                    xAxis: plot1.xAxis
                    yAxis: plot1.yAxis
                    plotRect: Qt.rect(0, 0, annotationLayer.width, annotationLayer.height)
                    dataX1: plot1.xAxis.viewportMin
                    dataY1: 4
                    dataX2: plot1.xAxis.viewportMax
                    dataY2: 6

                    HoverHandler {
                        id: warningRangeHover
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: colorPalette.annotationRangeFill
                        opacity: warningRangeHover.hovered ? 1.0 : 0.6
                    }

                    Rectangle {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 6
                        width: warningLabel.implicitWidth + 10
                        height: warningLabel.implicitHeight + 6
                        color: colorPalette.control
                        border.color: colorPalette.annotationRange
                        radius: 3

                        Text {
                            id: warningLabel
                            anchors.centerIn: parent
                            color: colorPalette.text
                            text: "Warning range · 4–6 units"
                            font.pixelSize: 11
                        }
                    }
                }
            }

            QAccelPlot.LineCurve {
                objectName: "curve1"
                name: "Pressure response"
                xAxis: plot1.xAxis
                yAxis: plot1.yAxis
                color: colorPalette.seriesPrimary
                lineWidth: hovered ? 4 : 2
            }
        }
    }
}
