//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
import QtQuick
import QAccelPlot as QAccelPlot

Window {
    width: 400
    height: 300

    QAccelPlot.Plot {
        id: plot
        // The window is never shown, so its content item has no size to fill.
        width: 400
        height: 300
        // The legend would draw the curve color without the curve shaders.
        legendVisible: false

        xAxis: QAccelPlot.Axis {
            viewportMin: 0
            viewportMax: 10
        }

        yAxis: QAccelPlot.Axis {
            viewportMin: 0
            viewportMax: 10
        }

        QAccelPlot.LineCurve {
            objectName: "curve"
            xAxis: plot.xAxis
            yAxis: plot.yAxis
            color: "magenta"
            lineWidth: 6
        }
    }
}
