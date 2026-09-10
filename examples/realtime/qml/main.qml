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

    property bool realtimeEnabled: true
    property bool updatesRunning: true
    property real animationElapsedSeconds: 0
    readonly property QtObject colorPalette: QAccelPlot.Colors.dark

    width: 900
    height: 600
    visible: true
    title: "QAccelPlot Realtime"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent

    Typography {
        id: typography
    }

    FrameDriver {
        running: root.realtimeEnabled
        paused: !root.updatesRunning
        onTriggered: root.animationElapsedSeconds = elapsedTime
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 1

                Label {
                    text: "Realtime vibration sensor"
                    color: colorPalette.text
                    font.bold: true
                    font.pixelSize: 20
                }

                Label {
                    text: "A scrolling curve synchronized to rendered frames while retaining a fixed twenty-second window."
                    color: colorPalette.textSecondary
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: root.updatesRunning ? "Pause" : "Resume"
                Material.background: colorPalette.control
                Material.foreground: colorPalette.text
                Material.accent: colorPalette.materialAccent
                onClicked: root.updatesRunning = !root.updatesRunning
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            plotAreaColor: colorPalette.plotArea
            axesAreaColor: colorPalette.axesArea
            grid.gridColor: colorPalette.grid
            grid.subGridVisible: false

            xAxis: QAccelPlot.Axis {
                viewportMin: -20
                viewportMax: 0
                dataMin: -20
                dataMax: 0
                side: QAccelPlot.Axis.Bottom
                label: "Time before present (s)"
                baselineColor: colorPalette.axisLine
                labelColor: colorPalette.axisLabel
                labelFont: typography.axisLabel
                hoverColor: colorPalette.hover
                ticker.tickColor: colorPalette.tick
                ticker.tickLabelColor: colorPalette.axisTickLabel
                ticker.tickLabelFont: typography.axisTickLabel
                ticker.subtickColor: colorPalette.subtick
                ticker.tickCount: 6
            }

            yAxis: QAccelPlot.Axis {
                viewportMin: -2.5
                viewportMax: 2.5
                dataMin: -2.5
                dataMax: 2.5
                side: QAccelPlot.Axis.Left
                axisTitlePadding: 40
                layoutSize: 60
                label: "Acceleration (g)"
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
                objectName: "vibrationCurve"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: colorPalette.seriesTertiary
                lineWidth: 2.5
                antialiasingFeather: 1.25
            }
        }
    }
}
