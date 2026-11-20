//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
import QtQuick
import QAccelPlot

Item {
    id: root

    // A LegendEntry, as LegendModel provides it.
    required property var entry
    property real requestedWidth: 28

    readonly property bool isFillSymbol: entry.symbol === PlotSeries.Fill
    readonly property bool isMarkerSymbol: entry.symbol === PlotSeries.Marker
    readonly property bool isCandleSymbol: entry.symbol === PlotSeries.Candle
    readonly property color curveColor: entry.color
    readonly property var curveLineStyle: entry.lineStyle
    readonly property bool showLine: !isMarkerSymbol && !isCandleSymbol && (curveLineStyle ? curveLineStyle.showLine : true)
    readonly property int curveMarker: entry.markerShape
    // Marker-only series such as dense point clouds often use tiny markers; keep their swatch legible.
    readonly property real curveMarkerSize: isMarkerSymbol ? Math.min(Math.max(entry.markerSize, 3.5), 7) : entry.markerSize
    readonly property bool curveMarkerFilled: entry.markerFilled
    readonly property real curveMarkerStrokeWidth: entry.markerStrokeWidth
    readonly property real curveLineWidth: Math.max(0, entry.lineWidth)
    readonly property bool curveAntialiasing: entry.antialiasing
    readonly property real curveAntialiasingFeather: entry.antialiasingFeather
    readonly property bool hasMarker: curveMarker !== MarkerShape.None
    // A single pixel is hard to see in a legend, so Pixel markers are shown as a small dot.
    readonly property bool pixelMarker: curveMarker === MarkerShape.Pixel
    readonly property int sampleMarker: pixelMarker ? MarkerShape.Circle : curveMarker
    readonly property real sampleMarkerSize: pixelMarker ? 1.5 : curveMarkerSize

    width: Math.max(requestedWidth, hasMarker ? sampleMarkerSize * 2 + 2 : 0)
    height: Math.max(14, hasMarker ? sampleMarkerSize * 2 + 2 : 0, showLine ? curveLineWidth + 2 : 0)

    Axis {
        id: sampleXAxis

        visible: false
        viewportMin: 0
        viewportMax: 1
        dataMin: 0
        dataMax: 1
    }

    Axis {
        id: sampleYAxis

        visible: false
        side: Axis.Left
        viewportMin: 0
        viewportMax: 1
        dataMin: 0
        dataMax: 1
    }

    SolidLine {
        id: defaultLineStyle
    }

    NoLine {
        id: markerOnlyStyle
    }

    Rectangle {
        visible: root.isFillSymbol
        x: 2
        y: 3
        width: Math.max(0, root.width - 4)
        height: Math.max(0, root.height - 6)
        color: root.curveColor
    }

    // An up candle next to a down candle, each a body over a wick.
    Repeater {
        model: root.isCandleSymbol ? 2 : 0

        delegate: Item {
            id: candleSample

            required property int index
            readonly property bool up: index === 0
            readonly property color candleColor: up ? root.curveColor : root.entry.secondaryColor
            readonly property real candleWidth: 6
            readonly property real gap: 3

            x: (root.width - 2 * candleWidth - gap) / 2 + index * (candleWidth + gap)
            y: 1
            width: candleWidth
            height: root.height - 2

            Rectangle {
                x: (parent.width - width) / 2
                width: Math.max(1, root.curveLineWidth)
                height: parent.height
                color: candleSample.candleColor
            }

            Rectangle {
                y: parent.height * (candleSample.up ? 0.35 : 0.2)
                width: parent.width
                height: parent.height * 0.45
                color: candleSample.candleColor
            }
        }
    }

    LineCurve {
        id: lineSample

        anchors.fill: root
        visible: !root.isFillSymbol && root.showLine
        enabled: false
        xAxis: sampleXAxis
        yAxis: sampleYAxis
        color: root.curveColor
        lineWidth: root.curveLineWidth
        lineStyle: root.curveLineStyle || defaultLineStyle
        antialiasing: root.curveAntialiasing
        antialiasingFeather: root.curveAntialiasingFeather

        Component.onCompleted: setData([Qt.point(0, 0.5), Qt.point(1, 0.5)])
    }

    LineCurve {
        id: markerSample

        anchors.fill: root
        visible: !root.isFillSymbol && root.hasMarker
        enabled: false
        xAxis: sampleXAxis
        yAxis: sampleYAxis
        color: root.curveColor
        lineStyle: markerOnlyStyle
        marker.shape: root.sampleMarker
        marker.size: root.sampleMarkerSize
        marker.filled: root.curveMarkerFilled
        marker.strokeWidth: root.curveMarkerStrokeWidth
        antialiasing: root.curveAntialiasing
        antialiasingFeather: root.curveAntialiasingFeather

        Component.onCompleted: setData([Qt.point(0.5, 0.5)])
    }
}
