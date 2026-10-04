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
    objectName: "rectangleSeriesPlot"

    required property QtObject settings
    readonly property var seriesColors: ["cornflowerblue", "#ffb454", "#ff6b9d", "mediumaquamarine"]
    // Tiles overlap more as the count grows, so each gets fainter and overlap adds up to brightness.
    readonly property real tileOpacity: Math.max(0.015, Math.min(0.7, 100000 / settings.count))
    // One color per category, from blue for the troughs of the plasma field to yellow for its crests.
    readonly property var categoryColors: {
        const colors = [];
        for (let category = 0; category < settings.categoryCount; ++category) {
            colors.push(Qt.hsla(0.62 - 0.48 * category / Math.max(1, settings.categoryCount - 1), 0.85, 0.6, tileOpacity));
        }
        return colors;
    }

    legendVisible: false

    xAxis: ExampleAxis {
        viewportMin: 0
        viewportMax: 160
        dataMin: 0
        dataMax: 160
        label: "x"
    }

    yAxis: ExampleAxis {
        viewportMin: 0
        viewportMax: 100
        dataMin: 0
        dataMax: 100
        axisTitlePadding: 40
        layoutSize: 60
        label: "y"
    }

    // The series share the tile grid: each draws a run of its rows.
    Repeater {
        model: plot.settings.seriesCount

        delegate: QAccelPlot.RectangleSeries {
            id: series

            required property int index
            readonly property color tileColor: plot.seriesColors[index % plot.seriesColors.length]

            xAxis: plot.xAxis
            yAxis: plot.yAxis
            color: Qt.rgba(tileColor.r, tileColor.g, tileColor.b, plot.tileOpacity)
            categoryColors: plot.categoryColors
            border.width: plot.settings.borderWidth
            border.color: QAccelPlot.Colors.dark.plotArea
            minimumWidth: plot.settings.minimumSize
            minimumHeight: plot.settings.minimumSize

            // Without the binding the hover color stays invalid, which draws no highlight.
            Binding {
                target: series
                property: "hoverColor"
                value: QAccelPlot.Colors.dark.text
                when: plot.settings.hoverHighlight
            }
        }
    }
}
