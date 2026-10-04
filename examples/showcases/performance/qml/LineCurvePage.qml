//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

// Filled from C++ with setDataFNoRange(), so the axes keep the data range set here.
QAccelPlot.Plot {
    id: plot
    objectName: "lineCurvePlot"

    required property QtObject settings

    readonly property var seriesColors: ["mediumaquamarine", "#ffb454", "#ff6b9d", "#7aa2f7"]

    legendVisible: false

    xAxis: ExampleAxis {
        viewportMin: 0
        viewportMax: 1000
        dataMin: 0
        dataMax: 1000
        label: "Sample domain"
    }

    yAxis: ExampleAxis {
        viewportMin: -10
        viewportMax: 10
        dataMin: -10
        dataMax: 10
        axisTitlePadding: 40
        layoutSize: 60
        label: "Amplitude"
    }

    // The curves share the wave: each draws one stretch of it.
    Repeater {
        model: plot.settings.seriesCount

        delegate: QAccelPlot.LineCurve {
            required property int index

            xAxis: plot.xAxis
            yAxis: plot.yAxis
            color: plot.seriesColors[index % plot.seriesColors.length]
            lineWidth: 3
        }
    }
}
