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

    readonly property QtObject colorPalette: QAccelPlot.Colors.dark
    readonly property real windowSeconds: 10
    readonly property int traceSamples: 600
    property real time: 0

    // A carrier with a slow beat, as a scope would show it.
    function signalAt(t) {
        return Math.sin(2 * Math.PI * 0.9 * t) * (0.6 + 0.3 * Math.sin(0.4 * t));
    }

    function updateTrace() {
        const points = [];
        for (let i = 0; i < traceSamples; ++i) {
            const t = time - windowSeconds + i * windowSeconds / (traceSamples - 1);
            points.push(Qt.point(t, signalAt(t)));
        }
        trace.setData(points);
    }

    // Two clusters of points, so the glow of neighbors overlaps.
    function cloudPoints() {
        const points = [];
        for (let i = 0; i < 400; ++i) {
            const center = i % 2 === 0 ? Qt.point(-1.2, 0.5) : Qt.point(1.1, -0.4);
            const radius = 0.8 * Math.sqrt(Math.random());
            const angle = 2 * Math.PI * Math.random();
            points.push(Qt.point(center.x + radius * Math.cos(angle), center.y + radius * Math.sin(angle)));
        }
        return points;
    }

    width: 1100
    height: 640
    visible: true
    title: "QAccelPlot Effects"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Effects"
            subtitle: "Glow and FadeTrail effects on a streaming trace and a point cloud."
            interactions: [
                "Drag <b>Radius</b> and <b>Intensity</b> to shape the glow",
                "Drag <b>Trail</b> and choose its curve to set how the trace fades",
                "Check <b>Glow</b> or <b>Fade trail</b> to turn an effect off"
            ]
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            CheckBox {
                id: glowCheck
                text: "Glow"
                checked: true
            }

            Label {
                text: "Radius " + radiusSlider.value.toFixed(0) + " px"
                Layout.preferredWidth: 90
            }

            Slider {
                id: radiusSlider
                from: 2
                to: 24
                stepSize: 1
                value: 8
                Layout.preferredWidth: 150
            }

            Label {
                text: "Intensity " + intensitySlider.value.toFixed(2)
                Layout.preferredWidth: 100
            }

            Slider {
                id: intensitySlider
                from: 0.1
                to: 1
                value: 0.6
                Layout.preferredWidth: 150
            }

            Item {
                Layout.fillWidth: true
            }

            CheckBox {
                id: trailCheck
                text: "Fade trail"
                checked: true
            }

            Label {
                text: "Trail " + (trailSlider.value * 100).toFixed(0) + " %"
                Layout.preferredWidth: 80
            }

            Slider {
                id: trailSlider
                from: 0.1
                to: 1
                value: 0.7
                Layout.preferredWidth: 150
            }

            ComboBox {
                id: trailCurveBox
                model: ["In quad", "In cubic", "Linear"]
                Layout.preferredWidth: 120
                Material.background: colorPalette.plotArea
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            QAccelPlot.Plot {
                id: tracePlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 3
                legendVisible: false
                grid.subGridVisible: false

                xAxis: ExampleAxis {
                    viewportMin: window.time - window.windowSeconds
                    viewportMax: window.time
                    label.text: "Time (s)"
                }

                yAxis: ExampleAxis {
                    viewportMin: -1.2
                    viewportMax: 1.2
                    label.text: "Signal"
                    label.padding: 40
                    layoutSize: 60
                }

                QAccelPlot.LineCurve {
                    id: trace
                    xAxis: tracePlot.xAxis
                    yAxis: tracePlot.yAxis
                    color: colorPalette.seriesCyan
                    lineWidth: 2
                    effects: [
                        QAccelPlot.Glow {
                            enabled: glowCheck.checked
                            radius: radiusSlider.value
                            intensity: intensitySlider.value
                        },
                        QAccelPlot.FadeTrail {
                            enabled: trailCheck.checked
                            length: trailSlider.value
                            easing.type: [Easing.InQuad, Easing.InCubic, Easing.Linear][trailCurveBox.currentIndex]
                        }
                    ]
                }
            }

            QAccelPlot.Plot {
                id: cloudPlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 2
                legendVisible: false
                grid.subGridVisible: false

                xAxis: ExampleAxis {
                    viewportMin: -2.5
                    viewportMax: 2.5
                    label.text: "x"
                }

                yAxis: ExampleAxis {
                    viewportMin: -2
                    viewportMax: 2
                    label.text: "y"
                    label.padding: 40
                    layoutSize: 60
                }

                QAccelPlot.PointCloud {
                    xAxis: cloudPlot.xAxis
                    yAxis: cloudPlot.yAxis
                    color: colorPalette.seriesRose
                    marker.size: 3
                    effects: QAccelPlot.Glow {
                        enabled: glowCheck.checked
                        radius: radiusSlider.value
                        intensity: intensitySlider.value
                    }
                    Component.onCompleted: setData(window.cloudPoints())
                }
            }
        }
    }

    Timer {
        interval: 16
        running: true
        repeat: true
        triggeredOnStart: true
        onTriggered: {
            window.time += interval / 1000;
            window.updateTrace();
        }
    }
}
