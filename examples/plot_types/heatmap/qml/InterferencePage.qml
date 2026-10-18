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
        id: wavePlot
        anchors.fill: parent
        border.color: colorPalette.plotBorder
        border.width: 2
        legendVisible: false

        xAxis: ExampleAxis {
            viewportMin: -10
            viewportMax: 10
            label: "x (m)"
        }

        yAxis: ExampleAxis {
            viewportMin: -7.5
            viewportMax: 7.5
            label: "y (m)"
        }

        // Filled from C++ with setValues(), 400 columns by 300 rows.
        QAccelPlot.HeatmapSeries {
            id: waves
            objectName: "interferenceHeatmap"
            name: "Amplitude"
            xAxis: wavePlot.xAxis
            yAxis: wavePlot.yAxis
            xMin: -10
            xMax: 10
            yMin: -7.5
            yMax: 7.5
            colormap: QAccelPlot.Colormap {
                preset: root.preset
                reversed: root.reversed
            }
        }

        HeatmapColorBar {
            id: colorBar
            colorPalette: root.colorPalette
            plot: wavePlot
            heatmap: waves
            barLabel: "Amplitude"
        }

        // Reports the cell under the pointer: its center and its value.
        QAccelPlot.PlotInspector {
            plot: wavePlot
            enabled: !colorBar.barHovered

            QAccelPlot.InspectionTooltip {}
        }
    }
}
