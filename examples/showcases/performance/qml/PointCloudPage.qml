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

    required property QtObject colorPalette
    required property int pointCount
    // Points overlap more as the count grows, so each gets fainter and the arms stay distinct.
    readonly property real pointOpacity: Math.max(0.035, Math.min(0.35, 350000 / pointCount))
    // Widens the X range with the plot, so the galaxy stays circular.
    readonly property real xHalfRange: 1.15 * (plotRect.height > 0 ? plotRect.width / plotRect.height : 1)

    legendVisible: false

    xAxis: ExampleAxis {
        viewportMin: -plot.xHalfRange
        viewportMax: plot.xHalfRange
        dataMin: -plot.xHalfRange
        dataMax: plot.xHalfRange
        label: "x"
    }

    yAxis: ExampleAxis {
        viewportMin: -1.15
        viewportMax: 1.15
        dataMin: -1.15
        dataMax: 1.15
        axisTitlePadding: 40
        layoutSize: 60
        label: "y"
    }

    QAccelPlot.PointCloud {
        objectName: "pointCloud"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        marker.shape: QAccelPlot.PointCloud.Circle
        marker.size: 2
        opacity: plot.pointOpacity
        // A fixed value range spares the cloud a scan of every value per update.
        // Warm core, blue arms, and violet rim; pink marks star-forming regions in the arms.
        colormap: QAccelPlot.Colormap {
            min: 0
            max: 1
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
    }
}
