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
    readonly property real slope: 2.1
    readonly property real intercept: 0.4
    // Reference concentrations in mg/L and their uncertainties.
    readonly property var references: Array.from({ length: 12 }, (_, i) => 0.5 * (i + 1))
    readonly property var referenceErrors: references.map(x => 0.04 + 0.02 * x)
    // Sensor readings in mV around a linear response. The sensor saturates, so readings err more
    // above the value than below it.
    readonly property var readings: references.map(x => slope * x + intercept + 0.35 * Math.sin(5.3 * x))
    readonly property var errorsBelow: references.map(x => 0.2 + 0.05 * x)
    readonly property var errorsAbove: references.map(x => 0.3 + 0.12 * x)
    // The measurement under the cursor: the point, or one of its error bars.
    readonly property int hoveredIndex: points.hoveredIndex >= 0 ? points.hoveredIndex : yErrors.hoveredIndex >= 0 ? yErrors.hoveredIndex : xErrors.hoveredIndex

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: "Calibration of a sensor against reference solutions. One ErrorBarSeries shows the asymmetric reading errors, another the reference errors along X. The PointCloud is declared last, so its markers are drawn and hovered on top. Hover a point or an error bar to read the measurement."
            color: root.colorPalette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            spacing: 12

            CheckBox {
                id: referenceErrorsCheck
                text: "Reference errors"
                checked: true
            }

            Label {
                text: "Cap length " + capLengthSlider.value.toFixed(0) + " px"
                color: root.colorPalette.text
            }
            Slider {
                id: capLengthSlider
                from: 0
                to: 20
                stepSize: 1
                value: 8
                Layout.preferredWidth: 140
            }

            Label {
                text: "Line width " + lineWidthSlider.value.toFixed(1) + " px"
                color: root.colorPalette.text
            }
            Slider {
                id: lineWidthSlider
                from: 0.5
                to: 4
                value: 1.5
                Layout.preferredWidth: 140
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                viewportMin: 0
                viewportMax: 6.5
                label: "Reference concentration (mg/L)"
            }

            yAxis: ExampleAxis {
                viewportMin: 0
                viewportMax: 18
                labelPadding: 40
                layoutSize: 60
                label: "Sensor reading (mV)"
            }

            QAccelPlot.LineCurve {
                name: "Linear fit"
                hoverEnabled: false
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.colorPalette.seriesSecondary
                lineWidth: 1.5
                lineStyle: QAccelPlot.DashLine {
                    pattern: [6, 4]
                }
                Component.onCompleted: setData([Qt.point(0, root.intercept), Qt.point(6.5, root.slope * 6.5 + root.intercept)])
            }

            QAccelPlot.ErrorBarSeries {
                id: xErrors
                visible: referenceErrorsCheck.checked
                orientation: Qt.Horizontal
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.colorPalette.textSecondary
                hoverColor: root.colorPalette.text
                lineWidth: lineWidthSlider.value
                capLength: capLengthSlider.value
                Component.onCompleted: setErrorData(root.references, root.readings, root.referenceErrors)
            }

            QAccelPlot.ErrorBarSeries {
                id: yErrors
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.colorPalette.seriesPrimary
                hoverColor: root.colorPalette.text
                lineWidth: lineWidthSlider.value
                capLength: capLengthSlider.value
                Component.onCompleted: setErrorData(root.references, root.readings, root.errorsBelow, root.errorsAbove)
            }

            QAccelPlot.PointCloud {
                id: points
                name: "Measurements"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.colorPalette.seriesPrimary
                marker.size: 4
                Component.onCompleted: setData(root.references.map((x, i) => Qt.point(x, root.readings[i])))
            }

            Item {
                id: overlay
                x: plot.plotRect.x
                y: plot.plotRect.y
                width: plot.plotRect.width
                height: plot.plotRect.height
                clip: true

                QAccelPlot.DataAnchor {
                    id: tooltipAnchor

                    readonly property int index: root.hoveredIndex

                    visible: index >= 0
                    xAxis: plot.xAxis
                    yAxis: plot.yAxis
                    plotRect: Qt.rect(0, 0, overlay.width, overlay.height)
                    dataX1: index >= 0 ? root.references[index] : 0
                    dataX2: dataX1
                    dataY1: index >= 0 ? root.readings[index] + root.errorsAbove[index] : 0
                    dataY2: dataY1

                    Rectangle {
                        x: -width / 2
                        y: -height - 12
                        width: tooltipText.implicitWidth + 12
                        height: tooltipText.implicitHeight + 8
                        radius: 3
                        color: root.colorPalette.tooltipBackground

                        Text {
                            id: tooltipText
                            anchors.centerIn: parent
                            color: root.colorPalette.tooltipText
                            font.pixelSize: 11
                            text: {
                                const i = tooltipAnchor.index;
                                if (i < 0) {
                                    return "";
                                }
                                return root.references[i].toFixed(2) + " ± " + root.referenceErrors[i].toFixed(2) + " mg/L:  " + root.readings[i].toFixed(2) + " +"
                                    + root.errorsAbove[i].toFixed(2) + " / −" + root.errorsBelow[i].toFixed(2) + " mV";
                            }
                        }
                    }
                }
            }
        }
    }
}
