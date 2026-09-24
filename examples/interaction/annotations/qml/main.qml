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
    title: "QAccelPlot Annotations"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    // Anchors children to data coordinates of the plot below.
    component PlotAnchor: QAccelPlot.DataAnchor {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        plotRect: Qt.rect(0, 0, annotationLayer.width, annotationLayer.height)
    }

    component CalloutLabel: Rectangle {
        property alias text: label.text
        property color accentColor

        width: label.implicitWidth + 12
        height: label.implicitHeight + 8
        radius: 3
        color: colorPalette.control
        border.color: accentColor

        Text {
            id: label
            anchors.centerIn: parent
            color: colorPalette.text
            font.pixelSize: 12
        }
    }

    component PointMarker: Rectangle {
        anchors.centerIn: parent
        width: 10
        height: 10
        radius: 5
        border.color: colorPalette.annotationMarkerOutline
        border.width: 1
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Process telemetry annotations"
            description: "Peak, valley, event marker, and warning range stay attached to data while you pan or zoom. Hover the event marker or warning range."
        }

        QAccelPlot.Plot {
            id: plot

            Layout.fillWidth: true
            Layout.fillHeight: true
            border.color: colorPalette.plotBorder
            border.width: 3

            xAxis: ExampleAxis {
                viewportMin: 0
                viewportMax: 100
                dataMin: 0
                dataMax: 100
                label: "Process time (s)"
                baselineWidth: 3
            }

            yAxis: ExampleAxis {
                viewportMin: -6
                viewportMax: 8
                dataMin: -6
                dataMax: 8
                axisTitlePadding: 40
                layoutSize: 60
                label: "Pressure response (units)"
                baselineWidth: 3
            }

            // Clips annotations to the drawable plot area.
            Item {
                id: annotationLayer
                z: 1
                x: plot.plotRect.x
                y: plot.plotRect.y
                width: plot.plotRect.width
                height: plot.plotRect.height
                clip: true

                // Event: a vertical line spanning the visible Y range.
                PlotAnchor {
                    dataX1: 42
                    dataY1: plot.yAxis.viewportMin
                    dataX2: 42
                    dataY2: plot.yAxis.viewportMax

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

                    CalloutLabel {
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.topMargin: 10
                        anchors.leftMargin: 6
                        text: "Valve opened · 42 s"
                        accentColor: colorPalette.annotationEvent
                    }
                }

                // Points: a zero-size anchor at a data coordinate.
                PlotAnchor {
                    dataX1: window.peakX
                    dataY1: window.peakY
                    dataX2: window.peakX
                    dataY2: window.peakY

                    PointMarker {
                        color: colorPalette.annotationPeak
                    }

                    CalloutLabel {
                        anchors.left: parent.right
                        anchors.bottom: parent.top
                        anchors.leftMargin: 6
                        anchors.bottomMargin: 4
                        text: "Peak pressure · " + window.peakY.toFixed(1)
                        accentColor: colorPalette.annotationPeak
                    }
                }

                PlotAnchor {
                    dataX1: window.valleyX
                    dataY1: window.valleyY
                    dataX2: window.valleyX
                    dataY2: window.valleyY

                    PointMarker {
                        color: colorPalette.annotationValley
                    }

                    CalloutLabel {
                        anchors.left: parent.right
                        anchors.top: parent.bottom
                        anchors.leftMargin: 6
                        anchors.topMargin: 4
                        text: "Pressure drop · " + window.valleyY.toFixed(1)
                        accentColor: colorPalette.annotationValley
                    }
                }

                // Range: a band spanning the visible X range.
                PlotAnchor {
                    dataX1: plot.xAxis.viewportMin
                    dataY1: 4
                    dataX2: plot.xAxis.viewportMax
                    dataY2: 6

                    HoverHandler {
                        id: warningRangeHover
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: colorPalette.annotationRangeFill
                        opacity: warningRangeHover.hovered ? 1.0 : 0.6
                    }

                    CalloutLabel {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 6
                        text: "Warning range · 4–6 units"
                        accentColor: colorPalette.annotationRange
                    }
                }
            }

            QAccelPlot.LineCurve {
                objectName: "pressureCurve"
                name: "Pressure response"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: colorPalette.seriesPrimary
                lineWidth: hovered ? 4 : 2
            }
        }
    }
}
