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
    required property var colorPalette

    readonly property var alignments: [QAccelPlot.Axis.Start, QAccelPlot.Axis.Center, QAccelPlot.Axis.End]

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Label {
                text: "Alignment"
                color: root.colorPalette.textSecondary
            }
            ComboBox {
                id: alignmentBox
                model: ["Start", "Center", "End"]
                currentIndex: 1
            }
            CheckBox {
                id: autoRotationBox
                text: "Auto rotation"
                checked: true
            }
            Label {
                text: "Rotation " + rotationSlider.value + "°"
                color: root.colorPalette.textSecondary
                enabled: !autoRotationBox.checked
            }
            Slider {
                id: rotationSlider
                from: -90
                to: 90
                stepSize: 15
                value: 0
                enabled: !autoRotationBox.checked
            }
            Label {
                text: "Padding " + paddingSlider.value
                color: root.colorPalette.textSecondary
            }
            Slider {
                id: paddingSlider
                from: 0
                to: 60
                stepSize: 2
                value: 40
            }
            Item {
                Layout.fillWidth: true
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                id: xAxis
                viewportMin: 0
                viewportMax: 10
                label.text: "Time (s)"
                label.padding: paddingSlider.value
                label.alignment: root.alignments[alignmentBox.currentIndex]
                // undefined restores the side's default rotation.
                label.rotation: autoRotationBox.checked ? undefined : rotationSlider.value
            }

            yAxis: ExampleAxis {
                id: yAxis
                viewportMin: -1.2
                viewportMax: 1.2
                label.text: "Amplitude (mV)"
                label.padding: paddingSlider.value
                label.alignment: root.alignments[alignmentBox.currentIndex]
                label.rotation: autoRotationBox.checked ? undefined : rotationSlider.value
                layoutSize: 60
            }

            QAccelPlot.LineCurve {
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.colorPalette.seriesPrimary
                lineWidth: 2

                Component.onCompleted: {
                    const points = [];
                    for (let sample = 0; sample <= 400; ++sample) {
                        const t = sample / 40;
                        points.push(Qt.point(t, Math.exp(-0.25 * t) * Math.sin(2 * Math.PI * 0.8 * t)));
                    }
                    setData(points);
                }
            }
        }
    }
}
