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
Item {
    id: root
    required property var palette
    required property var typography
    property int markerShapeIndex: 0
    property bool antialiasingEnabled: true
    property real antialiasingFeather: 1.0

    readonly property var markerNames: ["Circle", "Square", "Diamond", "Triangle", "Cross"]
    readonly property var markerShapes: [QAccelPlot.LineCurve.Circle, QAccelPlot.LineCurve.Square, QAccelPlot.LineCurve.Diamond, QAccelPlot.LineCurve.TriangleUp, QAccelPlot.LineCurve.Cross]

    function curvePoints(offset) {
        const values = [0.12, 0.32, 0.20, 0.58, 0.42, 0.76, 0.62, 0.88, 0.71, 0.93, 0.80, 1.00];
        const points = [];
        for (let i = 0; i < values.length; ++i)
            points.push(Qt.point(i, values[i] + offset));
        return points;
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: "Compare marker shapes, line patterns, and line weights using the same fixed data."
            color: root.palette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "Marker plot controls"
                color: root.palette.text
                font.bold: true
            }
            Label {
                text: "Shape"
                color: root.palette.text
            }
            ComboBox {
                model: root.markerNames
                currentIndex: root.markerShapeIndex
                Layout.preferredWidth: 140
                Material.background: root.palette.plotArea
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onActivated: root.markerShapeIndex = currentIndex
            }
            Switch {
                text: "Antialiasing"
                checked: root.antialiasingEnabled
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onToggled: root.antialiasingEnabled = checked
            }
            Label {
                text: "Feather " + root.antialiasingFeather.toFixed(1) + " px"
                color: root.palette.text
                opacity: root.antialiasingEnabled ? 1.0 : 0.45
            }
            Slider {
                from: 0
                to: 4
                stepSize: 0.1
                value: root.antialiasingFeather
                enabled: root.antialiasingEnabled
                Layout.preferredWidth: 120
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onMoved: root.antialiasingFeather = value
            }
            Item {
                Layout.fillWidth: true
            }
        }

        ColumnLayout {
            spacing: 10
            Layout.fillWidth: true
            Layout.fillHeight: true

            QAccelPlot.Plot {
                id: markerPlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                legendVisible: false
                plotAreaColor: root.palette.plotArea
                axesAreaColor: root.palette.axesArea
                grid.gridColor: root.palette.grid
                grid.subGridVisible: false

                xAxis: QAccelPlot.Axis {
                    viewportMin: 0
                    viewportMax: 11
                    dataMin: 0
                    dataMax: 11
                    side: QAccelPlot.Axis.Bottom
                    label: "Sample"
                    baselineColor: root.palette.axisLine
                    labelColor: root.palette.axisLabel
                    labelFont: root.typography.axisLabel
                    hoverColor: root.palette.hover
                    ticker.tickColor: root.palette.tick
                    ticker.tickLabelColor: root.palette.axisTickLabel
                    ticker.tickLabelFont: root.typography.axisTickLabel
                    ticker.subtickColor: root.palette.subtick
                }

                yAxis: QAccelPlot.Axis {
                    viewportMin: 0
                    viewportMax: 1.1
                    dataMin: 0
                    dataMax: 1.1
                    side: QAccelPlot.Axis.Left
                    axisTitlePadding: 40
                    label: "Signal"
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
                    xAxis: markerPlot.xAxis
                    yAxis: markerPlot.yAxis
                    color: root.palette.seriesPrimary
                    lineWidth: 2
                    markerSize: 7
                    markerShape: root.markerShapes[root.markerShapeIndex]
                    antialiasingEnabled: root.antialiasingEnabled
                    antialiasingFeather: root.antialiasingFeather
                    Component.onCompleted: setData(root.curvePoints(0))
                }
            }

            QAccelPlot.Plot {
                id: dashPlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                plotAreaColor: root.palette.plotArea
                axesAreaColor: root.palette.axesArea
                grid.gridColor: root.palette.grid
                grid.subGridVisible: false
                legend: QAccelPlot.Legend {
                    series: dashPlot.series
                    symbolWidth: 34
                    color: root.palette.legendBackground
                    textColor: root.palette.text
                    border.color: root.palette.legendBorder
                }

                xAxis: QAccelPlot.Axis {
                    viewportMin: 0
                    viewportMax: 11
                    dataMin: 0
                    dataMax: 11
                    side: QAccelPlot.Axis.Bottom
                    label: "Sample"
                    baselineColor: root.palette.axisLine
                    labelColor: root.palette.axisLabel
                    labelFont: root.typography.axisLabel
                    hoverColor: root.palette.hover
                    ticker.tickColor: root.palette.tick
                    ticker.tickLabelColor: root.palette.axisTickLabel
                    ticker.tickLabelFont: root.typography.axisTickLabel
                    ticker.subtickColor: root.palette.subtick
                }
                yAxis: QAccelPlot.Axis {
                    viewportMin: -0.1
                    viewportMax: 4.1
                    dataMin: -0.1
                    dataMax: 4.1
                    side: QAccelPlot.Axis.Left
                    axisTitlePadding: 40
                    label: "Style offset"
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
                    name: "Solid"
                    xAxis: dashPlot.xAxis
                    yAxis: dashPlot.yAxis
                    color: root.palette.seriesPrimary
                    lineWidth: 2
                    antialiasingEnabled: root.antialiasingEnabled
                    antialiasingFeather: root.antialiasingFeather
                    Component.onCompleted: setData(root.curvePoints(3.0))
                }
                QAccelPlot.LineCurve {
                    name: "Dash"
                    xAxis: dashPlot.xAxis
                    yAxis: dashPlot.yAxis
                    color: root.palette.seriesQuaternary
                    lineWidth: 2
                    lineStyle: QAccelPlot.DashLine {
                        pattern: [12, 6]
                    }
                    antialiasingEnabled: root.antialiasingEnabled
                    antialiasingFeather: root.antialiasingFeather
                    Component.onCompleted: setData(root.curvePoints(2.0))
                }
                QAccelPlot.LineCurve {
                    name: "Dot-dash"
                    xAxis: dashPlot.xAxis
                    yAxis: dashPlot.yAxis
                    color: root.palette.seriesTertiary
                    lineWidth: 2
                    lineStyle: QAccelPlot.DashLine {
                        pattern: [14, 5, 3, 5]
                    }
                    antialiasingEnabled: root.antialiasingEnabled
                    antialiasingFeather: root.antialiasingFeather
                    Component.onCompleted: setData(root.curvePoints(1.0))
                }
                QAccelPlot.LineCurve {
                    name: "Dots"
                    xAxis: dashPlot.xAxis
                    yAxis: dashPlot.yAxis
                    color: root.palette.seriesSecondary
                    lineWidth: 2
                    lineStyle: QAccelPlot.DashLine {
                        pattern: [2, 6]
                    }
                    antialiasingEnabled: root.antialiasingEnabled
                    antialiasingFeather: root.antialiasingFeather
                    Component.onCompleted: setData(root.curvePoints(0.0))
                }
            }

            QAccelPlot.Plot {
                id: weightPlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                plotAreaColor: root.palette.plotArea
                axesAreaColor: root.palette.axesArea
                grid.gridColor: root.palette.grid
                grid.subGridVisible: false
                legend: QAccelPlot.Legend {
                    series: weightPlot.series
                    symbolWidth: 34
                    color: root.palette.legendBackground
                    textColor: root.palette.text
                    border.color: root.palette.legendBorder
                }

                xAxis: QAccelPlot.Axis {
                    viewportMin: 0
                    viewportMax: 11
                    dataMin: 0
                    dataMax: 11
                    side: QAccelPlot.Axis.Bottom
                    label: "Sample"
                    baselineColor: root.palette.axisLine
                    labelColor: root.palette.axisLabel
                    labelFont: root.typography.axisLabel
                    hoverColor: root.palette.hover
                    ticker.tickColor: root.palette.tick
                    ticker.tickLabelColor: root.palette.axisTickLabel
                    ticker.tickLabelFont: root.typography.axisTickLabel
                    ticker.subtickColor: root.palette.subtick
                }
                yAxis: QAccelPlot.Axis {
                    viewportMin: -0.1
                    viewportMax: 4.1
                    dataMin: -0.1
                    dataMax: 4.1
                    side: QAccelPlot.Axis.Left
                    axisTitlePadding: 40
                    label: "Line weight"
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
                    name: "1 px"
                    xAxis: weightPlot.xAxis
                    yAxis: weightPlot.yAxis
                    color: root.palette.seriesMuted
                    lineWidth: 1
                    antialiasingEnabled: root.antialiasingEnabled
                    antialiasingFeather: root.antialiasingFeather
                    Component.onCompleted: setData(root.curvePoints(3.0))
                }
                QAccelPlot.LineCurve {
                    name: "2 px"
                    xAxis: weightPlot.xAxis
                    yAxis: weightPlot.yAxis
                    color: root.palette.seriesPrimary
                    lineWidth: 2
                    antialiasingEnabled: root.antialiasingEnabled
                    antialiasingFeather: root.antialiasingFeather
                    Component.onCompleted: setData(root.curvePoints(2.0))
                }
                QAccelPlot.LineCurve {
                    name: "4 px"
                    xAxis: weightPlot.xAxis
                    yAxis: weightPlot.yAxis
                    color: root.palette.seriesQuaternary
                    lineWidth: 4
                    antialiasingEnabled: root.antialiasingEnabled
                    antialiasingFeather: root.antialiasingFeather
                    Component.onCompleted: setData(root.curvePoints(1.0))
                }
                QAccelPlot.LineCurve {
                    name: "6 px"
                    xAxis: weightPlot.xAxis
                    yAxis: weightPlot.yAxis
                    color: root.palette.seriesSecondary
                    lineWidth: 6
                    antialiasingEnabled: root.antialiasingEnabled
                    antialiasingFeather: root.antialiasingFeather
                    Component.onCompleted: setData(root.curvePoints(0.0))
                }
            }
        }
    }
}
