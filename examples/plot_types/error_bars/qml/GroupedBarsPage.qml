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
    readonly property var weeks: [0, 1, 2, 3]
    readonly property var treatmentNames: ["Control", "Low dose", "High dose"]
    readonly property var treatmentColors: [colorPalette.seriesSecondary, colorPalette.seriesPrimary, colorPalette.seriesTertiary]
    // Mean plant height in cm and its standard deviation, per treatment and week.
    readonly property var means: [[12.1, 15.4, 18.2, 20.5], [12.4, 17.9, 23.0, 27.6], [11.8, 19.6, 27.1, 33.8]]
    readonly property var deviations: [[1.1, 1.4, 1.9, 2.2], [1.3, 1.8, 2.6, 3.1], [1.2, 2.4, 3.3, 4.6]]
    readonly property bool horizontal: horizontalCheck.checked
    readonly property real barWidth: 0.8 / treatmentNames.length

    // Shift of treatment `index` within its week, shared by its bars and error bars.
    function offsetOf(index) {
        return (index - (treatmentNames.length - 1) / 2) * barWidth;
    }

    QAccelPlot.TextTickLabelFormatter {
        id: weekLabels
        labels: ["Week 1", "Week 2", "Week 3", "Week 4"]
    }

    QAccelPlot.NumericTickLabelFormatter {
        id: heightLabels
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: "Mean plant height per week under three treatments, ± one standard deviation. Each ErrorBarSeries shares its offset with the BarSeries of its treatment. Error bars always take (x, y) data points, so turning the bars horizontal sets their data again with the weeks as y."
            color: root.colorPalette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            spacing: 12

            CheckBox {
                id: horizontalCheck
                text: "Horizontal"
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            grid.subGridVisible: false

            // The week axis is X for vertical bars and Y for horizontal ones.
            xAxis: ExampleAxis {
                viewportMin: root.horizontal ? 0 : -0.5
                viewportMax: root.horizontal ? 50 : 3.5
                label: root.horizontal ? "Height (cm)" : ""
                ticker.tickCount: root.horizontal ? 11 : 4
                ticker.subtickCount: 0
                ticker.tickLabelFormatter: root.horizontal ? heightLabels : weekLabels
            }

            yAxis: ExampleAxis {
                viewportMin: root.horizontal ? -0.5 : 0
                viewportMax: root.horizontal ? 3.5 : 50
                labelPadding: 40
                layoutSize: 70
                label: root.horizontal ? "" : "Height (cm)"
                ticker.tickCount: root.horizontal ? 4 : 11
                ticker.subtickCount: 0
                ticker.tickLabelFormatter: root.horizontal ? weekLabels : heightLabels
            }

            Repeater {
                model: root.treatmentNames

                QAccelPlot.BarSeries {
                    required property int index
                    required property string modelData

                    name: modelData
                    xAxis: plot.xAxis
                    yAxis: plot.yAxis
                    orientation: root.horizontal ? Qt.Horizontal : Qt.Vertical
                    color: root.treatmentColors[index]
                    barWidth: root.barWidth
                    barOffset: root.offsetOf(index)
                    Component.onCompleted: setData(root.means[index])
                }
            }

            Repeater {
                model: root.treatmentNames

                QAccelPlot.ErrorBarSeries {
                    id: errorBars

                    required property int index

                    function updateData() {
                        const values = root.means[index];
                        if (orientation === Qt.Horizontal) {
                            setErrorData(values, root.weeks, root.deviations[index]);
                        } else {
                            setErrorData(root.weeks, values, root.deviations[index]);
                        }
                    }

                    xAxis: plot.xAxis
                    yAxis: plot.yAxis
                    orientation: root.horizontal ? Qt.Horizontal : Qt.Vertical
                    offset: root.offsetOf(index)
                    color: root.colorPalette.text
                    capLength: 10
                    onOrientationChanged: updateData()
                    Component.onCompleted: updateData()
                }
            }
        }
    }
}
