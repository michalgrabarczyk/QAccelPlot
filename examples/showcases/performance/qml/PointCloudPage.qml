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
    objectName: "pointCloudPlot"

    required property QtObject settings
    // Points overlap more as the count grows, so each gets fainter and the arms stay distinct.
    readonly property real pointOpacity: Math.max(0.035, Math.min(0.35, 350000 / settings.count))
    // Widens the X range with the plot, so the galaxy stays circular.
    readonly property real xHalfRange: 1.15 * (plotRect.height > 0 ? plotRect.width / plotRect.height : 1)

    readonly property var seriesColors: ["#b9c8ff", "#ffb454", "#ff6b9d", "mediumaquamarine"]

    legendVisible: false

    xAxis: ExampleAxis {
        viewportMin: -plot.xHalfRange
        viewportMax: plot.xHalfRange
        label: "x"
    }

    yAxis: ExampleAxis {
        viewportMin: -1.15
        viewportMax: 1.15
        axisTitlePadding: 40
        layoutSize: 60
        label: "y"
    }

    // Warm core, blue arms, and violet rim; pink marks star-forming regions in the arms.
    QAccelPlot.Colormap {
        id: galaxyColormap

        // A fixed value range spares the clouds a scan of every value per update.
        min: plot.settings.autoColorRange ? NaN : 0
        max: plot.settings.autoColorRange ? NaN : 1
        stops: [
            GradientStop {
                position: 0.0
                color: "#fff8e7"
            },
            GradientStop {
                position: 0.12
                color: "#ffd27f"
            },
            GradientStop {
                position: 0.3
                color: "#ff8fb1"
            },
            GradientStop {
                position: 0.45
                color: "#b9c8ff"
            },
            GradientStop {
                position: 0.75
                color: "#5b6cff"
            },
            GradientStop {
                position: 1.0
                color: "#3a1f7a"
            }
        ]
    }

    // The clouds share the galaxy: each draws one ring of it.
    Repeater {
        model: plot.settings.seriesCount

        delegate: QAccelPlot.PointCloud {
            required property int index

            xAxis: plot.xAxis
            yAxis: plot.yAxis
            // Used when the points carry no values.
            color: plot.seriesColors[index % plot.seriesColors.length]
            marker.shape: plot.settings.markerShape
            marker.size: plot.settings.markerSize
            marker.filled: plot.settings.markerFilled
            marker.strokeWidth: plot.settings.markerStrokeWidth
            antialiasingEnabled: plot.settings.antialiasing
            opacity: plot.pointOpacity
            colormap: galaxyColormap
        }
    }
}
