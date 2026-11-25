//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QAccelPlot as QAccelPlot
Item {
    id: root
    required property var colorPalette
    property bool logarithmic: true

    // Second-order band-pass response: high-pass corner at 100 Hz, resonant low-pass corner at 10 MHz.
    function gainAt(hertz) {
        const high = hertz / 100;
        const low = hertz / 10e6;
        const highPass = high * high / Math.sqrt(Math.pow(1 - high * high, 2) + Math.pow(high / 0.707, 2));
        const lowPass = 1 / Math.sqrt(Math.pow(1 - low * low, 2) + Math.pow(low / 2, 2));
        return 20 * Math.log(highPass * lowPass) / Math.LN10;
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "With prefixMode PerTick, each tick label picks its own prefix. The gain axis lists no allowed prefixes, so it stays in dB."
                color: root.colorPalette.textSecondary
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Switch {
                text: "Logarithmic axis"
                checked: root.logarithmic
                onToggled: root.logarithmic = checked
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                viewportMin: 1
                viewportMax: 1e9
                logScale: root.logarithmic
                label.text: "Frequency"
                ticker.tickLabelFormatter: QAccelPlot.UnitTickLabelFormatter {
                    unit: "Hz"
                    prefixMode: QAccelPlot.UnitTickLabelFormatter.PerTick
                }
            }

            yAxis: ExampleAxis {
                viewportMin: -90
                viewportMax: 15
                label.text: "Gain"
                label.padding: 40
                layoutSize: 60
                ticker.tickLabelFormatter: QAccelPlot.UnitTickLabelFormatter {
                    unit: "dB"
                    allowedPrefixes: []
                }
            }

            QAccelPlot.LineCurve {
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.colorPalette.seriesTertiary
                lineWidth: 3

                Component.onCompleted: {
                    const points = [];
                    const sampleCount = 2000;
                    for (let sample = 0; sample <= sampleCount; ++sample) {
                        const hertz = Math.pow(1e9, sample / sampleCount);
                        points.push(Qt.point(hertz, root.gainAt(hertz)));
                    }
                    setData(points);
                }
            }
        }
    }
}
