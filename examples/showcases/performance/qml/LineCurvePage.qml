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
    objectName: "lineCurvePlot"

    required property QtObject settings

    readonly property var seriesColors: ["mediumaquamarine", "#ffb454", "#ff6b9d", "#7aa2f7"]

    legendVisible: false

    xAxis: ExampleAxis {
        viewportMin: 0
        viewportMax: 1000
        label: "Sample domain"
    }

    yAxis: ExampleAxis {
        viewportMin: -10
        viewportMax: 10
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
            lineWidth: plot.settings.lineWidth
            lineStyle: plot.settings.lineStyle === "dash" ? dashLine : plot.settings.lineStyle === "none" ? noLine : solidLine
            marker.shape: plot.settings.markerShape
            marker.size: plot.settings.markerSize
            effects: plot.settings.effect === "stroke" ? [gradientStroke] : plot.settings.effect === "fill" ? [gradientFill] : []
            antialiasingEnabled: plot.settings.antialiasing
            antialiasingFeather: plot.settings.antialiasingFeather
            gaps.nanMode: plot.settings.connectGaps ? QAccelPlot.NanGapMode.Connect : QAccelPlot.NanGapMode.Break

            QAccelPlot.SolidLine {
                id: solidLine
            }

            QAccelPlot.DashLine {
                id: dashLine
                pattern: [12, 6]
            }

            QAccelPlot.NoLine {
                id: noLine
            }

            // Fixed value ranges spare the effects a scan of the data per update.
            QAccelPlot.GradientStroke {
                id: gradientStroke
                direction: QAccelPlot.GradientDirection.Vertical
                gradientValueMinSource: QAccelPlot.GradientValueSource.Fixed
                gradientValueMin: -8
                gradientValueMaxSource: QAccelPlot.GradientValueSource.Fixed
                gradientValueMax: 8
                colormap: QAccelPlot.Colormap {
                    preset: QAccelPlot.Colormap.Plasma
                }
            }

            QAccelPlot.GradientFill {
                id: gradientFill
                direction: QAccelPlot.GradientDirection.Vertical
                baseline: QAccelPlot.GradientFillBaseline.Value
                baselineValue: 0
                gradientValueMinSource: QAccelPlot.GradientValueSource.Fixed
                gradientValueMin: -8
                gradientValueMaxSource: QAccelPlot.GradientValueSource.Fixed
                gradientValueMax: 8
                opacity: 0.4
                colormap: QAccelPlot.Colormap {
                    preset: QAccelPlot.Colormap.Plasma
                }
            }
        }
    }
}
