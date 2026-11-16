//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
import QtQuick
import QAccelPlot as QAccelPlot

// A histogram of amplitudes whose bins take their colors from colormap, so it reads as a color bar.
QAccelPlot.Plot {
    id: histogramPlot

    required property var colorPalette
    required property QAccelPlot.Colormap colormap

    border.color: colorPalette.plotBorder
    border.width: 2
    legendVisible: false
    grid.subGridVisible: false

    xAxis: ExampleAxis {
        autoRescale: true
        label.text: "Cells in view"
        ticker.subtickCount: 1
    }

    yAxis: ExampleAxis {
        label.text: "Amplitude"
    }

    // Filled from C++ by VisibleHistogram.
    QAccelPlot.BarSeries {
        objectName: "amplitudeHistogram"
        xAxis: histogramPlot.xAxis
        yAxis: histogramPlot.yAxis
        orientation: Qt.Horizontal
        colorSource: QAccelPlot.BarSeries.Position
        colormap: histogramPlot.colormap
        hoverColor: colorPalette.hover
        updateTransition: QAccelPlot.MorphTransition {
            duration: 150
        }
    }

    // Reports the bin under the pointer: its edges and its count.
    QAccelPlot.PlotInspector {
        plot: histogramPlot

        QAccelPlot.InspectionTooltip {}
    }
}
