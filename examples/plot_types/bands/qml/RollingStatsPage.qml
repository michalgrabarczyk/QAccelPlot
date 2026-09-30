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
    // The band at the cursor's X, or an empty object outside the band's samples.
    readonly property var cursorBand: hover.hovered ? band.valueAt(plot.pixelToDataX(hover.point.position.x)) : ({})

    function withAlpha(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha);
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: "Ten minutes of a vibration sensor, 10,000 samples, with a 10 s rolling mean and a mean ± 2σ band computed in C++. Pan and zoom to see the detail."
            color: root.palette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Switch {
                id: rawSwitch
                text: "Raw samples"
            }
            Switch {
                id: edgesSwitch
                text: "Band edges"
            }
            Item {
                Layout.fillWidth: true
            }
            Label {
                text: root.cursorBand.low === undefined ? "" : "t = " + root.cursorBand.x.toFixed(2) + " s   mean ± 2σ: " + root.cursorBand.low.toFixed(3) + " to " + root.cursorBand.high.toFixed(3) + " g"
                color: band.hovered ? root.palette.text : root.palette.textSecondary
                font.family: "monospace"
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                viewportMin: 0
                viewportMax: 600
                dataMin: 0
                dataMax: 600
                label: "Time (s)"
            }

            yAxis: ExampleAxis {
                viewportMin: -2.5
                viewportMax: 4
                dataMin: -2.5
                dataMax: 4
                axisTitlePadding: 40
                layoutSize: 60
                label: "Acceleration (g)"
            }

            // The lines give up hover so that the band beneath them receives it.
            QAccelPlot.LineCurve {
                objectName: "rawSamples"
                name: "Samples"
                visible: rawSwitch.checked
                hoverRadius: 0
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.withAlpha(root.palette.seriesMuted, 0.35)
            }

            QAccelPlot.BandSeries {
                id: band
                objectName: "rollingBand"
                name: "Mean ± 2σ"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.withAlpha(root.palette.seriesSecondary, 0.35)
                edges.width: edgesSwitch.checked ? 1 : 0
            }

            QAccelPlot.LineCurve {
                objectName: "rollingMean"
                name: "Rolling mean"
                hoverRadius: 0
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.palette.seriesSecondary
                lineWidth: 1.5
            }

            HoverHandler {
                id: hover
            }
        }
    }
}
