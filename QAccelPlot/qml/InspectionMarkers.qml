//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

// Dots marking the sample each series of a PlotInspector matched.
Item {
    id: root

    // Set automatically for a component declared inside a PlotInspector.
    property QAccelPlot.PlotInspector inspector
    // Marker diameter in logical pixels.
    property real size: 10
    property color borderColor: QAccelPlot.Colors.dark.plotArea
    property real borderWidth: 1.5
    readonly property rect area: inspector && inspector.plot ? inspector.plot.plotRect : Qt.rect(0, 0, 0, 0)

    parent: inspector && inspector.plot ? inspector.plot.overlay : null
    visible: inspector !== null && inspector.active
    x: area.x
    y: area.y
    width: area.width
    height: area.height
    clip: true

    Repeater {
        model: root.inspector ? root.inspector.model : null

        Rectangle {
            required property bool valid
            required property point pixelPosition
            required property color seriesColor

            visible: valid
            x: valid ? pixelPosition.x - root.x - width / 2 : 0
            y: valid ? pixelPosition.y - root.y - height / 2 : 0
            width: root.size
            height: root.size
            radius: root.size / 2
            color: seriesColor
            border.color: root.borderColor
            border.width: root.borderWidth
        }
    }
}
