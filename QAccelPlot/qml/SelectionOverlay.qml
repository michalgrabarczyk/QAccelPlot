//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

// Rectangle showing the gesture or the selected region of a SelectionTool.
Item {
    id: root

    required property QAccelPlot.SelectionTool tool
    property color color: tool.fillColor
    property color borderColor: tool.borderColor

    parent: tool.plot
    z: 2
    visible: tool.selecting || tool.hasSelection
    x: tool.plot ? tool.plot.plotRect.x : 0
    y: tool.plot ? tool.plot.plotRect.y : 0
    width: tool.plot ? tool.plot.plotRect.width : 0
    height: tool.plot ? tool.plot.plotRect.height : 0
    clip: true

    Rectangle {
        x: root.tool.pixelRect.x - root.x
        y: root.tool.pixelRect.y - root.y
        width: root.tool.pixelRect.width
        height: root.tool.pixelRect.height
        color: root.color
        border.color: root.borderColor
    }
}
