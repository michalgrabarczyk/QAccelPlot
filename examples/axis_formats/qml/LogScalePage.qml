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
    property bool logarithmic: true

    QAccelPlot.LogTickLabelFormatter {
        id: logarithmicFormatter
    }
    QAccelPlot.NumericTickLabelFormatter {
        id: numericFormatter
    }

    function responseAt(frequency) {
        const logFrequency = Math.log(frequency) / Math.LN10;
        const bassLift = 2.2 * Math.exp(-Math.pow((logFrequency - Math.log(110) / Math.LN10) / 0.23, 2));
        const cabinetNotch = 4.8 * Math.exp(-Math.pow((logFrequency - Math.log(3200) / Math.LN10) / 0.13, 2));
        const highFrequencyRollOff = 10 * Math.log(1 + Math.pow(frequency / 15000, 4)) / Math.LN10;
        return -1.2 + bassLift - cabinetNotch - highFrequencyRollOff;
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "A logarithmic frequency axis gives each audio decade equal space. Toggle it to see why a linear scale hides low-frequency detail."
                color: root.palette.textSecondary
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Switch {
                text: "Logarithmic axis"
                checked: root.logarithmic
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onToggled: root.logarithmic = checked
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            plotAreaColor: root.palette.plotArea
            axesAreaColor: root.palette.axesArea
            grid.gridColor: root.palette.grid
            grid.subGridVisible: false

            xAxis: QAccelPlot.Axis {
                viewportMin: 20
                viewportMax: 20000
                dataMin: 20
                dataMax: 20000
                logScale: root.logarithmic
                side: QAccelPlot.Axis.Bottom
                label: "Frequency (Hz)"
                baselineColor: root.palette.axisLine
                labelColor: root.palette.axisLabel
                labelFont: root.typography.axisLabel
                hoverColor: root.palette.hover
                ticker.tickColor: root.palette.tick
                ticker.tickLabelColor: root.palette.axisTickLabel
                ticker.tickLabelFont: root.typography.axisTickLabel
                ticker.subtickColor: root.palette.subtick
                ticker.tickLabelFormatter: root.logarithmic ? logarithmicFormatter : numericFormatter
            }

            yAxis: QAccelPlot.Axis {
                viewportMin: -18
                viewportMax: 3
                dataMin: -18
                dataMax: 3
                side: QAccelPlot.Axis.Left
                label: "Relative level (dB)"
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
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.palette.seriesTertiary
                lineWidth: 3
                antialiasingEnabled: true

                Component.onCompleted: {
                    const points = [];
                    for (let sample = 0; sample <= 180; ++sample) {
                        const frequency = 20 * Math.pow(1000, sample / 180);
                        points.push(Qt.point(frequency, root.responseAt(frequency)));
                    }
                    setData(points);
                }
            }
        }
    }
}
