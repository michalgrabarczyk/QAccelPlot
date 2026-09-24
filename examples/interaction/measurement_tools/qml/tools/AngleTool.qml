//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QtQuick.Shapes
import QAccelPlot as QAccelPlot
// AngleTool: measures the angle between two arms that share a common vertex.
//
// The vertex is the center handle; arm1 and arm2 are the two endpoints.
// The measured angle (in degrees) is the angle at the vertex.

Item {
    id: root

    required property var palette
    anchors.fill: parent
    opacity: active ? 1.0 : 0.78

    // Tool activation state (controls handle visibility)
    property bool active: false

    // Vertex (center point) in data coordinates
    property real vertexXData: 0.0
    property real vertexYData: 0.0

    // Arm 1 endpoint in data coordinates
    property real arm1XData: 1.0
    property real arm1YData: 0.0

    // Arm 2 endpoint in data coordinates
    property real arm2XData: 0.0
    property real arm2YData: 1.0

    // Style
    property color lineColor: palette.toolAngle
    property color handleColor: palette.toolAngle
    property real lineWidth: 2.0
    property real handleRadius: 6.0
    property real arcRadius: 30.0

    // Read-only: angle in degrees at vertex, measured in data coordinates.
    readonly property real angleDegrees: {
        const ax = arm1XData - vertexXData;
        const ay = arm1YData - vertexYData;
        const bx = arm2XData - vertexXData;
        const by = arm2YData - vertexYData;
        const dot = ax * bx + ay * by;
        const magA = Math.sqrt(ax * ax + ay * ay);
        const magB = Math.sqrt(bx * bx + by * by);
        if (magA === 0 || magB === 0) {
            return 0;
        }
        const cosAngle = Math.max(-1, Math.min(1, dot / (magA * magB)));
        return Math.acos(cosAngle) * (180 / Math.PI);
    }

    // Pixel positions (updated when data or plot transform changes)
    property real pxV: 0
    property real pyV: 0
    property real pxA1: 0
    property real pyA1: 0
    property real pxA2: 0
    property real pyA2: 0

    // Recompute on data-coordinate changes
    onVertexXDataChanged: updateLines()
    onVertexYDataChanged: updateLines()
    onArm1XDataChanged: updateLines()
    onArm1YDataChanged: updateLines()
    onArm2XDataChanged: updateLines()
    onArm2YDataChanged: updateLines()
    onActiveChanged: {
        if (active) {
            window.activateTool(root);
        }
    }
    Component.onCompleted: updateLines()

    // Recompute when axis range changes (pan / zoom)
    Connections {
        target: root.parent ? root.parent.xAxis : null
        function onRangeChanged() {
            root.updateLines();
        }
    }
    Connections {
        target: root.parent ? root.parent.yAxis : null
        function onRangeChanged() {
            root.updateLines();
        }
    }
    // Recompute when plot area is resized or layout changes
    Connections {
        target: root.parent
        function onPlotRectChanged() {
            root.updateLines();
        }
    }

    // Update pixel positions from data coordinates
    function updateLines() {
        if (!parent) {
            return;
        }
        pxV = parent.dataToPixelX(vertexXData);
        pyV = parent.dataToPixelY(vertexYData);
        pxA1 = parent.dataToPixelX(arm1XData);
        pyA1 = parent.dataToPixelY(arm1YData);
        pxA2 = parent.dataToPixelX(arm2XData);
        pyA2 = parent.dataToPixelY(arm2YData);
        rebuildArcPath();
    }

    // Arc path data for the angle indicator
    property string arcPathData: ""

    function rebuildArcPath() {
        const startAngle = Math.atan2(pyA1 - pyV, pxA1 - pxV);
        const endAngle = Math.atan2(pyA2 - pyV, pxA2 - pxV);
        let sweep = endAngle - startAngle;
        if (sweep > Math.PI) {
            sweep -= 2 * Math.PI;
        }
        if (sweep < -Math.PI) {
            sweep += 2 * Math.PI;
        }
        const segments = 20;
        let d = "";
        for (let i = 0; i <= segments; ++i) {
            const a = startAngle + sweep * i / segments;
            const px = pxV + Math.cos(a) * arcRadius;
            const py = pyV + Math.sin(a) * arcRadius;
            d += (i === 0 ? "M" : "L") + px.toFixed(2) + " " + py.toFixed(2) + " ";
        }
        arcPathData = d;
    }

    // Arm 1 line
    Rectangle {
        readonly property real dx: root.pxA1 - root.pxV
        readonly property real dy: root.pyA1 - root.pyV
        readonly property real len: Math.sqrt(dx * dx + dy * dy)
        x: root.pxV
        y: root.pyV - root.lineWidth / 2
        width: len
        height: root.lineWidth
        color: root.lineColor
        transformOrigin: Item.TopLeft
        rotation: Math.atan2(dy, dx) * 180 / Math.PI
    }

    // Arm 2 line
    Rectangle {
        readonly property real dx: root.pxA2 - root.pxV
        readonly property real dy: root.pyA2 - root.pyV
        readonly property real len: Math.sqrt(dx * dx + dy * dy)
        x: root.pxV
        y: root.pyV - root.lineWidth / 2
        width: len
        height: root.lineWidth
        color: root.lineColor
        transformOrigin: Item.TopLeft
        rotation: Math.atan2(dy, dx) * 180 / Math.PI
    }

    // Arc indicator
    Shape {
        anchors.fill: parent
        ShapePath {
            strokeColor: Qt.rgba(root.lineColor.r, root.lineColor.g, root.lineColor.b, 0.6)
            strokeWidth: 1.5
            fillColor: root.palette.transparent
            PathSvg {
                path: root.arcPathData
            }
        }
    }
    function distToSegment(px, py, x1, y1, x2, y2) {
        const dx = x2 - x1, dy = y2 - y1;
        const lenSq = dx * dx + dy * dy;
        if (lenSq < 0.0001) {
            const ex = px - x1, ey = py - y1;
            return Math.sqrt(ex * ex + ey * ey);
        }
        const t = Math.max(0, Math.min(1, ((px - x1) * dx + (py - y1) * dy) / lenSq));
        const nx = x1 + t * dx - px, ny = y1 + t * dy - py;
        return Math.sqrt(nx * nx + ny * ny);
    }

    // Angle label near bisector
    Rectangle {
        readonly property real bisX: {
            const ax = root.pxA1 - root.pxV;
            const ay = root.pyA1 - root.pyV;
            const bx = root.pxA2 - root.pxV;
            const by = root.pyA2 - root.pyV;
            const mx = ax + bx;
            const my = ay + by;
            const ml = Math.sqrt(mx * mx + my * my);
            if (ml === 0) {
                return root.pxV;
            }
            return root.pxV + (mx / ml) * (root.arcRadius + 20);
        }
        readonly property real bisY: {
            const ax = root.pxA1 - root.pxV;
            const ay = root.pyA1 - root.pyV;
            const bx = root.pxA2 - root.pxV;
            const by = root.pyA2 - root.pyV;
            const mx = ax + bx;
            const my = ay + by;
            const ml = Math.sqrt(mx * mx + my * my);
            if (ml === 0) {
                return root.pyV;
            }
            return root.pyV + (my / ml) * (root.arcRadius + 20);
        }

        x: bisX - width / 2
        y: bisY - height / 2
        width: angleLabel.implicitWidth + 10
        height: angleLabel.implicitHeight + 6
        radius: 3
        color: root.palette.tooltipBackground

        Text {
            id: angleLabel
            anchors.centerIn: parent
            color: root.palette.tooltipText
            font.pixelSize: 11
            font.bold: true
            text: root.angleDegrees.toFixed(1) + "°"
        }
    }

    // A narrow hit target follows each arm, avoiding cursor interference when
    // several angle tools overlap in the same plot.
    component ArmDragTarget: Item {
        property real endX: 0
        property real endY: 0

        readonly property real dx: endX - root.pxV
        readonly property real dy: endY - root.pyV
        readonly property real len: Math.sqrt(dx * dx + dy * dy)

        x: root.pxV
        y: root.pyV - 8
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
                root.vertexXData += dxData;
                root.vertexYData += dyData;
                root.arm1XData += dxData;
                root.arm1YData += dyData;
                root.arm2XData += dxData;
                root.arm2YData += dyData;
                lastX = point.x;
                lastY = point.y;
            }
        }
    }

    ArmDragTarget {
        endX: root.pxA1
        endY: root.pyA1
    }
    ArmDragTarget {
        endX: root.pxA2
        endY: root.pyA2
    }

    // Helper for creating handles
    component DragHandle: Rectangle {
        id: dragHandle

        property real dataX: 0
        property real dataY: 0
        property real pixelX: 0
        property real pixelY: 0

        x: pixelX - root.handleRadius
        y: pixelY - root.handleRadius
        width: root.handleRadius * 2
        height: root.handleRadius * 2
        radius: root.handleRadius
        color: handleMA.pressed ? Qt.lighter(root.handleColor, 1.5) : root.handleColor
        border.color: root.palette.handleBorder
        border.width: 2
        visible: root.active

        signal dragged(real newDataX, real newDataY)

        MouseArea {
            id: handleMA
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeAllCursor

            onPressed: root.active = true
            onPositionChanged: mouse => {
                if (pressed) {
                    const pt = mapToItem(root, mouse.x, mouse.y);
                    dragHandle.dragged(root.parent.pixelToDataX(pt.x), root.parent.pixelToDataY(pt.y));
                }
            }
        }
    }

    // Vertex handle
    DragHandle {
        pixelX: root.pxV
        pixelY: root.pyV
        onDragged: (dx, dy) => {
            root.vertexXData = dx;
            root.vertexYData = dy;
        }
    }

    // Arm 1 handle
    DragHandle {
        pixelX: root.pxA1
        pixelY: root.pyA1
        onDragged: (dx, dy) => {
            root.arm1XData = dx;
            root.arm1YData = dy;
        }
    }

    // Arm 2 handle
    DragHandle {
        pixelX: root.pxA2
        pixelY: root.pyA2
        onDragged: (dx, dy) => {
            root.arm2XData = dx;
            root.arm2YData = dy;
        }
    }
}
