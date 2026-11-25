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
    property int prefixIndex: 0
    // undefined restores the default: every power-of-1000 prefix and the bare unit.
    readonly property var prefixChoices: [undefined, ["m", "µ"], []]

    // Gaussian laser beam profile with fine interference fringes.
    function powerAt(meters) {
        const beam = Math.exp(-meters * meters / (2 * 0.5e-3 * 0.5e-3));
        const fringes = 1 + 0.08 * Math.cos(2 * Math.PI * meters / 25e-6);
        return 2.5e-3 * beam * fringes;
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "With unitPosition TickLabels, each tick label carries the shared prefixed unit. Zoom into a fringe to see positions in µm."
                color: root.colorPalette.textSecondary
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                text: "Allowed prefixes"
                color: root.colorPalette.text
            }
            ComboBox {
                model: ["All powers of 1000", "Milli and micro", "None"]
                currentIndex: root.prefixIndex
                Layout.preferredWidth: 190
                Material.background: root.colorPalette.plotArea
                onActivated: root.prefixIndex = currentIndex
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                viewportMin: -0.002
                viewportMax: 0.002
                label.text: "Position"
                ticker.tickLabelFormatter: QAccelPlot.UnitTickLabelFormatter {
                    unit: "m"
                    unitPosition: QAccelPlot.UnitTickLabelFormatter.TickLabels
                    allowedPrefixes: root.prefixChoices[root.prefixIndex]
                }
            }

            yAxis: ExampleAxis {
                viewportMin: 0
                viewportMax: 0.003
                label.text: "Power"
                label.padding: 70
                layoutSize: 90
                ticker.tickLabelFormatter: QAccelPlot.UnitTickLabelFormatter {
                    unit: "W"
                    unitPosition: QAccelPlot.UnitTickLabelFormatter.TickLabels
                    allowedPrefixes: root.prefixChoices[root.prefixIndex]
                }
            }

            QAccelPlot.LineCurve {
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.colorPalette.seriesSecondary
                lineWidth: 2

                Component.onCompleted: {
                    const points = [];
                    const sampleCount = 8000;
                    for (let sample = 0; sample <= sampleCount; ++sample) {
                        const meters = -2.5e-3 + 5e-3 * sample / sampleCount;
                        points.push(Qt.point(meters, root.powerAt(meters)));
                    }
                    setData(points);
                }
            }
        }
    }
}
