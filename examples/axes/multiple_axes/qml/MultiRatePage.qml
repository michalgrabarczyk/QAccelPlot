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

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: "Independent time scales in one plot"
            color: root.palette.text
            font.bold: true
            font.pixelSize: 14
        }

        Label {
            text: "The settling response uses the primary 0–10 s axis. The damped chirp uses the extra 0–0.5 s axis, yet both curves share the same plot area and amplitude axis."
            color: root.palette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            grid.gridVisible: false
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                viewportMin: 0
                viewportMax: 10
                dataMin: 0
                dataMax: 10
                label: "Slow channel time (s)"
                baselineColor: root.palette.seriesPrimary
                baselineWidth: 2
                ticker.tickColor: root.palette.seriesPrimary
            }

            yAxis: ExampleAxis {
                viewportMin: -1.8
                viewportMax: 1.8
                dataMin: -1.8
                dataMax: 1.8
                axisTitlePadding: 40
                layoutSize: 60
                label: "Amplitude"
                baselineWidth: 2
            }

            extraAxes: [
                ExampleAxis {
                    id: fastAxis
                    viewportMin: 0
                    viewportMax: 0.5
                    dataMin: 0
                    dataMax: 0.5
                    orientation: QAccelPlot.Axis.Horizontal
                    side: QAccelPlot.Axis.Bottom
                    label: "Fast channel time (s)"
                    baselineColor: root.palette.seriesSecondary
                    baselineWidth: 2
                    ticker.tickColor: root.palette.seriesSecondary
                    ticker.tickCount: 6
                }
            ]

            QAccelPlot.LineCurve {
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                name: "Slow settling response"
                color: root.palette.seriesPrimary
                lineWidth: 3
                antialiasingEnabled: true

                Component.onCompleted: {
                    const points = [];
                    for (let i = 0; i <= 300; ++i) {
                        const t = i * 10 / 300;
                        const value = 0.85 * (1 - Math.exp(-t / 1.4)) + 0.20 * Math.exp(-t / 4) * Math.sin(2 * Math.PI * 0.7 * t);
                        points.push(Qt.point(t, value));
                    }
                    setData(points);
                }
            }

            QAccelPlot.LineCurve {
                xAxis: fastAxis
                yAxis: plot.yAxis
                name: "Fast damped chirp"
                color: root.palette.seriesSecondary
                lineWidth: 3
                antialiasingEnabled: true

                Component.onCompleted: {
                    const points = [];
                    for (let i = 0; i <= 300; ++i) {
                        const t = i * 0.5 / 300;
                        const value = 1.7 * Math.exp(-5 * t) * Math.sin(2 * Math.PI * (8 * t + 35 * t * t));
                        points.push(Qt.point(t, value));
                    }
                    setData(points);
                }
            }
        }
    }
}
