//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot 1.0

Rectangle {
    id: root

    required property var series
    property color textColor: "#333333"
    property font font: Qt.font({
        family: "Arial",
        pixelSize: 12
    })
    property real symbolWidth: 28

    visible: series.length > 0

    width: column.width + 16
    height: column.height + 12

    color: Qt.rgba(1, 1, 1, 0.7)
    radius: 4
    border.color: Qt.rgba(0, 0, 0, 0.15)
    border.width: 1

    Column {
        id: column

        x: 8
        y: 6
        spacing: 4

        Repeater {
            id: curveRepeater

            model: root.series

            Row {
                id: row

                spacing: 6

                Canvas {
                    id: symbol

                    property color curveColor: modelData.color !== undefined ? modelData.color : "#808080"
                    property var curveLineStyle: modelData.lineStyle || null
                    property int curveMarker: modelData.markerShape !== undefined ? modelData.markerShape : 0
                    property real curveMarkerSize: modelData.markerSize !== undefined ? modelData.markerSize : 4
                    property bool isFillSymbol: modelData.legendSymbol === PlotSeries.Fill

                    width: root.symbolWidth
                    height: 14
                    anchors.verticalCenter: parent.verticalCenter

                    onCurveColorChanged: requestPaint()
                    onCurveLineStyleChanged: requestPaint()
                    onCurveMarkerChanged: requestPaint()
                    onCurveMarkerSizeChanged: requestPaint()
                    onIsFillSymbolChanged: requestPaint()

                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.clearRect(0, 0, width, height);
                        var cy = Math.round(height / 2);
                        var showLine = true;
                        var dashPattern = [];

                        // Determine line visibility and dash pattern
                        if (curveLineStyle) {
                            if (curveLineStyle.showLine !== undefined)
                                showLine = curveLineStyle.showLine;
                            if (curveLineStyle.pattern !== undefined && curveLineStyle.pattern.length > 0) {
                                var raw = curveLineStyle.pattern;
                                for (var k = 0; k < raw.length; ++k)
                                    dashPattern.push(raw[k]);
                            }
                        }

                        // Draw a fill swatch or a line segment.
                        if (isFillSymbol) {
                            ctx.fillStyle = curveColor;
                            ctx.fillRect(2, 3, width - 4, height - 6);
                        } else if (showLine) {
                            var actualLineWidth = Math.min(modelData.lineWidth || 1, 4);
                            ctx.strokeStyle = curveColor;
                            ctx.lineWidth = actualLineWidth;
                            ctx.lineCap = "butt";
                            if (dashPattern.length > 0) {
                                // QML Canvas (backed by QPen) interprets dash pattern units as multiples
                                // of the line width. We must divide by lineWidth to get absolute pixels.
                                var scaledDash = [];
                                for (var i = 0; i < dashPattern.length; ++i)
                                    scaledDash.push(dashPattern[i] / actualLineWidth);
                                ctx.setLineDash(scaledDash);
                            } else {
                                ctx.setLineDash([]);
                            }
                            ctx.beginPath();
                            ctx.moveTo(0, cy);
                            ctx.lineTo(width, cy);
                            ctx.stroke();
                            ctx.setLineDash([]);
                        }

                        // Draw marker at center
                        var markerShape = curveMarker;
                        if (markerShape > 0) {
                            var cx = Math.round(width / 2);
                            var r = Math.min(curveMarkerSize, 5);
                            ctx.fillStyle = curveColor;
                            ctx.strokeStyle = curveColor;
                            ctx.lineWidth = 1.5;
                            switch (markerShape) {
                            case LineCurve.Circle:
                                ctx.beginPath();
                                ctx.arc(cx, cy, r, 0, 2 * Math.PI);
                                ctx.fill();
                                break;
                            case LineCurve.Square:
                                ctx.fillRect(cx - r, cy - r, 2 * r, 2 * r);
                                break;
                            case LineCurve.Diamond:
                                ctx.beginPath();
                                ctx.moveTo(cx, cy - r);
                                ctx.lineTo(cx + r, cy);
                                ctx.lineTo(cx, cy + r);
                                ctx.lineTo(cx - r, cy);
                                ctx.closePath();
                                ctx.fill();
                                break;
                            case LineCurve.TriangleUp:
                                ctx.beginPath();
                                ctx.moveTo(cx, cy - r);
                                ctx.lineTo(cx + r, cy + r);
                                ctx.lineTo(cx - r, cy + r);
                                ctx.closePath();
                                ctx.fill();
                                break;
                            case LineCurve.TriangleDown:
                                ctx.beginPath();
                                ctx.moveTo(cx, cy + r);
                                ctx.lineTo(cx + r, cy - r);
                                ctx.lineTo(cx - r, cy - r);
                                ctx.closePath();
                                ctx.fill();
                                break;
                            case LineCurve.Cross:
                                ctx.beginPath();
                                ctx.moveTo(cx - r, cy - r);
                                ctx.lineTo(cx + r, cy + r);
                                ctx.moveTo(cx + r, cy - r);
                                ctx.lineTo(cx - r, cy + r);
                                ctx.stroke();
                                break;
                            }
                        }
                    }
                }

                Text {
                    text: modelData.name || modelData.objectName || ("Curve " + (index + 1))
                    font: root.font
                    color: root.textColor
                }
            }
        }
    }
}
