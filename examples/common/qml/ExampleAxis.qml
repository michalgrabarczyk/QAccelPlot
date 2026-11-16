//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
import QtQuick
import QAccelPlot as QAccelPlot

QAccelPlot.Axis {
    property QtObject colorPalette: QAccelPlot.Colors.dark

    baselineColor: colorPalette.axisLine
    hoverColor: colorPalette.hover
    label.color: colorPalette.axisLabel
    label.font: Qt.font({
        pixelSize: 13,
        weight: Font.Normal
    })
    ticker.tickColor: colorPalette.tick
    ticker.subtickColor: colorPalette.subtick
    ticker.tickLabelColor: colorPalette.axisTickLabel
    ticker.tickLabelFont: Qt.font({
        pixelSize: 12,
        weight: Font.Normal
    })
}
