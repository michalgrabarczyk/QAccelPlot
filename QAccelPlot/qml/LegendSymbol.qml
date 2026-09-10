//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot 1.0

Item {
    id: root

    required property var sourceSeries
    property real requestedWidth: 28

    readonly property bool isFillSymbol: sourceSeries.legendSymbol === PlotSeries.Fill
    readonly property color curveColor: sourceSeries.color !== undefined ? sourceSeries.color : "#808080"
    readonly property var curveLineStyle: sourceSeries.lineStyle !== undefined ? sourceSeries.lineStyle : null
    readonly property bool showLine: curveLineStyle ? curveLineStyle.showLine : true
    readonly property int curveMarker: sourceSeries.markerShape !== undefined ? sourceSeries.markerShape : LineCurve.None
    readonly property real curveMarkerSize: sourceSeries.markerSize !== undefined ? Math.max(0, sourceSeries.markerSize) : 0
    readonly property real curveLineWidth: sourceSeries.lineWidth !== undefined ? Math.max(0, sourceSeries.lineWidth) : 1
    readonly property bool curveAntialiasingEnabled: sourceSeries.antialiasingEnabled !== undefined ? sourceSeries.antialiasingEnabled : true
    readonly property real curveAntialiasingFeather: sourceSeries.antialiasingFeather !== undefined ? sourceSeries.antialiasingFeather : 1
    readonly property bool hasMarker: curveMarker !== LineCurve.None

    width: Math.max(requestedWidth, hasMarker ? curveMarkerSize * 2 + 2 : 0)
    height: Math.max(14, hasMarker ? curveMarkerSize * 2 + 2 : 0, showLine ? curveLineWidth + 2 : 0)

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
        markerShape: LineCurve.None
        antialiasingEnabled: root.curveAntialiasingEnabled
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
        markerShape: root.curveMarker
        markerSize: root.curveMarkerSize
        antialiasingEnabled: root.curveAntialiasingEnabled
        antialiasingFeather: root.curveAntialiasingFeather

        Component.onCompleted: setData([Qt.point(0.5, 0.5)])
    }
}
