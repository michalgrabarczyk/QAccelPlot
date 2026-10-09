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
    objectName: "barSeriesPlot"

    required property QtObject settings
    readonly property var seriesColors: ["mediumaquamarine", "#ffb454", "#ff6b9d", "cornflowerblue"]
    // Bars narrower than a pixel overlap, so each gets fainter as the count grows and overlap adds up to brightness.
    readonly property real barOpacity: Math.max(0.02, Math.min(1, 1500 / settings.count))
    // One color per category, from blue for the lowest bars to yellow for the highest.
    readonly property var categoryColors: {
        const colors = [];
        for (let category = 0; category < settings.categoryCount; ++category) {
            colors.push(Qt.hsla(0.62 - 0.48 * category / Math.max(1, settings.categoryCount - 1), 0.85, 0.6, barOpacity));
        }
        return colors;
    }

    legendVisible: false

    // A bar's position is its index, so the range follows the bar count.
    xAxis: ExampleAxis {
        viewportMin: -0.5
        viewportMax: plot.settings.count - 0.5
        label: "Bar"
    }

    yAxis: ExampleAxis {
        viewportMin: 0
        viewportMax: 1
        axisTitlePadding: 40
        layoutSize: 60
        label: "Value"
    }

    // The series share the bars: each draws one run of them.
    Repeater {
        model: plot.settings.seriesCount

        delegate: QAccelPlot.BarSeries {
            id: series

            required property int index
            readonly property color barColor: plot.seriesColors[index % plot.seriesColors.length]

            xAxis: plot.xAxis
            yAxis: plot.yAxis
            color: Qt.rgba(barColor.r, barColor.g, barColor.b, plot.barOpacity)
            categoryColors: plot.categoryColors
            barWidth: plot.settings.barWidth
            border.width: plot.settings.borderWidth
            border.color: QAccelPlot.Colors.dark.plotArea
            minimumWidth: plot.settings.minimumWidth

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
