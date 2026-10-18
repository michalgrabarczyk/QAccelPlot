//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

QAccelPlot.Plot {
    id: plot
    objectName: "heatmapSeriesPlot"

    required property QtObject settings
    // Rows and columns of the square grid; matches SpectrogramGenerator::gridSide().
    readonly property int side: Math.max(1, Math.round(Math.sqrt(settings.count)))
    // The series whose value range the color bar shows.
    property QAccelPlot.HeatmapSeries keySeries: null

    legendVisible: false

    xAxis: ExampleAxis {
        viewportMin: 0
        viewportMax: plot.side
        label: "frequency"
    }

    yAxis: ExampleAxis {
        viewportMin: 0
        viewportMax: plot.side
        labelPadding: 40
        layoutSize: 60
        label: "time"
    }

    QAccelPlot.Colormap {
        id: spectrumColormap

        // A fixed value range spares the heatmaps a scan of every value per update.
        min: plot.settings.autoColorRange ? NaN : 0
        max: plot.settings.autoColorRange ? NaN : 1
        preset: QAccelPlot.Colormap.Inferno
    }

    // The heatmaps share the spectrogram: each draws a band of its rows.
    Repeater {
        model: plot.settings.seriesCount

        delegate: QAccelPlot.HeatmapSeries {
            xAxis: plot.xAxis
            yAxis: plot.yAxis
            colormap: spectrumColormap
        }

        onItemAdded: (index, item) => {
            if (index === 0) {
                plot.keySeries = item;
            }
        }
    }

    Rectangle {
        x: plot.plotRect.right - width - 10
        y: plot.plotRect.y + 10
        width: colorBar.width + 20
        height: colorBar.height + 16
        radius: 4
        color: QAccelPlot.Colors.dark.legendBackground
        border.color: QAccelPlot.Colors.dark.legendBorder

        QAccelPlot.ColorBar {
            id: colorBar
            x: 10
            y: 8
            height: 200
            barThickness: 18
            tickLabelWidth: 30
            series: plot.keySeries
            // Writes the shared colormap's range until "Auto color range" is next toggled.
            interactive: true
            label: "level"
            labelColor: QAccelPlot.Colors.dark.axisLabel
            borderColor: hovered ? QAccelPlot.Colors.dark.hover : QAccelPlot.Colors.dark.axisLine
            ticker.tickColor: QAccelPlot.Colors.dark.tick
            ticker.tickLabelColor: QAccelPlot.Colors.dark.axisTickLabel
        }
    }
}
