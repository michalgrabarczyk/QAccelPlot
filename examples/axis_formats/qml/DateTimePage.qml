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
    property int formatIndex: 0
    readonly property var formats: ["hh:mm:ss", "mm:ss", "hh:mm:ss.zzz"]
    readonly property real traceStart: new Date(2026, 8, 10, 9, 30, 0, 0).getTime()
    readonly property real traceEnd: traceStart + 60000

    function responseTimeAt(seconds) {
        const backgroundVariation = 5 * Math.sin(seconds * 0.42) + 2.5 * Math.sin(seconds * 1.7);
        const cacheMissBurst = 48 * Math.exp(-Math.pow((seconds - 18) / 2.2, 2));
        const deploymentBurst = 34 * Math.exp(-Math.pow((seconds - 44) / 3.1, 2));
        return 34 + backgroundVariation + cacheMissBurst + deploymentBurst;
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "Raw X values are milliseconds since the Unix epoch. DateTimeTickLabelFormatter turns them into readable clock labels."
                color: root.palette.textSecondary
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                text: "Label format"
                color: root.palette.text
            }
            ComboBox {
                model: ["Hours, minutes, seconds", "Minutes and seconds", "With milliseconds"]
                currentIndex: root.formatIndex
                Layout.preferredWidth: 190
                Material.background: root.palette.plotArea
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onActivated: root.formatIndex = currentIndex
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
                viewportMin: root.traceStart
                viewportMax: root.traceEnd
                dataMin: root.traceStart
                dataMax: root.traceEnd
                label: "Trace time"
                baselineColor: root.palette.axisLine
                labelColor: root.palette.axisLabel
                labelFont: root.typography.axisLabel
                hoverColor: root.palette.hover
                ticker.tickCount: 6
                ticker.tickColor: root.palette.tick
                ticker.tickLabelColor: root.palette.axisTickLabel
                ticker.tickLabelFont: root.typography.axisTickLabel
                ticker.subtickColor: root.palette.subtick
                ticker.tickLabelFormatter: QAccelPlot.DateTimeTickLabelFormatter {
                    dateTimeFormat: root.formats[root.formatIndex]
                }
            }

            yAxis: QAccelPlot.Axis {
                viewportMin: 20
                viewportMax: 95
                dataMin: 20
                dataMax: 95
                axisTitlePadding: 40
                layoutSize: 60
                label: "Response time (ms)"
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
                color: root.palette.seriesPrimary
                lineWidth: 3
                antialiasingEnabled: true

                Component.onCompleted: {
                    const points = [];
                    for (let sample = 0; sample <= 120; ++sample) {
                        const seconds = sample * 0.5;
                        points.push(Qt.point(root.traceStart + seconds * 1000, root.responseTimeAt(seconds)));
                    }
                    setData(points);
                }
            }
        }
    }
}
