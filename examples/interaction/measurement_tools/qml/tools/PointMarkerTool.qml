//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot
// PointMarkerTool: a single draggable data-coordinate marker that shows its
// x / y values in a label.

Item {
    id: root

    required property var palette
    anchors.fill: parent
    opacity: active ? 1.0 : 0.78

    // Tool activation state (controls handle visibility)
    property bool active: false

    // Data coordinates
    property real dataX: 0.0
    property real dataY: 0.0

    // Style
    property color markerColor: palette.toolPoint
    property real markerRadius: 7.0

    // Pixel positions (updated when data or plot transform changes)
    property real px: 0
    property real py: 0

    // Recompute on data-coordinate changes
    onDataXChanged: updatePositions()
    onDataYChanged: updatePositions()
    onWidthChanged: updatePositions()
    onHeightChanged: updatePositions()
    onActiveChanged: {
        if (active) {
            window.activateTool(root);
        }
    }
    Component.onCompleted: updatePositions()

    // Recompute when axis range changes (pan / zoom)
    Connections {
        target: root.parent ? root.parent.xAxis : null
        function onRangeChanged() {
            root.updatePositions();
        }
    }
    Connections {
        target: root.parent ? root.parent.yAxis : null
        function onRangeChanged() {
            root.updatePositions();
        }
    }
    // Recompute when plot area is resized or layout changes
    Connections {
        target: root.parent
        function onPlotRectChanged() {
            root.updatePositions();
        }
    }

    function updatePositions() {
        if (!parent) {
            return;
        }
        px = parent.dataToPixelX(dataX);
        py = parent.dataToPixelY(dataY);
    }

    // Crosshair — vertical line
    Rectangle {
        x: root.px - 0.5
        y: 0
        width: 1
        height: root.height
        color: Qt.rgba(root.markerColor.r, root.markerColor.g, root.markerColor.b, 0.20)
    }

    // Crosshair — horizontal line
    Rectangle {
        x: 0
        y: root.py - 0.5
        width: root.width
        height: 1
        color: Qt.rgba(root.markerColor.r, root.markerColor.g, root.markerColor.b, 0.20)
    }

    // Marker dot
    Rectangle {
        x: root.px - root.markerRadius
        y: root.py - root.markerRadius
        width: root.markerRadius * 2
        height: root.markerRadius * 2
        radius: root.markerRadius
        color: markerArea.pressed ? Qt.lighter(root.markerColor, 1.6) : root.markerColor
        border.color: root.palette.handleBorder
        border.width: 2

        MouseArea {
            id: markerArea
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeAllCursor

            onPressed: root.active = true
            onPositionChanged: mouse => {
                if (pressed) {
                    const pt = mapToItem(root, mouse.x, mouse.y);
                    root.dataX = root.parent.pixelToDataX(pt.x);
                    root.dataY = root.parent.pixelToDataY(pt.y);
                }
            }
        }
    }

    // Label
    Rectangle {
        x: root.px + root.markerRadius + 6
        y: root.py - height - 2
        width: coordLabel.implicitWidth + 10
        height: coordLabel.implicitHeight + 6
        radius: 3
        color: root.palette.tooltipBackground

        Text {
            id: coordLabel
            anchors.centerIn: parent
            font.pixelSize: 11
            font.bold: true
            color: root.palette.tooltipText
            text: "(" + root.dataX.toFixed(2) + ", " + root.dataY.toFixed(2) + ")"
        }
    }
}
