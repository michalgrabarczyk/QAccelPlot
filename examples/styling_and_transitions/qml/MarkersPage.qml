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
    required property var palette
    required property var typography
    property real markerSize: 8
    property real markerStrokeWidth: 1.5
    property bool antialiasingEnabled: true

    // The gallery's shape names are its only labels, so they are drawn larger than other tick labels.
    readonly property font galleryTickLabelFont: Qt.font({
        pixelSize: 14,
        weight: Font.Normal
    })

    // Pixel draws a single pixel, so it is shown in the dense scatter plot instead of the gallery.
    readonly property var shapes: [
        { name: "Circle", shape: QAccelPlot.LineCurve.Circle },
        { name: "Square", shape: QAccelPlot.LineCurve.Square },
        { name: "Diamond", shape: QAccelPlot.LineCurve.Diamond },
        { name: "TriangleUp", shape: QAccelPlot.LineCurve.TriangleUp },
        { name: "TriangleDown", shape: QAccelPlot.LineCurve.TriangleDown },
        { name: "TriangleLeft", shape: QAccelPlot.LineCurve.TriangleLeft },
        { name: "TriangleRight", shape: QAccelPlot.LineCurve.TriangleRight },
        { name: "Pentagon", shape: QAccelPlot.LineCurve.Pentagon },
        { name: "Hexagon", shape: QAccelPlot.LineCurve.Hexagon },
        { name: "Star", shape: QAccelPlot.LineCurve.Star },
        { name: "Cross", shape: QAccelPlot.LineCurve.Cross },
        { name: "XCross", shape: QAccelPlot.LineCurve.XCross },
        { name: "Asterisk", shape: QAccelPlot.LineCurve.Asterisk },
        { name: "HLine", shape: QAccelPlot.LineCurve.HLine },
        { name: "VLine", shape: QAccelPlot.LineCurve.VLine }
    ]

    // One gallery cell per shape and fill style: shapes run left to right, filled above hollow.
    readonly property var galleryCells: {
        const cells = [];
        for (let column = 0; column < shapes.length; ++column) {
            cells.push({ shape: shapes[column].shape, filled: true, x: column, y: 1 });
            cells.push({ shape: shapes[column].shape, filled: false, x: column, y: 0 });
        }
        return cells;
    }

    readonly property var shapeLabels: shapes.map(entry => entry.name)

    // Deterministic Gaussian cloud, so every run and screenshot shows the same points.
    function gaussianCloud(count, centerX, centerY, spread, seed) {
        let state = seed;
        function random() {
            state = (Math.imul(state, 1664525) + 1013904223) >>> 0;
            return (state + 0.5) / 4294967296;
        }
        const points = [];
        for (let i = 0; i < count; ++i) {
            const radius = Math.sqrt(-2 * Math.log(random())) * spread;
            const angle = 2 * Math.PI * random();
            points.push(Qt.point(centerX + radius * Math.cos(angle), centerY + 0.6 * radius * Math.sin(angle)));
        }
        return points;
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: "Every marker shape, filled and hollow. For dense scatter plots, Pixel markers draw one pixel per sample and hollow markers keep overlapping points readable."
            color: root.palette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            // Value labels keep the width of their widest value so the sliders do not shift while dragging.
            Label {
                id: sizeLabel
                text: "Size " + root.markerSize.toFixed(0) + " px"
                color: root.palette.text
                Layout.preferredWidth: Math.ceil(widestSizeText.advanceWidth)

                TextMetrics {
                    id: widestSizeText
                    font: sizeLabel.font
                    text: "Size 88 px"
                }
            }
            Slider {
                from: 3
                to: 12
                stepSize: 1
                value: root.markerSize
                Layout.preferredWidth: 120
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onMoved: root.markerSize = value
            }
            Label {
                id: outlineLabel
                text: "Outline " + root.markerStrokeWidth.toFixed(1) + " px"
                color: root.palette.text
                Layout.preferredWidth: Math.ceil(widestOutlineText.advanceWidth)

                TextMetrics {
                    id: widestOutlineText
                    font: outlineLabel.font
                    text: "Outline 8.8 px"
                }
            }
            Slider {
                from: 0.5
                to: 4
                stepSize: 0.5
                value: root.markerStrokeWidth
                Layout.preferredWidth: 120
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onMoved: root.markerStrokeWidth = value
            }
            Switch {
                text: "Antialiasing"
                checked: root.antialiasingEnabled
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onToggled: root.antialiasingEnabled = checked
            }
            Item {
                Layout.fillWidth: true
            }
        }

        ColumnLayout {
            spacing: 10
            Layout.fillWidth: true
            Layout.fillHeight: true

            QAccelPlot.Plot {
                id: densePlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                plotAreaColor: root.palette.plotArea
                axesAreaColor: root.palette.axesArea
                grid.gridColor: root.palette.grid
                grid.subGridVisible: false
                legend: QAccelPlot.Legend {
                    series: densePlot.series
                    color: root.palette.legendBackground
                    textColor: root.palette.text
                    border.color: root.palette.legendBorder
                }

                xAxis: QAccelPlot.Axis {
                    viewportMin: -8
                    viewportMax: 8
                    dataMin: -8
                    dataMax: 8
                    label: "X"
                    baselineColor: root.palette.axisLine
                    labelColor: root.palette.axisLabel
                    labelFont: root.typography.axisLabel
                    hoverColor: root.palette.hover
                    ticker.tickColor: root.palette.tick
                    ticker.tickLabelColor: root.palette.axisTickLabel
                    ticker.tickLabelFont: root.typography.axisTickLabel
                    ticker.subtickColor: root.palette.subtick
                }

                yAxis: QAccelPlot.Axis {
                    viewportMin: -5
                    viewportMax: 5
                    dataMin: -5
                    dataMax: 5
                    axisTitlePadding: 40
                    layoutSize: 60
                    label: "Y"
                    baselineColor: root.palette.axisLine
                    labelColor: root.palette.axisLabel
                    labelFont: root.typography.axisLabel
                    hoverColor: root.palette.hover
                    ticker.tickColor: root.palette.tick
                    ticker.tickLabelColor: root.palette.axisTickLabel
                    ticker.tickLabelFont: root.typography.axisTickLabel
                    ticker.subtickColor: root.palette.subtick
                }

                QAccelPlot.LineCurve {
                    name: "Pixel, 40 000 points"
                    xAxis: densePlot.xAxis
                    yAxis: densePlot.yAxis
                    color: Qt.rgba(root.palette.seriesPrimary.r, root.palette.seriesPrimary.g, root.palette.seriesPrimary.b, 0.6)
                    lineStyle: QAccelPlot.NoLine {}
                    marker.shape: QAccelPlot.LineCurve.Pixel
                    Component.onCompleted: setData(root.gaussianCloud(40000, -2.5, 1.5, 1.0, 7))
                }

                QAccelPlot.LineCurve {
                    name: "Hollow circle, 2 000 points"
                    xAxis: densePlot.xAxis
                    yAxis: densePlot.yAxis
                    color: root.palette.seriesSecondary
                    lineStyle: QAccelPlot.NoLine {}
                    marker.shape: QAccelPlot.LineCurve.Circle
                    marker.filled: false
                    marker.size: 4
                    marker.strokeWidth: 1
                    antialiasingEnabled: root.antialiasingEnabled
                    Component.onCompleted: setData(root.gaussianCloud(2000, 2.5, -1.5, 1.0, 11))
                }
            }

            QAccelPlot.Plot {
                id: galleryPlot
                Layout.fillWidth: true
                Layout.preferredHeight: 300
                legendVisible: false
                plotAreaColor: root.palette.plotArea
                axesAreaColor: root.palette.axesArea
                grid.gridColor: root.palette.grid
                grid.subGridVisible: false

                xAxis: QAccelPlot.Axis {
                    viewportMin: -0.6
                    viewportMax: root.shapes.length - 0.4
                    dataMin: -0.6
                    dataMax: root.shapes.length - 0.4
                    layoutSize: 118
                    axisTitlePadding: 100
                    label: "Shape"
                    baselineColor: root.palette.axisLine
                    labelColor: root.palette.axisLabel
                    labelFont: root.typography.axisLabel
                    hoverColor: root.palette.hover
                    // One tick per shape: range / tickCount must stay at or below 1.
                    ticker.tickCount: root.shapes.length + 1
                    ticker.subtickCount: 0
                    ticker.tickLabelRotation: -40
                    ticker.tickLabelPadding: 25
                    ticker.tickColor: root.palette.tick
                    ticker.tickLabelColor: root.palette.axisTickLabel
                    ticker.tickLabelFont: root.galleryTickLabelFont
                    ticker.tickLabelFormatter: QAccelPlot.TextTickLabelFormatter {
                        labels: root.shapeLabels
                    }
                }

                yAxis: QAccelPlot.Axis {
                    viewportMin: -0.6
                    viewportMax: 1.6
                    dataMin: -0.6
                    dataMax: 1.6
                    layoutSize: 70
                    baselineColor: root.palette.axisLine
                    labelColor: root.palette.axisLabel
                    labelFont: root.typography.axisLabel
                    hoverColor: root.palette.hover
                    ticker.tickCount: 3
                    ticker.subtickCount: 0
                    ticker.tickColor: root.palette.tick
                    ticker.tickLabelColor: root.palette.axisTickLabel
                    ticker.tickLabelFont: root.galleryTickLabelFont
                    ticker.tickLabelFormatter: QAccelPlot.TextTickLabelFormatter {
                        labels: ["Hollow", "Filled"]
                    }
                }

                Repeater {
                    model: root.galleryCells

                    QAccelPlot.LineCurve {
                        required property var modelData
                        xAxis: galleryPlot.xAxis
                        yAxis: galleryPlot.yAxis
                        color: root.palette.seriesPrimary
                        lineStyle: QAccelPlot.NoLine {}
                        marker.shape: modelData.shape
                        marker.filled: modelData.filled
                        marker.size: root.markerSize
                        marker.strokeWidth: root.markerStrokeWidth
                        antialiasingEnabled: root.antialiasingEnabled
                        Component.onCompleted: setData([Qt.point(modelData.x, modelData.y)])
                    }
                }
            }
        }
    }
}
