//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QAccelPlot as QAccelPlot
Item {
    id: root
    required property var palette
    required property var typography
    readonly property int sampleCount: 4000

    function ecg(phase) {
        return 0.12 * Math.exp(-Math.pow((phase - 0.18) / 0.035, 2)) - 0.16 * Math.exp(-Math.pow((phase - 0.37) / 0.012, 2)) + Math.exp(-Math.pow((phase - 0.40) / 0.010, 2)) - 0.28 * Math.exp(-Math.pow((phase - 0.43) / 0.014, 2)) + 0.30 * Math.exp(-Math.pow((phase - 0.68) / 0.070, 2));
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        Label {
            text: "Three panels sharing one time axis"
            color: root.palette.text
            font.bold: true
            font.pixelSize: 14
        }

        Label {
            text: "The lower panel owns the time axis. Pan or zoom it to update both ECG leads and the respiration trace together."
            color: root.palette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            Layout.bottomMargin: 4
        }

        QAccelPlot.Plot {
            id: leadOnePlot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            plotAreaColor: root.palette.plotArea
            axesAreaColor: root.palette.axesArea
            grid.gridColor: root.palette.grid
            grid.subGridVisible: false

            yAxis: QAccelPlot.Axis {
                viewportMin: -0.5
                viewportMax: 1.2
                dataMin: -0.5
                dataMax: 1.2
                side: QAccelPlot.Axis.Left
                label: "Lead I (mV)"
                baselineColor: root.palette.axisLine
                hoverColor: root.palette.hover
                labelColor: root.palette.axisLabel
                labelFont: root.typography.axisLabel
                ticker.tickColor: root.palette.tick
                ticker.tickLabelColor: root.palette.axisTickLabel
                ticker.tickLabelFont: root.typography.axisTickLabel
                ticker.subtickColor: root.palette.subtick
                ticker.tickCount: 4
            }

            QAccelPlot.LineCurve {
                xAxis: respirationPlot.xAxis
                yAxis: leadOnePlot.yAxis
                color: root.palette.seriesTertiary
                lineWidth: 2
                antialiasingEnabled: true
                Component.onCompleted: {
                    const points = [];
                    for (let i = 0; i <= root.sampleCount; ++i) {
                        const t = i * 10 / root.sampleCount;
                        points.push(Qt.point(t, root.ecg((t % 1.0)) + 0.02 * Math.sin(2 * Math.PI * 0.2 * t)));
                    }
                    setData(points);
                }
            }
        }

        QAccelPlot.Plot {
            id: leadTwoPlot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            plotAreaColor: root.palette.plotArea
            axesAreaColor: root.palette.axesArea
            grid.gridColor: root.palette.grid
            grid.subGridVisible: false

            yAxis: QAccelPlot.Axis {
                viewportMin: -0.7
                viewportMax: 1.5
                dataMin: -0.7
                dataMax: 1.5
                side: QAccelPlot.Axis.Left
                label: "Lead II (mV)"
                baselineColor: root.palette.axisLine
                hoverColor: root.palette.hover
                labelColor: root.palette.axisLabel
                labelFont: root.typography.axisLabel
                ticker.tickColor: root.palette.tick
                ticker.tickLabelColor: root.palette.axisTickLabel
                ticker.tickLabelFont: root.typography.axisTickLabel
                ticker.subtickColor: root.palette.subtick
                ticker.tickCount: 4
            }

            QAccelPlot.LineCurve {
                xAxis: respirationPlot.xAxis
                yAxis: leadTwoPlot.yAxis
                color: root.palette.seriesRose
                lineWidth: 2
                antialiasingEnabled: true
                Component.onCompleted: {
                    const points = [];
                    for (let i = 0; i <= root.sampleCount; ++i) {
                        const t = i * 10 / root.sampleCount;
                        points.push(Qt.point(t, 1.22 * root.ecg((t % 1.0) + 0.015) - 0.03 * Math.sin(2 * Math.PI * 0.1 * t)));
                    }
                    setData(points);
                }
            }
        }

        QAccelPlot.Plot {
            id: respirationPlot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            plotAreaColor: root.palette.plotArea
            axesAreaColor: root.palette.axesArea
            grid.gridColor: root.palette.grid
            grid.subGridVisible: false

            xAxis: QAccelPlot.Axis {
                viewportMin: 0
                viewportMax: 10
                dataMin: 0
                dataMax: 10
                side: QAccelPlot.Axis.Bottom
                label: "Shared time (s) — pan or zoom here"
                baselineColor: root.palette.axisLine
                baselineWidth: 2
                labelColor: root.palette.axisLabel
                labelFont: root.typography.axisLabel
                hoverColor: root.palette.hover
                ticker.tickColor: root.palette.tick
                ticker.tickLabelColor: root.palette.axisTickLabel
                ticker.tickLabelFont: root.typography.axisTickLabel
                ticker.subtickColor: root.palette.subtick
            }

            yAxis: QAccelPlot.Axis {
                viewportMin: -1
                viewportMax: 1
                dataMin: -1
                dataMax: 1
                side: QAccelPlot.Axis.Left
                label: "Respiration"
                baselineColor: root.palette.axisLine
                hoverColor: root.palette.hover
                labelColor: root.palette.axisLabel
                labelFont: root.typography.axisLabel
                ticker.tickColor: root.palette.tick
                ticker.tickLabelColor: root.palette.axisTickLabel
                ticker.tickLabelFont: root.typography.axisTickLabel
                ticker.subtickColor: root.palette.subtick
                ticker.tickCount: 4
            }

            QAccelPlot.LineCurve {
                xAxis: respirationPlot.xAxis
                yAxis: respirationPlot.yAxis
                color: root.palette.seriesCyan
                lineWidth: 2
                antialiasingEnabled: true
                Component.onCompleted: {
                    const points = [];
                    for (let i = 0; i <= root.sampleCount; ++i) {
                        const t = i * 10 / root.sampleCount;
                        points.push(Qt.point(t, 0.82 * Math.sin(2 * Math.PI * 0.25 * t)));
                    }
                    setData(points);
                }
            }
        }
    }
}
