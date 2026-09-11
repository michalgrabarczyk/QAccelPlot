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

    property bool metricsEnabled: true
    property int pointCount: 1000000
    property bool rectanglesVisible: false
    property int rectangleCount: 1000
    property int fps: 60
    property real averageFrameTimeMs: 16.7
    property int updateRate: 60
    property real longestDataGapMs: 0
    readonly property real curveThroughputMillions: pointCount * updateRate / 1000000
    readonly property QtObject colorPalette: QAccelPlot.Colors.dark

    width: 1400
    height: 900
    visible: true
    title: "QAccelPlot Performance Showcase"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent

    Typography {
        id: typography
    }

    FrameDriver {
        running: true
        targetWindow: window
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        ControlPanel {
            Layout.preferredWidth: 240
            Layout.fillHeight: true
            window: window
            palette: colorPalette
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            Label {
                text: window.pointCount.toLocaleString(Qt.locale("en_US"), "f", 0) + " live points"
                color: colorPalette.text
                font.bold: true
                font.pixelSize: 20
            }

            Label {
                text: "Display FPS counts frames presented to the screen. Data Update Rate shows how many new curve datasets are applied to the plot each second."
                color: colorPalette.textSecondary
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            GridLayout {
                columns: 4
                columnSpacing: 8
                Layout.fillWidth: true

                Repeater {
                    model: [
                        {
                            label: "DISPLAY FPS",
                            value: window.fps.toString(),
                            detail: "frames presented / second"
                        },
                        {
                            label: "DATA UPDATE RATE",
                            value: window.updateRate.toString() + " Hz",
                            detail: "peak update gap " + window.longestDataGapMs.toFixed(0) + " ms"
                        },
                        {
                            label: "FRAME TIME",
                            value: window.averageFrameTimeMs.toFixed(1) + " ms",
                            detail: "between presents"
                        },
                        {
                            label: "CURVE THROUGHPUT",
                            value: window.curveThroughputMillions.toFixed(1) + " M",
                            detail: "points updated / second"
                        }
                    ]

                    delegate: Rectangle {
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.preferredHeight: 70
                        color: colorPalette.control
                        border.color: colorPalette.outline
                        radius: 4

                        Column {
                            anchors.centerIn: parent
                            spacing: 2

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.label
                                color: colorPalette.textSecondary
                                font.bold: true
                                font.pixelSize: 11
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.value
                                color: colorPalette.text
                                font.bold: true
                                font.pixelSize: 22
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.detail
                                color: colorPalette.textMuted
                                font.pixelSize: 10
                            }
                        }
                    }
                }
            }

            QAccelPlot.Plot {
                id: plot1
                Layout.fillWidth: true
                Layout.fillHeight: true
                legendVisible: false
                plotAreaColor: colorPalette.plotArea
                axesAreaColor: colorPalette.axesArea
                grid.gridColor: colorPalette.grid
                grid.subGridColor: colorPalette.subGrid

                xAxis: QAccelPlot.Axis {
                    viewportMin: 0
                    viewportMax: 1000
                    dataMin: 0
                    dataMax: 1000
                    label: "Sample domain"
                    baselineColor: colorPalette.axisLine
                    labelColor: colorPalette.axisLabel
                    labelFont: typography.axisLabel
                    hoverColor: colorPalette.hover
                    ticker.tickColor: colorPalette.tick
                    ticker.tickLabelColor: colorPalette.axisTickLabel
                    ticker.tickLabelFont: typography.axisTickLabel
                    ticker.subtickColor: colorPalette.subtick
                }

                yAxis: QAccelPlot.Axis {
                    viewportMin: -10
                    viewportMax: 10
                    dataMin: -10
                    dataMax: 10
                    axisTitlePadding: 40
                    layoutSize: 60
                    label: "Amplitude"
                    baselineColor: colorPalette.axisLine
                    labelColor: colorPalette.axisLabel
                    labelFont: typography.axisLabel
                    hoverColor: colorPalette.hover
                    ticker.tickColor: colorPalette.tick
                    ticker.tickLabelColor: colorPalette.axisTickLabel
                    ticker.tickLabelFont: typography.axisTickLabel
                    ticker.subtickColor: colorPalette.subtick
                }

                QAccelPlot.RectangleList {
                    objectName: "rectangleList"
                    z: 1
                    xAxis: plot1.xAxis
                    yAxis: plot1.yAxis
                    plotRect: plot1.plotRect
                    color: colorPalette.performanceRectangles
                }

                QAccelPlot.LineCurve {
                    objectName: "curve1"
                    z: 2
                    xAxis: plot1.xAxis
                    yAxis: plot1.yAxis
                    color: colorPalette.performanceCurve
                    lineWidth: 3
                }
            }
        }
    }
}
