//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QtQuick.Layouts
import QAccelPlot as QAccelPlot

Item {
    id: root

    required property var colorPalette
    required property int preset
    required property bool reversed

    // Shared by the heatmap and the histogram. The histogram's amplitude axis sets its range, so
    // panning or zooming that axis recolors both.
    readonly property QAccelPlot.Colormap colormap: QAccelPlot.Colormap {
        preset: root.preset
        reversed: root.reversed
        min: histogram.yAxis.viewportMin
        max: histogram.yAxis.viewportMax
    }

    RowLayout {
        anchors.fill: parent
        spacing: 6

        QAccelPlot.Plot {
            id: wavePlot
            Layout.fillWidth: true
            Layout.fillHeight: true
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
                objectName: "interferenceHeatmap"
                name: "Amplitude"
                xAxis: wavePlot.xAxis
                yAxis: wavePlot.yAxis
                xMin: -10
                xMax: 10
                yMin: -7.5
                yMax: 7.5
                colormap: root.colormap
            }

            // Reports the cell under the pointer: its center and its value.
            QAccelPlot.PlotInspector {
                plot: wavePlot

                QAccelPlot.InspectionTooltip {}
            }
        }

        // Counts the cells in view of the heatmap, recounted from C++ as it pans and zooms.
        AmplitudeHistogram {
            id: histogram
            Layout.preferredWidth: 240
            Layout.fillHeight: true
            colorPalette: root.colorPalette
            colormap: root.colormap
        }
    }
}
