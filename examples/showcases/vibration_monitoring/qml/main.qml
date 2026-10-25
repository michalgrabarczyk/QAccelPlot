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

    // Acquisition state and channel facts, provided by C++ (MonitorController).
    required property QtObject monitor
    property bool darkTheme: true
    readonly property QtObject colorPalette: darkTheme ? QAccelPlot.Colors.dark : QAccelPlot.Colors.light

    // Recordings use one fixed size, so every run produces the same frames.
    width: monitor.recording ? 896 : 1280
    height: monitor.recording ? 504 : 720
    minimumWidth: 800
    minimumHeight: 450
    visible: true
    title: "Vibration Monitoring"
    color: colorPalette.window
    // A real mouse over a recording window must not disturb the scripted demo.
    flags: monitor.recording ? Qt.Window | Qt.WindowTransparentForInput : Qt.Window
    Material.theme: darkTheme ? Material.Dark : Material.Light
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    // Renders every display frame; each frame requests the next block of data.
    FrameDriver {
        running: !window.monitor.recording
        targetWindow: window
    }

    Shortcut {
        sequence: "Space"
        enabled: !window.monitor.recording
        onActivated: window.monitor.paused = !window.monitor.paused
    }

    // An opaque background, so that text antialiasing blends against it in offscreen captures too.
    Rectangle {
        anchors.fill: parent
        color: window.colorPalette.window
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            // Large enough to stay readable in a GIF scaled down to the README width.
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                Label {
                    text: Math.round(window.monitor.pointCount / 1e6) + "M datapoints · " + window.monitor.channelNames.length + " ch × "
                          + (window.monitor.sampleRate / 1000).toFixed(1) + " kHz · no downsampling"
                    color: window.colorPalette.text
                    font.pixelSize: 20
                    font.bold: true
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Label {
                    text: "P-101 Cooling Water Pump"
                    color: window.colorPalette.textSecondary
                    font.pixelSize: 12
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 6

            WaveformPanel {
                id: waveformPanel

                colorPalette: window.colorPalette
                monitor: window.monitor
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            RmsPanel {
                colorPalette: window.colorPalette
                monitor: window.monitor
                channelAxis: waveformPanel.plot.yAxis
                Layout.preferredWidth: 90
                Layout.fillHeight: true
            }

            ColumnLayout {
                Layout.preferredWidth: window.width * 0.38
                Layout.fillWidth: false
                Layout.fillHeight: true
                spacing: 6

                SpectrumPanel {
                    id: spectrumPanel

                    colorPalette: window.colorPalette
                    monitor: window.monitor
                    curveColor: waveformPanel.channelColor(window.monitor.selectedChannel)
                    rightGutter: spectrogramPanel.keyWidth
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                SpectrogramPanel {
                    id: spectrogramPanel

                    colorPalette: window.colorPalette
                    monitor: window.monitor
                    frequencyAxis: spectrumPanel.plot.xAxis
                    Layout.fillWidth: true
                    Layout.preferredHeight: window.height * 0.47
                }
            }
        }

        StatusStrip {
            monitor: window.monitor
            colorPalette: window.colorPalette
            darkTheme: window.darkTheme
            Layout.fillWidth: true
            onDarkThemeToggled: dark => window.darkTheme = dark
        }
    }

    DemoTimeline {
        anchors.fill: parent
        running: window.monitor.demo
        monitor: window.monitor
        waveformPanel: waveformPanel
        spectrumPanel: spectrumPanel
        time: window.monitor.dataTime
    }
}
