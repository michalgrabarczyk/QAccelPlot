//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

// Crosshair lines at a PlotInspector's cursor, with optional value badges at the plot edges.
Item {
    id: root

    // Set automatically for a component declared inside a PlotInspector.
    property QAccelPlot.PlotInspector inspector
    property color color: QAccelPlot.Colors.dark.hover
    property real lineWidth: 1
    property bool horizontal: true
    property bool vertical: true
    // Shows the cursor's axis values at the bottom and left plot edges.
    property bool axisLabels: false
    property color labelColor: QAccelPlot.Colors.dark.textOnAccent
    property font font: Qt.font({
        pixelSize: 11
    })

    // A cursor set from code may have no Y; the horizontal line is hidden then.
    readonly property bool hasX: inspector !== null && !isNaN(inspector.position.x)
    readonly property bool hasY: inspector !== null && !isNaN(inspector.position.y)
    readonly property real cursorX: hasX ? inspector.position.x - x : 0
    readonly property real cursorY: hasY ? inspector.position.y - y : 0
    readonly property rect area: inspector && inspector.plot ? inspector.plot.plotRect : Qt.rect(0, 0, 0, 0)

    parent: inspector && inspector.plot ? inspector.plot.overlay : null
    visible: inspector !== null && inspector.active
    x: area.x
    y: area.y
    width: area.width
    height: area.height
    clip: true

    Rectangle {
        visible: root.vertical && root.hasX
        x: root.cursorX - width / 2
        width: root.lineWidth
        height: root.height
        color: root.color
    }

    Rectangle {
        visible: root.horizontal && root.hasY
        y: root.cursorY - height / 2
        width: root.width
        height: root.lineWidth
        color: root.color
    }

    Rectangle {
        visible: root.axisLabels && root.vertical && root.hasX && xLabel.text.length > 0
        x: Math.max(0, Math.min(root.cursorX - width / 2, root.width - width))
        y: root.height - height
        width: xLabel.width + 8
        height: xLabel.height + 4
        radius: 2
        color: root.color

        Text {
            id: xLabel

            anchors.centerIn: parent
            text: root.inspector ? root.inspector.cursorXText : ""
            font: root.font
            color: root.labelColor
        }
    }

    Rectangle {
        visible: root.axisLabels && root.horizontal && root.hasY && yLabel.text.length > 0
        y: Math.max(0, Math.min(root.cursorY - height / 2, root.height - height))
        width: yLabel.width + 8
        height: yLabel.height + 4
        radius: 2
        color: root.color

        Text {
            id: yLabel

            anchors.centerIn: parent
            text: root.inspector ? root.inspector.cursorYText : ""
            font: root.font
            color: root.labelColor
        }
    }
}
