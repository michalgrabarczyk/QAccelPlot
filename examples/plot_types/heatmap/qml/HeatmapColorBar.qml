//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

// Covers the plot area of plot with a color bar for heatmap in its top-right corner.
Item {
    id: overlay

    required property var colorPalette
    required property QAccelPlot.Plot plot
    required property QAccelPlot.HeatmapSeries heatmap
    required property string barLabel
    readonly property bool barHovered: colorBar.hovered

    x: plot.plotRect.x
    y: plot.plotRect.y
    width: plot.plotRect.width
    height: plot.plotRect.height
    clip: true

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 10
        width: colorBar.width + 20
        height: colorBar.height + 16
        radius: 4
        color: colorPalette.legendBackground
        border.color: colorPalette.legendBorder

        QAccelPlot.ColorBar {
            id: colorBar
            x: 10
            y: 8
            height: 260
            barThickness: 24
            tickLabelWidth: 30
            series: overlay.heatmap
            interactive: true
            label: overlay.barLabel
            labelColor: colorPalette.axisLabel
            border.color: hovered ? colorPalette.hover : colorPalette.axisLine
            ticker.tickColor: colorPalette.tick
            ticker.tickLabelColor: colorPalette.axisTickLabel
            ticker.tickCount: 8
        }
    }
}
