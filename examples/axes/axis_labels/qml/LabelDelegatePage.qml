//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QAccelPlot as QAccelPlot

Item {
    id: root
    required property var colorPalette

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Label {
            text: "Each axis label is a QML item anchored to its axis: a unit above the y axis and a swatch with a title at the end of the x axis."
            color: root.colorPalette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                id: xAxis
                viewportMin: 0
                viewportMax: 10
                label.text: "Time (s)"
                label.delegate: Row {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 4
                    spacing: 6

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 12
                        height: 12
                        radius: 2
                        color: root.colorPalette.seriesPrimary
                    }
                    Text {
                        text: xAxis.label.text
                        font: xAxis.label.font
                        color: xAxis.hovered ? xAxis.hoverColor : xAxis.label.color
                    }
                }
            }

            yAxis: ExampleAxis {
                id: yAxis
                viewportMin: -1.2
                viewportMax: 1.2
                label.text: "mV"
                // Above the axis, over the tick labels.
                label.delegate: Text {
                    anchors.bottom: parent.top
                    anchors.right: parent.right
                    anchors.rightMargin: 20
                    text: yAxis.label.text
                    font: yAxis.label.font
                    color: yAxis.hovered ? yAxis.hoverColor : yAxis.label.color
                }
            }

            QAccelPlot.LineCurve {
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.colorPalette.seriesPrimary
                lineWidth: 2

                Component.onCompleted: {
                    const points = [];
                    for (let sample = 0; sample <= 400; ++sample) {
                        const t = sample / 40;
                        points.push(Qt.point(t, Math.exp(-0.25 * t) * Math.sin(2 * Math.PI * 0.8 * t)));
                    }
                    setData(points);
                }
            }
        }
    }
}
