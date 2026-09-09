//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot
// RulerTool: measures the straight-line distance between two draggable handles.
// The ruler line itself can also be dragged to translate the entire tool.
// Drag handles are only visible when the tool is active.
//
// Usage — add as a child of Plot:
//
//   QAccelPlot.Plot {
//       id: myPlot
//       RulerTool { x1Data: 100; y1Data: 0; x2Data: 500; y2Data: 5 }
//   }

Item {
    id: root

    required property var palette
    anchors.fill: parent
    opacity: active ? 1.0 : 0.78

    // Tool activation state (controls handle visibility)
    property bool active: false

    // Handle positions in data coordinates
    property real x1Data: 0.0
    property real y1Data: 0.0
    property real x2Data: 1.0
    property real y2Data: 0.0

    // Style
    property color lineColor: palette.toolRuler
    property color handleColor: palette.toolRuler
    property real lineWidth: 2.0
    property real handleRadius: 6.0

    // Read-only: computed Euclidean distance in data units
    readonly property real dataDistance: {
        const dx = x2Data - x1Data;
        const dy = y2Data - y1Data;
        return Math.sqrt(dx * dx + dy * dy);
    }

    // Pixel positions (updated when data or plot transform changes)
    property real px1: 0
    property real py1: 0
    property real px2: 0
    property real py2: 0

    // Midpoint distance label
    onX1DataChanged: updatePixelPositions()
    onY1DataChanged: updatePixelPositions()
    onX2DataChanged: updatePixelPositions()
    onY2DataChanged: updatePixelPositions()
    onActiveChanged: {
        if (active) {
            window.activateTool(root);
        }
    }
    Component.onCompleted: updatePixelPositions()

    Connections {
        target: root.parent ? root.parent.xAxis : null
        function onRangeChanged() {
            root.updatePixelPositions();
        }
    }
    Connections {
        target: root.parent ? root.parent.yAxis : null
        function onRangeChanged() {
            root.updatePixelPositions();
        }
    }
    Connections {
        target: root.parent
        function onPlotRectChanged() {
            root.updatePixelPositions();
        }
    }

    function updatePixelPositions() {
        if (!parent) {
            return;
        }
        px1 = parent.dataToPixelX(x1Data);
        py1 = parent.dataToPixelY(y1Data);
        px2 = parent.dataToPixelX(x2Data);
        py2 = parent.dataToPixelY(y2Data);
    }

    // Ruler line rendered as a rotated rectangle
    Rectangle {
        readonly property real dx: root.px2 - root.px1
        readonly property real dy: root.py2 - root.py1
        readonly property real len: Math.sqrt(dx * dx + dy * dy)

        x: root.px1
        y: root.py1 - root.lineWidth / 2
        width: len
        height: root.lineWidth
        color: root.lineColor
        transformOrigin: Item.TopLeft
        rotation: Math.atan2(dy, dx) * 180 / Math.PI
    }

    // Returns the pixel-space distance from (px, py) to the ruler line segment.
    function distToLine(px, py) {
        const dx = root.px2 - root.px1;
        const dy = root.py2 - root.py1;
        const lenSq = dx * dx + dy * dy;
        if (lenSq < 0.0001) {
            const ex = px - root.px1;
            const ey = py - root.py1;
            return Math.sqrt(ex * ex + ey * ey);
        }
        const t = Math.max(0, Math.min(1, ((px - root.px1) * dx + (py - root.py1) * dy) / lenSq));
        const nx = root.px1 + t * dx - px;
        const ny = root.py1 + t * dy - py;
        return Math.sqrt(nx * nx + ny * ny);
    }

    // Midpoint distance label
    Rectangle {
        readonly property real mx: (root.px1 + root.px2) / 2
        readonly property real my: (root.py1 + root.py2) / 2

        x: mx - width / 2
        y: my - height - 8
        width: distLabel.implicitWidth + 10
        height: distLabel.implicitHeight + 6
        radius: 3
        color: root.palette.tooltipBackground
        visible: root.parent && root.parent.isInsidePlotArea(mx, my)

        Text {
            id: distLabel
            anchors.centerIn: parent
            color: root.palette.tooltipText
            font.pixelSize: 11
            font.bold: true
            text: "d = " + root.dataDistance.toFixed(2)
        }
    }

    Item {
        readonly property real dx: root.px2 - root.px1
        readonly property real dy: root.py2 - root.py1
        readonly property real len: Math.sqrt(dx * dx + dy * dy)

        x: root.px1
        y: root.py1 - 8
        width: len
        height: 16
        transformOrigin: Item.TopLeft
        rotation: Math.atan2(dy, dx) * 180 / Math.PI

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.SizeAllCursor

            property real lastX: 0
            property real lastY: 0

            onPressed: mouse => {
                const point = mapToItem(root, mouse.x, mouse.y);
                lastX = point.x;
                lastY = point.y;
                root.active = true;
            }

            onPositionChanged: mouse => {
                if (!pressed || !root.parent) {
                    return;
                }
                const point = mapToItem(root, mouse.x, mouse.y);
                const dxData = root.parent.pixelToDataX(point.x) - root.parent.pixelToDataX(lastX);
                const dyData = root.parent.pixelToDataY(point.y) - root.parent.pixelToDataY(lastY);
                root.x1Data += dxData;
                root.y1Data += dyData;
                root.x2Data += dxData;
                root.y2Data += dyData;
                lastX = point.x;
                lastY = point.y;
            }
        }
    }

    // Handle 1 — only visible when the tool is active
    Rectangle {
        id: handle1
        x: root.px1 - root.handleRadius
        y: root.py1 - root.handleRadius
        width: root.handleRadius * 2
        height: root.handleRadius * 2
        radius: root.handleRadius
        color: handle1Area.pressed ? Qt.lighter(root.handleColor, 1.5) : root.handleColor
        border.color: root.palette.handleBorder
        border.width: 2
        visible: root.active

        MouseArea {
            id: handle1Area
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeAllCursor

            onPressed: root.active = true
            onPositionChanged: mouse => {
                if (pressed) {
                    const pt = mapToItem(root, mouse.x, mouse.y);
                    root.x1Data = root.parent.pixelToDataX(pt.x);
                    root.y1Data = root.parent.pixelToDataY(pt.y);
                }
            }
        }
    }

    // Handle 2 — only visible when the tool is active
    Rectangle {
        id: handle2
        x: root.px2 - root.handleRadius
        y: root.py2 - root.handleRadius
        width: root.handleRadius * 2
        height: root.handleRadius * 2
        radius: root.handleRadius
        color: handle2Area.pressed ? Qt.lighter(root.handleColor, 1.5) : root.handleColor
        border.color: root.palette.handleBorder
        border.width: 2
        visible: root.active

        MouseArea {
            id: handle2Area
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeAllCursor

            onPressed: root.active = true
            onPositionChanged: mouse => {
                if (pressed) {
                    const pt = mapToItem(root, mouse.x, mouse.y);
                    root.x2Data = root.parent.pixelToDataX(pt.x);
                    root.y2Data = root.parent.pixelToDataY(pt.y);
                }
            }
        }
    }
}
