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
    property int formatIndex: 0
    readonly property var unitFormats: ["%1 (%2)", "%1 [%2]", "%1 / %2"]

    // RC step response with a fast ringing transient right after the step.
    function voltageAt(seconds) {
        const charge = 1 - Math.exp(-seconds / 1e-3);
        const ringing = 0.15 * Math.exp(-seconds / 200e-9) * Math.sin(2 * Math.PI * 20e6 * seconds);
        return charge + ringing;
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "Values stay in seconds and volts. All tick labels share one unit prefix, shown in the axis label. Zoom into the first microsecond to see the ringing in ns."
                color: root.colorPalette.textSecondary
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                text: "Unit format"
                color: root.colorPalette.text
            }
            ComboBox {
                model: root.unitFormats
                currentIndex: root.formatIndex
                Layout.preferredWidth: 130
                Material.background: root.colorPalette.plotArea
                onActivated: root.formatIndex = currentIndex
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                viewportMin: 0
                viewportMax: 0.006
                label.text: "Time"
                label.unitFormat: root.unitFormats[root.formatIndex]
                ticker.tickLabelFormatter: QAccelPlot.UnitTickLabelFormatter {
                    unit: "s"
                }
            }

            yAxis: ExampleAxis {
                viewportMin: 0
                viewportMax: 1.2
                label.text: "Voltage"
                label.unitFormat: root.unitFormats[root.formatIndex]
                label.padding: 40
                layoutSize: 60
                ticker.tickLabelFormatter: QAccelPlot.UnitTickLabelFormatter {
                    unit: "V"
                }
            }

            QAccelPlot.LineCurve {
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.colorPalette.seriesPrimary
                lineWidth: 2

                Component.onCompleted: {
                    const points = [Qt.point(0, 0)];
                    const sampleCount = 6000;
                    for (let sample = 0; sample <= sampleCount; ++sample) {
                        const seconds = 1e-10 * Math.pow(1e8, sample / sampleCount);
                        points.push(Qt.point(seconds, root.voltageAt(seconds)));
                    }
                    setData(points);
                }
            }
        }
    }
}
