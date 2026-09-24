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
    property int nanGapModeIndex: 0
    property real lineWidth: 3.0

    readonly property var nanGapModes: [QAccelPlot.NanGapMode.Break, QAccelPlot.NanGapMode.Connect]
    readonly property int activeNanGapMode: nanGapModes[nanGapModeIndex]

    // Pump pressure sampled once per second with lost packets marked as NaN:
    // a single missing sample, a longer outage, a corrupt +Inf reading, and an
    // isolated valid sample between two losses.
    function pressurePoints() {
        const points = [];
        for (let second = 0; second <= 120; ++second) {
            let pressure = 4.2 + 1.1 * Math.sin(second / 9) + 0.35 * Math.sin(second / 2.3);
            if (second === 14 || (second >= 38 && second <= 49) || second === 83 || second === 85)
                pressure = NaN;
            else if (second === 66)
                pressure = Infinity;
            points.push(Qt.point(second, pressure));
        }
        return points;
    }

    // Vibration envelope on a logarithmic axis. Readings at or below zero are
    // invalid for a log scale and render as gaps instead of spikes.
    function vibrationPoints() {
        const points = [];
        for (let hertz = 0; hertz <= 200; hertz += 1) {
            const resonance = 60 * Math.exp(-Math.pow((hertz - 120) / 14, 2));
            points.push(Qt.point(hertz, 1.5 * Math.cos(hertz / 11) + resonance + 0.4));
        }
        return points;
    }

    Gradient {
        id: pressureGradient
        GradientStop {
            position: 0.0
            color: root.palette.seriesPrimary
        }
        GradientStop {
            position: 1.0
            color: Qt.rgba(root.palette.seriesPrimary.r, root.palette.seriesPrimary.g, root.palette.seriesPrimary.b, 0.0)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: "Telemetry often contains missing or invalid samples. NaN, ±Inf, and non-positive values on a log axis are invalid: Break leaves a gap, Connect joins the neighboring valid samples. Invalid samples never draw markers and are excluded from auto-ranging."
            color: root.palette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "NaN gap mode"
                color: root.palette.text
            }
            ComboBox {
                model: ["Break", "Connect"]
                currentIndex: root.nanGapModeIndex
                Layout.preferredWidth: 140
                Material.background: root.palette.plotArea
                onActivated: root.nanGapModeIndex = currentIndex
            }
            Label {
                text: "Line width " + root.lineWidth.toFixed(0) + " px"
                color: root.palette.text
            }
            Slider {
                from: 1
                to: 12
                stepSize: 1
                value: root.lineWidth
                Layout.preferredWidth: 160
                onMoved: root.lineWidth = value
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
                id: pressurePlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                legendVisible: false
                plotAreaColor: root.palette.plotArea
                axesAreaColor: root.palette.axesArea
                grid.gridColor: root.palette.grid
                grid.subGridVisible: false

                xAxis: ExampleAxis {
                    colorPalette: root.palette
                    viewportMin: 0
                    viewportMax: 120
                    dataMin: 0
                    dataMax: 120
                    label: "Time (s)"
                }

                yAxis: ExampleAxis {
                    colorPalette: root.palette
                    viewportMin: 2
                    viewportMax: 6.5
                    dataMin: 2
                    dataMax: 6.5
                    axisTitlePadding: 40
                    layoutSize: 60
                    label: "Pressure (bar)"
                }

                QAccelPlot.LineCurve {
                    xAxis: pressurePlot.xAxis
                    yAxis: pressurePlot.yAxis
                    color: root.palette.seriesPrimary
                    lineWidth: root.lineWidth
                    gaps.nanMode: root.activeNanGapMode
                    effects: [
                        QAccelPlot.GradientFill {
                            direction: QAccelPlot.GradientDirection.Vertical
                            baseline: QAccelPlot.GradientFillBaseline.AxisMinimum
                            opacity: 0.45
                            gradient: pressureGradient
                        }
                    ]
                    Component.onCompleted: setData(root.pressurePoints())
                }
            }

            QAccelPlot.Plot {
                id: markerPlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                legendVisible: false
                plotAreaColor: root.palette.plotArea
                axesAreaColor: root.palette.axesArea
                grid.gridColor: root.palette.grid
                grid.subGridVisible: false

                xAxis: ExampleAxis {
                    colorPalette: root.palette
                    viewportMin: 0
                    viewportMax: 120
                    dataMin: 0
                    dataMax: 120
                    label: "Time (s)"
                }

                yAxis: ExampleAxis {
                    colorPalette: root.palette
                    viewportMin: 2
                    viewportMax: 6.5
                    dataMin: 2
                    dataMax: 6.5
                    axisTitlePadding: 40
                    layoutSize: 60
                    label: "Pressure (bar)"
                }

                QAccelPlot.LineCurve {
                    xAxis: markerPlot.xAxis
                    yAxis: markerPlot.yAxis
                    color: root.palette.seriesSecondary
                    lineWidth: root.lineWidth
                    gaps.nanMode: root.activeNanGapMode
                    lineStyle: QAccelPlot.DashLine {
                        pattern: [6, 4]
                    }
                    marker.shape: QAccelPlot.LineCurve.Circle
                    marker.size: 6
                    Component.onCompleted: {
                        const points = root.pressurePoints().filter((point, index) => index % 3 === 0 || !isFinite(point.y));
                        setData(points);
                    }
                }
            }

            QAccelPlot.Plot {
                id: vibrationPlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                legendVisible: false
                plotAreaColor: root.palette.plotArea
                axesAreaColor: root.palette.axesArea
                grid.gridColor: root.palette.grid
                grid.subGridVisible: false

                xAxis: ExampleAxis {
                    colorPalette: root.palette
                    viewportMin: 0
                    viewportMax: 200
                    dataMin: 0
                    dataMax: 200
                    label: "Frequency (Hz)"
                }

                yAxis: ExampleAxis {
                    colorPalette: root.palette
                    viewportMin: 0.01
                    viewportMax: 100
                    dataMin: 0.01
                    dataMax: 100
                    logScale: true
                    axisTitlePadding: 40
                    layoutSize: 60
                    label: "Amplitude (log)"
                }

                QAccelPlot.LineCurve {
                    xAxis: vibrationPlot.xAxis
                    yAxis: vibrationPlot.yAxis
                    color: root.palette.seriesTertiary
                    lineWidth: root.lineWidth
                    gaps.nanMode: root.activeNanGapMode
                    Component.onCompleted: setData(root.vibrationPoints())
                }
            }
        }
    }
}
