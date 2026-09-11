//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QtQuick.Layouts
import QAccelPlot as QAccelPlot

Window {
    id: window

    readonly property QtObject colorPalette: QAccelPlot.Colors.dark
    readonly property int ridgeCount: 80
    readonly property real ridgePitch: 6.0

    function ridgeColor(idx) {
        // Inverted Viridis colormap across the waterfall ridges:
        // Top / horizon: Radiant Yellow #fde725
        // Descending through Emerald Green, Ocean Teal, Cobalt Blue,
        // to Foreground / bottom (idx = 0): Luminous Deep Violet #611c82
        const viridisStops = [
            Qt.rgba(0.380, 0.110, 0.510, 1.0), // 0.00: #611c82 (Foreground: Luminous Violet)
            Qt.rgba(0.306, 0.208, 0.525, 1.0), // 0.12: #4e3586 (Purple)
            Qt.rgba(0.231, 0.341, 0.545, 1.0), // 0.25: #3b578b (Cobalt Blue)
            Qt.rgba(0.165, 0.471, 0.557, 1.0), // 0.37: #2a788e (Ocean Blue)
            Qt.rgba(0.129, 0.600, 0.549, 1.0), // 0.50: #21998c (Vivid Teal)
            Qt.rgba(0.208, 0.718, 0.478, 1.0), // 0.62: #35b779 (Emerald Green)
            Qt.rgba(0.420, 0.804, 0.345, 1.0), // 0.75: #6bcd58 (Light Green)
            Qt.rgba(0.706, 0.867, 0.169, 1.0), // 0.87: #b4de2c (Yellow-Green)
            Qt.rgba(0.992, 0.906, 0.145, 1.0)  // 1.00: #fde725 (Horizon / Top: Radiant Yellow)
        ];
        const norm = (idx / (ridgeCount - 1)) * (viridisStops.length - 1);
        const i0 = Math.floor(norm);
        const i1 = Math.min(i0 + 1, viridisStops.length - 1);
        const frac = norm - i0;
        const c0 = viridisStops[i0];
        const c1 = viridisStops[i1];
        return Qt.rgba(
            c0.r + (c1.r - c0.r) * frac,
            c0.g + (c1.g - c0.g) * frac,
            c0.b + (c1.b - c0.b) * frac,
            1.0
        );
    }

    width: 766
    // Keep CI capture dimensions exactly representable at QT_SCALE_FACTOR=0.5.
    height: 852
    visible: true
    title: "QAccelPlot Cosmic Pulsar (CP 1919) Showcase"
    color: colorPalette.window

    onWidthChanged: console.log("width: ", window.width)
    onHeightChanged: console.log("height: ", window.height)

    FrameDriver {
        running: true
        targetWindow: window
    }

    Gradient {
        id: fillGradient

        GradientStop {
            position: 0.0
            color: colorPalette.controlHover
        }
        GradientStop {
            position: 0.25
            color: colorPalette.control
        }
        GradientStop {
            position: 1.0
            color: colorPalette.plotArea
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 0

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "Cosmic Pulsar CP 1919"
            horizontalAlignment: Text.AlignHCenter
            color: colorPalette.text
            font.pixelSize: 22
            font.bold: true
            font.letterSpacing: 1.5
        }

        QAccelPlot.Plot {
            id: plot

            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 16
            legendVisible: false
            plotAreaColor: colorPalette.window
            axesAreaColor: colorPalette.window
            border.color: colorPalette.plotBorder
            border.width: 1
            grid.gridColor: colorPalette.grid
            grid.lineWidth: 1
            grid.subGridVisible: false

            xAxis: QAccelPlot.Axis {
                viewportMin: 200
                viewportMax: 800
                dataMin: 0
                dataMax: 1000
                baselineColor: colorPalette.axisLine
                baselineWidth: 1
                labelColor: colorPalette.axisLabel
                hoverColor: colorPalette.axisLine
                ticker.tickCount: 6
                ticker.subtickCount: 1
                ticker.tickColor: colorPalette.tick
                ticker.tickLabelColor: colorPalette.axisTickLabel
                ticker.subtickColor: colorPalette.subtick
            }

            yAxis: QAccelPlot.Axis {
                visible: false
                viewportMin: 0.0
                viewportMax: 530.0
                dataMin: 0.0
                dataMax: 560.0
                axisTitlePadding: 40
                layoutSize: 60
            }

            Component {
                id: ridgeComponent

                QAccelPlot.LineCurve {
                    id: curveItem
                    property int index: 0

                    xAxis: plot.xAxis
                    yAxis: plot.yAxis
                    lineWidth: 1.75
                    antialiasingEnabled: true
                    antialiasingFeather: 0.5
                }
            }

            Component.onCompleted: {
                for (let i = 0; i < window.ridgeCount; ++i) {
                    ridgeComponent.createObject(plot, {
                        "objectName": "ridge_" + i,
                        "index": i,
                        "z": window.ridgeCount - i,
                        "color": window.ridgeColor(i)
                    });
                }
            }
        }
    }
}
