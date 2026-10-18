//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

Item {
    id: root

    required property var colorPalette
    required property int preset
    required property bool reversed

    QAccelPlot.Plot {
        id: spectrumPlot
        anchors.fill: parent
        border.color: colorPalette.plotBorder
        border.width: 2
        legendVisible: false

        xAxis: ExampleAxis {
            viewportMin: 0
            viewportMax: 10
            label: "Time (s)"
        }

        yAxis: ExampleAxis {
            viewportMin: 20
            viewportMax: 20000
            logScale: true
            label: "Frequency (Hz)"
        }

        // Filled from C++ with setValues(): 60 rows per decade, each spanning the same frequency ratio.
        QAccelPlot.HeatmapSeries {
            id: spectrum
            objectName: "logBinsHeatmap"
            name: "Level (dB)"
            xAxis: spectrumPlot.xAxis
            yAxis: spectrumPlot.yAxis
            xMin: 0
            xMax: 10
            yMin: 20
            yMax: 20000
            ySpacing: QAccelPlot.HeatmapSeries.Log
            colormap: QAccelPlot.Colormap {
                preset: root.preset
                reversed: root.reversed
                min: -80
                max: 0
            }
        }

        HeatmapColorBar {
            id: colorBar
            colorPalette: root.colorPalette
            plot: spectrumPlot
            heatmap: spectrum
            barLabel: "Level (dB)"
        }

        // Reports the cell under the pointer: its center and its value.
        QAccelPlot.PlotInspector {
            plot: spectrumPlot
            enabled: !colorBar.barHovered

            QAccelPlot.InspectionTooltip {}
        }
    }
}
