//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot
// RectangleTool: a resizable data-rectangle with 4 corner handles, draggable
// body, and an area label.

Item {
    id: root

    required property var palette
    anchors.fill: parent
    opacity: active ? 1.0 : 0.78

    // Tool activation state (controls handle visibility)
    property bool active: false

    // Rectangle corners in data coordinates
    property real x1Data: 0.0
    property real y1Data: 0.0
    property real x2Data: 1.0
    property real y2Data: 1.0

    // Style
    property color borderColor: palette.toolRegion
    property color fillColor: palette.toolOverlay
    property real borderWidth: 2.0
    property real handleRadius: 6.0

    // Read-only: area in data units (absolute)
    readonly property real dataArea: Math.abs((x2Data - x1Data) * (y2Data - y1Data))

    // Pixel corners (updated when data or plot transform changes)
    property real px1: 0
    property real py1: 0
    property real px2: 0
    property real py2: 0

    // Normalised pixel rect (derived from pixel corners)
    readonly property real rectX: Math.min(px1, px2)
    readonly property real rectY: Math.min(py1, py2)
    readonly property real rectW: Math.abs(px2 - px1)
    readonly property real rectH: Math.abs(py2 - py1)

    // Recompute on data-coordinate changes
    onX1DataChanged: updatePixels()
    onY1DataChanged: updatePixels()
    onX2DataChanged: updatePixels()
    onY2DataChanged: updatePixels()
    onActiveChanged: {
        if (active) {
            window.activateTool(root);
        }
    }
    Component.onCompleted: updatePixels()

    // Recompute when axis range changes (pan / zoom)
    Connections {
        target: root.parent ? root.parent.xAxis : null
        function onRangeChanged() {
            root.updatePixels();
        }
    }
    Connections {
        target: root.parent ? root.parent.yAxis : null
        function onRangeChanged() {
            root.updatePixels();
        }
    }
    // Recompute when plot area is resized or layout changes
    Connections {
        target: root.parent
        function onPlotRectChanged() {
            root.updatePixels();
        }
    }

    function updatePixels() {
        if (!parent) {
            return;
        }
        px1 = parent.dataToPixelX(x1Data);
        py1 = parent.dataToPixelY(y1Data);
        px2 = parent.dataToPixelX(x2Data);
        py2 = parent.dataToPixelY(y2Data);
    }

    // Filled rectangle
    Rectangle {
        x: root.rectX
        y: root.rectY
        width: root.rectW
        height: root.rectH
        color: root.fillColor
        border.color: root.borderColor
        border.width: root.borderWidth

        // Drag the whole rectangle
        MouseArea {
            id: bodyArea
            anchors.fill: parent
            cursorShape: Qt.SizeAllCursor

            property real startMouseX: 0
            property real startMouseY: 0
            property real startX1: 0
            property real startY1: 0
            property real startX2: 0
            property real startY2: 0

            onPressed: mouse => {
                root.active = true;
                const pt = mapToItem(root, mouse.x, mouse.y);
                startMouseX = pt.x;
                startMouseY = pt.y;
                startX1 = root.x1Data;
                startY1 = root.y1Data;
                startX2 = root.x2Data;
                startY2 = root.y2Data;
            }
            onPositionChanged: mouse => {
                if (!pressed) {
                    return;
                }
                // The body moves while dragging, so MouseArea-local coordinates
                // would make the delta jump. Use RectangleTool-local coordinates.
                const pt = mapToItem(root, mouse.x, mouse.y);
                const dxPx = pt.x - startMouseX;
                const dyPx = pt.y - startMouseY;

                // Convert pixel delta to data delta
                const newPx1 = root.parent.dataToPixelX(startX1) + dxPx;
                const newPy1 = root.parent.dataToPixelY(startY1) + dyPx;
                const newPx2 = root.parent.dataToPixelX(startX2) + dxPx;
                const newPy2 = root.parent.dataToPixelY(startY2) + dyPx;
                root.x1Data = root.parent.pixelToDataX(newPx1);
                root.y1Data = root.parent.pixelToDataY(newPy1);
                root.x2Data = root.parent.pixelToDataX(newPx2);
                root.y2Data = root.parent.pixelToDataY(newPy2);
            }
        }
    }

    // Area label at center
    Rectangle {
        readonly property real cx: (root.px1 + root.px2) / 2
        readonly property real cy: (root.py1 + root.py2) / 2

        x: cx - width / 2
        y: cy - height / 2
        width: areaLabel.implicitWidth + 10
        height: areaLabel.implicitHeight + 6
        radius: 3
        color: root.palette.tooltipBackground

        Text {
            id: areaLabel
            anchors.centerIn: parent
            font.pixelSize: 11
            font.bold: true
            color: root.palette.tooltipText
            text: "A = " + root.dataArea.toFixed(2)
        }
    }

    // Corner resize handles
    component CornerHandle: Rectangle {
        id: ch

        property real pixelX: 0
        property real pixelY: 0

        signal dragged(real newDataX, real newDataY)

        x: pixelX - root.handleRadius
        y: pixelY - root.handleRadius
        width: root.handleRadius * 2
        height: root.handleRadius * 2
        radius: root.handleRadius
        color: chArea.pressed ? Qt.lighter(root.borderColor, 1.5) : root.borderColor
        border.color: root.palette.handleBorder
        border.width: 2
        visible: root.active

        MouseArea {
            id: chArea
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeFDiagCursor

            onPressed: root.active = true
            onPositionChanged: mouse => {
                if (pressed) {
                    const pt = mapToItem(root, mouse.x, mouse.y);
                    ch.dragged(root.parent.pixelToDataX(pt.x), root.parent.pixelToDataY(pt.y));
                }
            }
        }
    }

    // Top-left (x1, y1)
    CornerHandle {
        pixelX: root.px1
        pixelY: root.py1
        onDragged: (dx, dy) => {
            root.x1Data = dx;
            root.y1Data = dy;
        }
    }

    // Top-right (x2, y1)
    CornerHandle {
        pixelX: root.px2
        pixelY: root.py1
        onDragged: (dx, dy) => {
            root.x2Data = dx;
            root.y1Data = dy;
        }
    }

    // Bottom-left (x1, y2)
    CornerHandle {
        pixelX: root.px1
        pixelY: root.py2
        onDragged: (dx, dy) => {
            root.x1Data = dx;
            root.y2Data = dy;
        }
    }

    // Bottom-right (x2, y2)
    CornerHandle {
        pixelX: root.px2
        pixelY: root.py2
        onDragged: (dx, dy) => {
            root.x2Data = dx;
            root.y2Data = dy;
        }
    }
}
