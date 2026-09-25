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

Window {
    id: window

    readonly property QtObject colorPalette: QAccelPlot.Colors.dark

    width: 1200
    height: 800
    visible: true
    title: "Point Cloud Example"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        ExampleHeader {
            title: "Point cloud"
            description: "200,000 points of a Clifford attractor in one PointCloud, colored by the length of the jump that reached each point. Hover a point to read it; pan and zoom to explore."

            Label {
                text: "Marker radius " + markerSizeSlider.value.toFixed(1) + " px"
                color: colorPalette.text
            }
            Slider {
                id: markerSizeSlider
                from: 0.5
                to: 4
                stepSize: 0.1
                value: 1
                Layout.preferredWidth: 160
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            border.color: colorPalette.plotBorder
            border.width: 2
            legendVisible: false

            xAxis: ExampleAxis {
                viewportMin: -3.2
                viewportMax: 3.2
                dataMin: -3.2
                dataMax: 3.2
                label: "x"
            }

            yAxis: ExampleAxis {
                viewportMin: -1.6
                viewportMax: 1.6
                dataMin: -1.6
                dataMax: 1.6
                label: "y"
            }

            // Filled from C++ with setDataF(), which also passes one value per point.
            QAccelPlot.PointCloud {
                id: cloud
                objectName: "cloud"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                marker.shape: QAccelPlot.PointCloud.Circle
                marker.size: markerSizeSlider.value
                colormap: QAccelPlot.Colormap {
                    preset: QAccelPlot.Colormap.Plasma
                }
            }

            Item {
                id: overlay
                x: plot.plotRect.x
                y: plot.plotRect.y
                width: plot.plotRect.width
                height: plot.plotRect.height
                clip: true

                QAccelPlot.DataAnchor {
                    id: hoverAnchor

                    readonly property point hovered: cloud.hoveredIndex >= 0 ? cloud.pointAt(cloud.hoveredIndex) : Qt.point(0, 0)

                    visible: cloud.hoveredIndex >= 0
                    xAxis: plot.xAxis
                    yAxis: plot.yAxis
                    plotRect: Qt.rect(0, 0, overlay.width, overlay.height)
                    dataX1: hovered.x
                    dataY1: hovered.y
                    dataX2: hovered.x
                    dataY2: hovered.y

                    Rectangle {
                        anchors.centerIn: parent
                        width: markerSizeSlider.value * 2 + 10
                        height: width
                        radius: width / 2
                        color: "transparent"
                        border.color: colorPalette.annotationMarkerOutline
                        border.width: 2
                    }

                    Rectangle {
                        x: 10
                        y: -height - 8
                        width: tooltipText.implicitWidth + 12
                        height: tooltipText.implicitHeight + 8
                        radius: 3
                        color: colorPalette.tooltipBackground

                        Text {
                            id: tooltipText
                            anchors.centerIn: parent
                            color: colorPalette.tooltipText
                            font.pixelSize: 11
                            text: "#" + cloud.hoveredIndex + "  (" + hoverAnchor.hovered.x.toFixed(3) + ", " + hoverAnchor.hovered.y.toFixed(3) + ")  jump " + cloud.valueAt(cloud.hoveredIndex).toFixed(3)
                        }
                    }
                }
            }
        }
    }
}
