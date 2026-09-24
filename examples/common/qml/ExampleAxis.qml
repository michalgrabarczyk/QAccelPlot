//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

QAccelPlot.Axis {
    property QtObject colorPalette: QAccelPlot.Colors.dark

    baselineColor: colorPalette.axisLine
    hoverColor: colorPalette.hover
    labelColor: colorPalette.axisLabel
    labelFont: Qt.font({
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
