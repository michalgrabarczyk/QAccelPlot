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
    property int presetIndex: 2
    property int directionIndex: 0
    property int baselineIndex: 0
    property real gradientOpacity: 0.75
    property real verticalMaximumPercent: 100.0

    readonly property var presetNames: ["Polar", "Neon", "Viridis", "Spectrum"]
    readonly property var activeGradient: qAccelPlotGradients[presetIndex]
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

    Gradient {
        id: polarGradient
        GradientStop {
            position: 0.0
            color: "#08306b"
        }
        GradientStop {
            position: 0.25
            color: "#4292c6"
        }
        GradientStop {
            position: 0.55
            color: "#f7fbff"
        }
        GradientStop {
            position: 0.78
            color: "#ef8a62"
        }
        GradientStop {
            position: 1.0
            color: "#b2182b"
        }
    }

    Gradient {
        id: neonGradient
        GradientStop {
            position: 0.0
            color: "#17002d"
        }
        GradientStop {
            position: 0.25
            color: "#5926ff"
        }
        GradientStop {
            position: 0.55
            color: "#e02bff"
        }
        GradientStop {
            position: 0.78
            color: "#ff3b81"
        }
        GradientStop {
            position: 1.0
            color: "#ffe66d"
        }
    }

    Gradient {
        id: viridisGradient
        GradientStop {
            position: 0.0
            color: "#440154"
        }
        GradientStop {
            position: 0.25
            color: "#3b528b"
        }
        GradientStop {
            position: 0.55
            color: "#21918c"
        }
        GradientStop {
            position: 0.78
            color: "#5ec962"
        }
        GradientStop {
            position: 1.0
            color: "#fde725"
        }
    }

    Gradient {
        id: spectrumGradient
        GradientStop {
            position: 0.0
            color: "#6a00ff"
        }
        GradientStop {
            position: 0.25
            color: "#00a7ff"
        }
        GradientStop {
            position: 0.55
            color: "#00e5a8"
        }
        GradientStop {
            position: 0.78
            color: "#ffe600"
        }
        GradientStop {
            position: 1.0
            color: "#ff3d71"
        }
    }

    readonly property var qAccelPlotGradients: [polarGradient, neonGradient, viridisGradient, spectrumGradient]

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Label {
            text: "A home imports power below 0 kW and exports surplus solar power above it. Compare filling from the chart minimum with filling from the meaningful 0 kW baseline."
            color: root.palette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "Preset"
                color: root.palette.text
            }
            ComboBox {
                model: root.presetNames
                currentIndex: root.presetIndex
                Layout.preferredWidth: 175
                Material.background: root.palette.plotArea
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onActivated: root.presetIndex = currentIndex
            }
            Label {
                text: "Direction"
                color: root.palette.text
            }
            ComboBox {
                model: ["Vertical", "Horizontal"]
                currentIndex: root.directionIndex
                Layout.preferredWidth: 120
                Material.background: root.palette.plotArea
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onActivated: root.directionIndex = currentIndex
            }
            Label {
                text: "Baseline"
                color: root.palette.text
            }
            ComboBox {
                model: ["Axis minimum", "Grid balance (0 kW)"]
                currentIndex: root.baselineIndex
                Layout.preferredWidth: 175
                Material.background: root.palette.plotArea
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onActivated: root.baselineIndex = currentIndex
            }
            Item {
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "Fill opacity " + Math.round(root.gradientOpacity * 100) + "%"
                color: root.palette.text
            }
            Slider {
                from: 0.1
                to: 1.0
                stepSize: 0.05
                value: root.gradientOpacity
                Layout.preferredWidth: 190
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onMoved: root.gradientOpacity = value
            }
            Label {
                text: "Vertical maximum " + root.verticalMaximumPercent.toFixed(0) + "% of Y-axis range"
                color: root.palette.text
                opacity: root.directionIndex === 0 ? 1.0 : 0.45
            }
            Slider {
                from: 1.0
                to: 100.0
                stepSize: 1.0
                value: root.verticalMaximumPercent
                enabled: root.directionIndex === 0
                Layout.preferredWidth: 190
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onMoved: root.verticalMaximumPercent = value
            }
            Item {
                Layout.fillWidth: true
            }
        }

        Label {
            text: "The lower chart shows battery temperature during a charge cycle. Its gradient stroke uses the same preset, direction, opacity, and normalized vertical maximum as the solar plot, with a heat spike during cell balancing."
            color: root.palette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
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
                plotAreaColor: root.palette.plotArea
                axesAreaColor: root.palette.axesArea
                grid.gridColor: root.palette.grid
                grid.subGridVisible: false

                xAxis: QAccelPlot.Axis {
                    viewportMin: -6
                    viewportMax: 6
                    dataMin: -6
                    dataMax: 6
                    side: QAccelPlot.Axis.Bottom
                    label: "Hours from solar noon"
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
                    viewportMin: -4
                    viewportMax: 6
                    dataMin: -4
                    dataMax: 6
                    side: QAccelPlot.Axis.Left
                    axisTitlePadding: 40
                    layoutSize: 60
                    label: "Net grid power (kW)"
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
                    xAxis: solarPlot.xAxis
                    yAxis: solarPlot.yAxis
                    color: root.palette.text
                    lineWidth: 3
                    antialiasingEnabled: true
                    effects: [
                        QAccelPlot.GradientFill {
                            direction: root.directionIndex === 0 ? QAccelPlot.GradientDirection.Vertical : QAccelPlot.GradientDirection.Horizontal
                            baseline: root.baselineIndex === 0 ? QAccelPlot.GradientFillBaseline.AxisMinimum : QAccelPlot.GradientFillBaseline.Value
                            baselineValue: 0
                            gradientValueMinSource: QAccelPlot.GradientValueSource.Fixed
                            gradientValueMin: root.directionIndex === 0 ? -4 : -6
                            gradientValueMaxSource: QAccelPlot.GradientValueSource.Fixed
                            gradientValueMax: root.directionIndex === 0 ? root.solarVerticalMaximum : 6
                            opacity: root.gradientOpacity
                            gradient: root.activeGradient
                        },
                        QAccelPlot.GradientStroke {
                            direction: root.directionIndex === 0 ? QAccelPlot.GradientDirection.Vertical : QAccelPlot.GradientDirection.Horizontal
                            gradientValueMinSource: QAccelPlot.GradientValueSource.Fixed
                            gradientValueMin: root.directionIndex === 0 ? -4 : -6
                            gradientValueMaxSource: QAccelPlot.GradientValueSource.Fixed
                            gradientValueMax: root.directionIndex === 0 ? root.solarVerticalMaximum : 6
                            gradient: root.activeGradient
                        }
                    ]
                    Component.onCompleted: {
                        const points = [];
                        for (let hour = -6; hour <= 6; hour += 0.05)
                            points.push(Qt.point(hour, root.netGridPowerAt(hour)));
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
                plotAreaColor: root.palette.plotArea
                axesAreaColor: root.palette.axesArea
                grid.gridColor: root.palette.grid
                grid.subGridVisible: false

                xAxis: QAccelPlot.Axis {
                    viewportMin: 0
                    viewportMax: 100
                    dataMin: 0
                    dataMax: 100
                    side: QAccelPlot.Axis.Bottom
                    label: "State of charge (%)"
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
                    viewportMin: 20
                    viewportMax: 30
                    dataMin: 20
                    dataMax: 30
                    side: QAccelPlot.Axis.Left
                    axisTitlePadding: 40
                    layoutSize: 60
                    label: "Battery temperature (°C)"
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
                    xAxis: batteryPlot.xAxis
                    yAxis: batteryPlot.yAxis
                    color: root.palette.text
                    lineWidth: 3
                    opacity: root.gradientOpacity
                    antialiasingEnabled: true
                    effects: [
                        QAccelPlot.GradientStroke {
                            direction: root.directionIndex === 0 ? QAccelPlot.GradientDirection.Vertical : QAccelPlot.GradientDirection.Horizontal
                            gradientValueMinSource: QAccelPlot.GradientValueSource.Fixed
                            gradientValueMin: root.directionIndex === 0 ? 20 : 0
                            gradientValueMaxSource: QAccelPlot.GradientValueSource.Fixed
                            gradientValueMax: root.directionIndex === 0 ? root.batteryVerticalMaximum : 100
                            gradient: root.activeGradient
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
