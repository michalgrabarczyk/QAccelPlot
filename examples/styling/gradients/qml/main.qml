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
    property int presetIndex: 0
    property int directionIndex: 0
    property int baselineIndex: 0
    property real gradientOpacity: 0.75
    property real verticalMaximumPercent: 100.0

    readonly property var presetNames: ["Viridis", "Plasma", "Inferno", "Magma", "Turbo"]
    readonly property var presets: [QAccelPlot.Colormap.Viridis, QAccelPlot.Colormap.Plasma, QAccelPlot.Colormap.Inferno, QAccelPlot.Colormap.Magma, QAccelPlot.Colormap.Turbo]
    readonly property real solarVerticalMaximum: -4.0 + (verticalMaximumPercent / 100.0) * 10.0
    readonly property real batteryVerticalMaximum: 20.0 + (verticalMaximumPercent / 100.0) * 10.0

    function netGridPowerAt(hoursFromNoon) {
        const daylight = Math.max(0, Math.cos(Math.PI * hoursFromNoon / 12));
        const solarGeneration = 7.2 * Math.pow(daylight, 1.8);
        const passingCloud = 1 - 0.22 * Math.exp(-Math.pow((hoursFromNoon - 1.4) / 0.55, 2));
        const baseConsumption = 2.0;
        const morningDemand = 0.8 * Math.exp(-Math.pow((hoursFromNoon + 4.8) / 1.0, 2));
        const eveningDemand = 1.1 * Math.exp(-Math.pow((hoursFromNoon - 5.0) / 1.1, 2));
        return solarGeneration * passingCloud - baseConsumption - morningDemand - eveningDemand;
    }

    QAccelPlot.Colormap {
        id: activeColormap
        preset: window.presets[window.presetIndex]
    }

    width: 900
    height: 960
    visible: true
    title: "QAccelPlot Gradients"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Gradients"
            description: "Upper plot: a home imports power below 0 kW and exports surplus solar power above it. Compare filling from the chart minimum with filling from the meaningful 0 kW baseline. Lower plot: a gradient stroke on battery temperature during a charge cycle, using the same settings."
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "Preset"
            }
            ComboBox {
                model: window.presetNames
                currentIndex: window.presetIndex
                Layout.preferredWidth: 175
                Material.background: colorPalette.plotArea
                onActivated: window.presetIndex = currentIndex
            }
            Label {
                text: "Direction"
            }
            ComboBox {
                model: ["Vertical", "Horizontal"]
                currentIndex: window.directionIndex
                Layout.preferredWidth: 120
                Material.background: colorPalette.plotArea
                onActivated: window.directionIndex = currentIndex
            }
            Label {
                text: "Baseline"
            }
            ComboBox {
                model: ["Axis minimum", "Grid balance (0 kW)"]
                currentIndex: window.baselineIndex
                Layout.preferredWidth: 175
                Material.background: colorPalette.plotArea
                onActivated: window.baselineIndex = currentIndex
            }
            Item {
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "Fill opacity " + Math.round(window.gradientOpacity * 100) + "%"
            }
            Slider {
                from: 0.1
                to: 1.0
                stepSize: 0.05
                value: window.gradientOpacity
                Layout.preferredWidth: 190
                onMoved: window.gradientOpacity = value
            }
            Label {
                text: "Vertical maximum " + window.verticalMaximumPercent.toFixed(0) + "% of Y-axis range"
                opacity: window.directionIndex === 0 ? 1.0 : 0.45
            }
            Slider {
                from: 1.0
                to: 100.0
                stepSize: 1.0
                value: window.verticalMaximumPercent
                enabled: window.directionIndex === 0
                Layout.preferredWidth: 190
                onMoved: window.verticalMaximumPercent = value
            }
            Item {
                Layout.fillWidth: true
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            QAccelPlot.Plot {
                id: solarPlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: 1
                legendVisible: false
                grid.subGridVisible: false

                xAxis: ExampleAxis {
                    viewportMin: -6
                    viewportMax: 6
                    dataMin: -6
                    dataMax: 6
                    label: "Hours from solar noon"
                }

                yAxis: ExampleAxis {
                    viewportMin: -4
                    viewportMax: 6
                    dataMin: -4
                    dataMax: 6
                    axisTitlePadding: 40
                    layoutSize: 60
                    label: "Net grid power (kW)"
                }

                QAccelPlot.LineCurve {
                    xAxis: solarPlot.xAxis
                    yAxis: solarPlot.yAxis
                    color: colorPalette.text
                    lineWidth: 3
                    effects: [
                        QAccelPlot.GradientFill {
                            direction: window.directionIndex === 0 ? QAccelPlot.GradientDirection.Vertical : QAccelPlot.GradientDirection.Horizontal
                            baseline: window.baselineIndex === 0 ? QAccelPlot.GradientFillBaseline.AxisMinimum : QAccelPlot.GradientFillBaseline.Value
                            baselineValue: 0
                            gradientValueMinSource: QAccelPlot.GradientValueSource.Fixed
                            gradientValueMin: window.directionIndex === 0 ? -4 : -6
                            gradientValueMaxSource: QAccelPlot.GradientValueSource.Fixed
                            gradientValueMax: window.directionIndex === 0 ? window.solarVerticalMaximum : 6
                            opacity: window.gradientOpacity
                            colormap: activeColormap
                        },
                        QAccelPlot.GradientStroke {
                            direction: window.directionIndex === 0 ? QAccelPlot.GradientDirection.Vertical : QAccelPlot.GradientDirection.Horizontal
                            gradientValueMinSource: QAccelPlot.GradientValueSource.Fixed
                            gradientValueMin: window.directionIndex === 0 ? -4 : -6
                            gradientValueMaxSource: QAccelPlot.GradientValueSource.Fixed
                            gradientValueMax: window.directionIndex === 0 ? window.solarVerticalMaximum : 6
                            colormap: activeColormap
                        }
                    ]
                    Component.onCompleted: {
                        const points = [];
                        for (let hour = -6; hour <= 6; hour += 0.05)
                            points.push(Qt.point(hour, window.netGridPowerAt(hour)));
                        setData(points);
                    }
                }
            }

            QAccelPlot.Plot {
                id: batteryPlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: 1
                legendVisible: false
                grid.subGridVisible: false

                xAxis: ExampleAxis {
                    viewportMin: 0
                    viewportMax: 100
                    dataMin: 0
                    dataMax: 100
                    label: "State of charge (%)"
                }

                yAxis: ExampleAxis {
                    viewportMin: 20
                    viewportMax: 30
                    dataMin: 20
                    dataMax: 30
                    axisTitlePadding: 40
                    layoutSize: 60
                    label: "Battery temperature (°C)"
                }

                QAccelPlot.LineCurve {
                    xAxis: batteryPlot.xAxis
                    yAxis: batteryPlot.yAxis
                    color: colorPalette.text
                    lineWidth: 3
                    opacity: window.gradientOpacity
                    effects: [
                        QAccelPlot.GradientStroke {
                            direction: window.directionIndex === 0 ? QAccelPlot.GradientDirection.Vertical : QAccelPlot.GradientDirection.Horizontal
                            gradientValueMinSource: QAccelPlot.GradientValueSource.Fixed
                            gradientValueMin: window.directionIndex === 0 ? 20 : 0
                            gradientValueMaxSource: QAccelPlot.GradientValueSource.Fixed
                            gradientValueMax: window.directionIndex === 0 ? window.batteryVerticalMaximum : 100
                            colormap: activeColormap
                        }
                    ]
                    Component.onCompleted: {
                        const points = [];
                        for (let charge = 0; charge <= 100; charge += 1) {
                            const chargingHeat = 3.0 * Math.pow(charge / 100, 1.7);
                            const balancingPulse = 4.5 * Math.exp(-Math.pow((charge - 82) / 8, 2));
                            const ambientCycle = 0.6 * Math.sin(charge / 11);
                            points.push(Qt.point(charge, 22 + chargingHeat + balancingPulse + ambientCycle));
                        }
                        setData(points);
                    }
                }
            }
        }
    }
}
