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
    Material.foreground: colorPalette.text

    FrameDriver {
        running: root.realtimeEnabled
        paused: !root.updatesRunning
        onTriggered: root.animationElapsedSeconds = elapsedTime
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Realtime vibration sensor"
            description: "A scrolling curve synchronized to rendered frames while retaining a fixed twenty-second window."

            Button {
                text: root.updatesRunning ? "Pause" : "Resume"
                Material.background: colorPalette.control
                onClicked: root.updatesRunning = !root.updatesRunning
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                viewportMin: -20
                viewportMax: 0
                dataMin: -20
                dataMax: 0
                label: "Time before present (s)"
                ticker.tickCount: 6
            }

            yAxis: ExampleAxis {
                viewportMin: -2.5
                viewportMax: 2.5
                dataMin: -2.5
                dataMax: 2.5
                axisTitlePadding: 40
                layoutSize: 60
                label: "Acceleration (g)"
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
