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

    // Read by C++ once per animation tick.
    property int pointCount: pointCountOptions[pointCountBox.currentIndex]
    property bool animate: false
    // Written by C++: number of finite, positive samples in the power-law series.
    property int powerLawValidCount: 0

    readonly property var pointCountOptions: [50000, 250000, 1000000]
    readonly property QtObject colorPalette: QAccelPlot.Colors.dark

    width: 1200
    height: 800
    visible: true
    title: "Point Cloud Example"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    Gradient {
        id: distanceColorMap
        orientation: Gradient.Horizontal
        GradientStop {
            position: 0.0
            color: "#fde725"
        }
        GradientStop {
            position: 0.3
            color: "#5ec962"
        }
        GradientStop {
            position: 0.6
            color: "#21918c"
        }
        GradientStop {
            position: 0.85
            color: "#3b528b"
        }
        GradientStop {
            position: 1.0
            color: "#440154"
        }
    }

    component StyledPlot: QAccelPlot.Plot {
        border.color: window.colorPalette.plotBorder
        border.width: 2
    }

    component PanelTitle: Label {
        color: window.colorPalette.text
        font.bold: true
        font.pixelSize: 13
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        ExampleHeader {
            title: "Point clouds"
            description: "Unconnected scatter data rendered as GPU markers: value-colored clusters and log-log axes that skip invalid samples. Hover a point on the left plot; pan and zoom any plot."
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "Points"
                color: colorPalette.text
            }
            ComboBox {
                id: pointCountBox
                model: ["50 000", "250 000", "1 000 000"]
                currentIndex: 1
                Layout.preferredWidth: 140
                Material.background: colorPalette.plotArea
            }
            Label {
                text: "Marker radius " + markerSizeSlider.value.toFixed(1) + " px"
                color: colorPalette.text
            }
            Slider {
                id: markerSizeSlider
                from: 0.5
                to: 8.0
                stepSize: 0.5
                value: 1.5
                Layout.preferredWidth: 160
            }
            Switch {
                id: antialiasingSwitch
                text: "Antialiasing"
                checked: true
            }
            Switch {
                text: "Animate"
                checked: window.animate
                onToggled: window.animate = checked
            }
            Item {
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            // ── Clustered returns: large count + value colormap + hover ─────────────
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 3
                spacing: 4

                PanelTitle {
                    text: "Clustered returns · " + clusterCloud.count.toLocaleString(Qt.locale(), "f", 0) + " points"
                }

                StyledPlot {
                    id: clusterPlot
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    legend: QAccelPlot.Legend {
                        series: clusterPlot.series
                    }

                    xAxis: ExampleAxis {
                        viewportMin: -10
                        viewportMax: 10
                        dataMin: -10
                        dataMax: 10
                        label: "East offset (m)"
                    }

                    yAxis: ExampleAxis {
                        viewportMin: -8
                        viewportMax: 8
                        dataMin: -8
                        dataMax: 8
                        axisTitlePadding: 34
                        layoutSize: 54
                        label: "North offset (m)"
                    }

                    QAccelPlot.PointCloud {
                        id: clusterCloud
                        objectName: "clusterCloud"
                        name: "Sensor returns"
                        xAxis: clusterPlot.xAxis
                        yAxis: clusterPlot.yAxis
                        // Legend swatch: a mid-range colormap color.
                        color: "#21918c"
                        marker.shape: QAccelPlot.PointCloud.Circle
                        marker.size: markerSizeSlider.value
                        antialiasingEnabled: antialiasingSwitch.checked
                        // The colour bar below paints the same stops, so share them rather than
                        // repeating the ramp or reaching for a preset that would not match it.
                        colormap: QAccelPlot.Colormap {
                            stops: distanceColorMap.stops
                            min: 0
                            max: 1
                        }
                        hoverRadius: 8
                    }

                    Item {
                        id: clusterOverlay
                        z: 2
                        x: clusterPlot.plotRect.x
                        y: clusterPlot.plotRect.y
                        width: clusterPlot.plotRect.width
                        height: clusterPlot.plotRect.height
                        clip: true

                        // Color bar bound to the resolved value range of the series.
                        Rectangle {
                            anchors.left: parent.left
                            anchors.bottom: parent.bottom
                            anchors.margins: 10
                            width: 190
                            height: colorBarColumn.implicitHeight + 12
                            radius: 4
                            color: colorPalette.legendBackground
                            border.color: colorPalette.legendBorder

                            Column {
                                id: colorBarColumn
                                anchors.fill: parent
                                anchors.margins: 6
                                spacing: 3

                                Text {
                                    text: "Distance from cluster core (σ)"
                                    color: colorPalette.text
                                    font.pixelSize: 11
                                }
                                Rectangle {
                                    width: parent.width
                                    height: 10
                                    gradient: distanceColorMap
                                }
                                Item {
                                    width: parent.width
                                    height: minLabel.implicitHeight
                                    Text {
                                        id: minLabel
                                        text: (clusterCloud.dataValueMin * 3).toFixed(0)
                                        color: colorPalette.textSecondary
                                        font.pixelSize: 10
                                    }
                                    Text {
                                        anchors.right: parent.right
                                        text: (clusterCloud.dataValueMax * 3).toFixed(0) + "+"
                                        color: colorPalette.textSecondary
                                        font.pixelSize: 10
                                    }
                                }
                            }
                        }

                        QAccelPlot.DataAnchor {
                            id: hoverAnchor

                            readonly property point hoveredPoint: clusterCloud.hoveredIndex >= 0 ? clusterCloud.pointAt(clusterCloud.hoveredIndex) : Qt.point(0, 0)

                            visible: clusterCloud.hoveredIndex >= 0
                            xAxis: clusterPlot.xAxis
                            yAxis: clusterPlot.yAxis
                            plotRect: Qt.rect(0, 0, clusterOverlay.width, clusterOverlay.height)
                            dataX1: hoveredPoint.x
                            dataY1: hoveredPoint.y
                            dataX2: hoveredPoint.x
                            dataY2: hoveredPoint.y

                            Rectangle {
                                anchors.centerIn: parent
                                width: markerSizeSlider.value * 2 + 8
                                height: width
                                radius: width / 2
                                color: "transparent"
                                border.color: colorPalette.annotationMarkerOutline
                                border.width: 2
                            }

                            Rectangle {
                                anchors.left: parent.right
                                anchors.bottom: parent.top
                                anchors.leftMargin: 10
                                anchors.bottomMargin: 6
                                width: tooltipText.implicitWidth + 12
                                height: tooltipText.implicitHeight + 8
                                radius: 3
                                color: colorPalette.tooltipBackground

                                Text {
                                    id: tooltipText
                                    anchors.centerIn: parent
                                    color: colorPalette.tooltipText
                                    font.pixelSize: 11
                                    text: clusterCloud.hoveredIndex < 0 ? "" : "#" + clusterCloud.hoveredIndex + "  (" + hoverAnchor.hoveredPoint.x.toFixed(2) + ", " + hoverAnchor.hoveredPoint.y.toFixed(2) + ")  value " + clusterCloud.valueAt(clusterCloud.hoveredIndex).toFixed(2)
                                }
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 2
                spacing: 4

                // ── Power law: log-log axes, invalid samples skipped ───────────────
                PanelTitle {
                    text: "Power-law scatter (log-log) · " + window.powerLawValidCount + " valid of " + powerLawCloud.count
                }

                StyledPlot {
                    id: powerLawPlot
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    legendVisible: false

                    xAxis: ExampleAxis {
                        viewportMin: 1
                        viewportMax: 10000
                        dataMin: 1
                        dataMax: 10000
                        logScale: true
                        label: "Event energy (keV)"
                    }

                    yAxis: ExampleAxis {
                        viewportMin: 1
                        viewportMax: 10000
                        dataMin: 1
                        dataMax: 10000
                        logScale: true
                        axisTitlePadding: 34
                        layoutSize: 54
                        label: "Counts"
                    }

                    QAccelPlot.PointCloud {
                        id: powerLawCloud
                        objectName: "powerLawCloud"
                        name: "Detector events"
                        xAxis: powerLawPlot.xAxis
                        yAxis: powerLawPlot.yAxis
                        // seriesSecondary at ~55 % alpha, so overlapping samples read as density.
                        color: "#8cffb454"
                        marker.shape: QAccelPlot.PointCloud.Diamond
                        marker.size: 3
                        antialiasingEnabled: antialiasingSwitch.checked
                    }
                }
            }
        }
    }
}
