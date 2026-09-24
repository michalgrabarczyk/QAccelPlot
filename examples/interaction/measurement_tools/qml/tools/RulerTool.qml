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
// The ruler is drawn as a translucent band with tick marks spaced in data units.
// Dragging the band translates the entire tool. Drag handles are only visible
// when the tool is active.
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
    property color bandColor: palette.toolRuler
    property color handleColor: palette.toolRuler
    property real bandWidth: 24.0
    property real minTickSpacing: 6.0
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

    readonly property real pixelLength: Math.hypot(px2 - px1, py2 - py1)
    readonly property real angleDegrees: Math.atan2(py2 - py1, px2 - px1) * 180 / Math.PI

    // Smallest 1-2-5 data step whose ticks are at least minTickSpacing apart.
    readonly property var tickScale: {
        if (dataDistance <= 0 || pixelLength <= 0) {
            return null;
        }
        const raw = minTickSpacing * dataDistance / pixelLength;
        const base = Math.pow(10, Math.floor(Math.log10(raw)));
        const ratio = raw / base;
        const mantissa = ratio <= 1 ? 1 : ratio <= 2 ? 2 : ratio <= 5 ? 5 : 10;
        const majorEvery = mantissa === 2 ? 5 : mantissa === 5 ? 2 : 10;
        return {
            spacing: mantissa * base * pixelLength / dataDistance,
            majorEvery: majorEvery,
            midEvery: majorEvery === 10 ? 5 : 0
        };
    }

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
        py2 = parent.dataToPixelY(y2Data);    }

    // Band in ruler-local coordinates: x runs from handle 1 to handle 2 and the
    // measured segment is the horizontal center line.
    Item {
        id: band

        x: root.px1
        y: root.py1 - height / 2
        width: root.pixelLength
        height: root.bandWidth
        transformOrigin: Item.Left
        rotation: root.angleDegrees

        Rectangle {
            anchors.fill: parent
            radius: 2
            antialiasing: true
            color: Qt.rgba(root.bandColor.r, root.bandColor.g, root.bandColor.b, 0.16)
            border.color: Qt.rgba(root.bandColor.r, root.bandColor.g, root.bandColor.b, 0.55)
            border.width: 1
        }

        Rectangle {
            y: (parent.height - height) / 2
            width: parent.width
            height: 1
            antialiasing: true
            color: Qt.rgba(root.bandColor.r, root.bandColor.g, root.bandColor.b, 0.45)
        }

        Repeater {
            model: root.tickScale ? Math.min(1000, Math.floor(band.width / root.tickScale.spacing) + 1) : 0

            Item {
                id: tick

                required property int index
                readonly property bool major: index % root.tickScale.majorEvery === 0
                readonly property bool mid: root.tickScale.midEvery > 0 && index % root.tickScale.midEvery === 0
                readonly property real tickLength: major ? 10 : mid ? 7 : 4

                x: index * root.tickScale.spacing - width / 2
                width: 1
                height: band.height
                opacity: major ? 1.0 : 0.7

                Rectangle {
                    width: parent.width
                    height: tick.tickLength
                    antialiasing: true
                    color: root.bandColor
                }
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: tick.tickLength
                    antialiasing: true
                    color: root.bandColor
                }
            }
        }

        component EndCap: Rectangle {
            y: -2
            width: 2
            height: band.height + 4
            radius: 1
            antialiasing: true
            color: root.bandColor
        }

        EndCap {
            x: -width / 2
        }
        EndCap {
            x: band.width - width / 2
        }

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

    // Distance label, aligned with the band but kept upright. It sits inside
    // the band when there is room and above it otherwise.
    Item {
        readonly property real mx: (root.px1 + root.px2) / 2
        readonly property real my: (root.py1 + root.py2) / 2

        x: mx
        y: my
        rotation: root.angleDegrees > 90 ? root.angleDegrees - 180 : root.angleDegrees < -90 ? root.angleDegrees + 180 : root.angleDegrees
        visible: root.parent && root.parent.isInsidePlotArea(mx, my)

        Rectangle {
            readonly property bool fitsInBand: root.pixelLength >= width + 2 * (root.handleRadius + 8)

            x: -width / 2
            y: fitsInBand ? -height / 2 : -root.bandWidth / 2 - height - 4
            width: distLabel.implicitWidth + 12
            height: distLabel.implicitHeight + 4
            radius: height / 2
            antialiasing: true
            color: root.palette.tooltipBackground
            border.color: root.bandColor
            border.width: 1

            Text {
                id: distLabel
                anchors.centerIn: parent
                color: root.palette.tooltipText
                font.pixelSize: 11
                font.bold: true
                text: "d = " + root.dataDistance.toFixed(2)
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
