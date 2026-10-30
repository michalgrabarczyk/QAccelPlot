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
    // Hourly electricity prices in ct/kWh, one data point at the start of each hour.
    readonly property var prices: [21.4, 19.8, 18.9, 18.2, 19.5, 24.1, 31.6, 35.2, 29.8, 26.3, 24.7, 27.9, 33.5]
    readonly property var modes: [
        { title: "None", mode: QAccelPlot.StepMode.None, text: "Straight segments between data points." },
        { title: "Before", mode: QAccelPlot.StepMode.Before, text: "Each value holds from the previous data point up to its own." },
        { title: "After", mode: QAccelPlot.StepMode.After, text: "Each value holds from its data point up to the next one." },
        { title: "Center", mode: QAccelPlot.StepMode.Center, text: "Each value holds between the midpoints to its neighbors." }
    ]

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: "The same hourly prices drawn with each stepMode. Markers show the data points; the fill and dash pattern follow the steps."
            color: root.colorPalette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            spacing: 12

            CheckBox {
                id: markersCheck
                text: "Markers"
                checked: true
            }
            CheckBox {
                id: fillCheck
                text: "Fill"
                checked: true
            }
            CheckBox {
                id: dashCheck
                text: "Dashed"
            }
        }

        GridLayout {
            columns: 2
            columnSpacing: 12
            rowSpacing: 8
            Layout.fillWidth: true
            Layout.fillHeight: true

            Repeater {
                model: root.modes

                ColumnLayout {
                    id: cell

                    required property var modelData

                    spacing: 2
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredWidth: 1
                    Layout.preferredHeight: 1

                    Label {
                        text: "<b>StepMode." + cell.modelData.title + "</b>  " + cell.modelData.text
                        color: root.colorPalette.text
                        textFormat: Text.StyledText
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    QAccelPlot.Plot {
                        id: plot
                        legendVisible: false
                        grid.subGridVisible: false
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        xAxis: ExampleAxis {
                            viewportMin: -0.5
                            viewportMax: 12.5
                            label: "Hour"
                        }

                        yAxis: ExampleAxis {
                            viewportMin: 10
                            viewportMax: 40
                            label: "ct/kWh"
                        }

                        QAccelPlot.LineCurve {
                            xAxis: plot.xAxis
                            yAxis: plot.yAxis
                            stepMode: cell.modelData.mode
                            color: root.colorPalette.seriesPrimary
                            lineWidth: 2
                            marker.shape: markersCheck.checked ? QAccelPlot.MarkerShape.Circle : QAccelPlot.MarkerShape.None
                            marker.size: 3.5
                            lineStyle: QAccelPlot.DashLine {
                                pattern: dashCheck.checked ? [8, 4] : []
                            }
                            effects: QAccelPlot.GradientFill {
                                enabled: fillCheck.checked
                                direction: Qt.Vertical
                                baseline: QAccelPlot.GradientFillBaseline.AxisMinimum
                                opacity: 0.35
                                gradient: Gradient {
                                    GradientStop { position: 0; color: root.colorPalette.seriesPrimary }
                                    GradientStop { position: 1; color: "transparent" }
                                }
                            }
                            Component.onCompleted: setData(root.prices.map((price, hour) => Qt.point(hour, price)))
                        }
                    }
                }
            }
        }
    }
}
