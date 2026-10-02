//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

// Tooltip listing the matching series of a PlotInspector next to its cursor.
Rectangle {
    id: root

    required property QAccelPlot.PlotInspector inspector
    property color textColor: QAccelPlot.Colors.dark.text
    property font font: Qt.font({
        pixelSize: 12
    })
    // Distance in logical pixels between the cursor and the tooltip.
    property real offset: 12
    // One delegate per inspected series; its required properties receive the InspectionRowModel roles.
    property Component rowDelegate: Row {
        id: row

        required property bool valid
        required property string seriesName
        required property color seriesColor
        required property string xText
        required property string yText
        required property bool hasSummary
        required property int summaryCount
        required property string minimumText
        required property string maximumText

        // The column skips hidden rows, so series without a match take no space.
        visible: valid
        spacing: 6

        Rectangle {
            width: 8
            height: 8
            radius: 4
            color: row.seriesColor
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            font: root.font
            color: root.textColor
            text: {
                if (row.hasSummary && row.summaryCount > 1)
                    return row.seriesName + ": " + row.minimumText + " … " + row.maximumText + " (" + row.summaryCount + ")";
                return row.seriesName + ": " + row.xText + ", " + row.yText;
            }
        }
    }

    readonly property rect area: inspector.plot ? inspector.plot.plotRect : Qt.rect(0, 0, 0, 0)
    readonly property real anchorX: isNaN(inspector.position.x) ? area.x : inspector.position.x
    readonly property real anchorY: isNaN(inspector.position.y) ? area.y + area.height / 2 : inspector.position.y

    // Places the tooltip after the cursor, or before it when it would not fit inside the plot area.
    function place(anchor, size, start, length) {
        const after = anchor + offset;
        const position = after + size <= start + length ? after : anchor - offset - size;
        return Math.max(start, Math.min(position, start + length - size));
    }

    parent: inspector.plot
    z: 3
    visible: inspector.active && inspector.validCount > 0
    clip: true
    color: QAccelPlot.Colors.dark.legendBackground
    border.color: QAccelPlot.Colors.dark.legendBorder
    radius: 4
    width: Math.min(column.width + 16, area.width)
    height: Math.min(column.height + 12, area.height)
    x: place(anchorX, width, area.x, area.width)
    y: place(anchorY, height, area.y, area.height)

    Column {
        id: column

        x: 8
        y: 6
        spacing: 4

        Repeater {
            model: root.inspector.model
            delegate: root.rowDelegate
        }
    }
}
