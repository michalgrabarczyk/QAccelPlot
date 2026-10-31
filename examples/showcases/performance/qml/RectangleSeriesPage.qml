//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQml.Models
import QtQuick
import QAccelPlot as QAccelPlot

QAccelPlot.Plot {
    id: plot
    objectName: "rectangleSeriesPlot"

    required property QtObject settings
    readonly property var seriesColors: ["cornflowerblue", "#ffb454", "#ff6b9d", "mediumaquamarine"]
    // Tiles overlap more as the count grows, so each gets fainter and overlap adds up to brightness.
    readonly property real tileOpacity: Math.max(0.015, Math.min(0.7, 100000 / settings.count))
    readonly property var categories: Array.from({ length: levels.count }, (_, index) => levels.objectAt(index))

    legendVisible: false

    // One category per level, from blue for the troughs of the plasma field to yellow for its crests.
    Instantiator {
        id: levels
        model: plot.settings.categoryCount

        QAccelPlot.Category {
            required property int index
            value: index
            color: Qt.hsla(0.62 - 0.48 * index / Math.max(1, plot.settings.categoryCount - 1), 0.85, 0.6, plot.tileOpacity)
        }
    }

    xAxis: ExampleAxis {
        viewportMin: 0
        viewportMax: 160
        label: "x"
    }

    yAxis: ExampleAxis {
        viewportMin: 0
        viewportMax: 100
        labelPadding: 40
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
            categories: plot.categories
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
