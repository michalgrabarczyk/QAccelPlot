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
    property real markerSize: 6
    property real markerStrokeWidth: 1.5
    property real markerOpacity: 0.25
    property bool antialiasingEnabled: true
    property int selectedColumn: 0
    property bool selectedFilled: true

    // The gallery's shape names are its only labels, so they are drawn larger than other tick labels.
    readonly property font galleryTickLabelFont: Qt.font({
        pixelSize: 14,
        weight: Font.Normal
    })

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

    readonly property string scatterMarkerName: (selectedFilled ? "" : "Hollow ") + shapes[selectedColumn].name

    // Deterministic two-arm spiral with Gaussian scatter, so every run and screenshot shows the same points.
    function spiralCloud(count, seed) {
        let state = seed;
        function random() {
            state = (Math.imul(state, 1664525) + 1013904223) >>> 0;
            return (state + 0.5) / 4294967296;
        }
        function gaussian() {
            return Math.sqrt(-2 * Math.log(random())) * Math.cos(2 * Math.PI * random());
        }
        const points = [];
        for (let i = 0; i < count; ++i) {
            const t = random();
            const angle = 2.5 * Math.PI * t + Math.PI * (i % 2);
            const radius = 0.2 + 5.5 * t;
            const scatter = 0.1 + 0.25 * t;
            points.push(Qt.point(radius * Math.cos(angle) + scatter * gaussian(), 0.6 * radius * Math.sin(angle) + scatter * gaussian()));
        }
        return points;
    }

    width: 900
    height: 900
    visible: true
    title: "QAccelPlot Markers"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Markers"
            description: "Click a shape in the gallery to draw the scatter cloud with it. Lower the opacity to see where overlapping markers pile up."
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            // Value labels keep the width of their widest value so the sliders do not shift while dragging.
            Label {
                id: sizeLabel
                text: "Size " + window.markerSize.toFixed(1) + " px"
                Layout.preferredWidth: Math.ceil(widestSizeText.advanceWidth)

                TextMetrics {
                    id: widestSizeText
                    font: sizeLabel.font
                    text: "Size 88.8 px"
                }
            }
            Slider {
                from: 0.1
                to: 12
                stepSize: 0.1
                value: window.markerSize
                Layout.preferredWidth: 100
                onMoved: window.markerSize = value
            }
            Label {
                id: outlineLabel
                text: "Outline " + window.markerStrokeWidth.toFixed(1) + " px"
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
                value: window.markerStrokeWidth
                Layout.preferredWidth: 100
                onMoved: window.markerStrokeWidth = value
            }
            Label {
                id: opacityLabel
                text: "Opacity " + Math.round(window.markerOpacity * 100) + " %"
                Layout.preferredWidth: Math.ceil(widestOpacityText.advanceWidth)

                TextMetrics {
                    id: widestOpacityText
                    font: opacityLabel.font
                    text: "Opacity 100 %"
                }
            }
            Slider {
                from: 0.1
                to: 1
                stepSize: 0.05
                value: window.markerOpacity
                Layout.preferredWidth: 100
                onMoved: window.markerOpacity = value
            }
            Switch {
                text: "Antialiasing"
                checked: window.antialiasingEnabled
                onToggled: window.antialiasingEnabled = checked
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
                id: scatterPlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                grid.subGridVisible: false
                legend: QAccelPlot.Legend {
                    series: scatterPlot.series
                }

                xAxis: ExampleAxis {
                    viewportMin: -8
                    viewportMax: 8
                    dataMin: -8
                    dataMax: 8
                    label: "X"
                }

                yAxis: ExampleAxis {
                    viewportMin: -5
                    viewportMax: 5
                    dataMin: -5
                    dataMax: 5
                    axisTitlePadding: 40
                    layoutSize: 60
                    label: "Y"
                }

                QAccelPlot.LineCurve {
                    name: window.scatterMarkerName + ", 4 000 points"
                    xAxis: scatterPlot.xAxis
                    yAxis: scatterPlot.yAxis
                    color: colorPalette.seriesPrimary
                    opacity: window.markerOpacity
                    lineStyle: QAccelPlot.NoLine {}
                    marker.shape: window.shapes[window.selectedColumn].shape
                    marker.filled: window.selectedFilled
                    marker.size: window.markerSize
                    marker.strokeWidth: window.markerStrokeWidth
                    antialiasingEnabled: window.antialiasingEnabled
                    Component.onCompleted: setData(window.spiralCloud(4000, 7))
                }
            }

            QAccelPlot.Plot {
                id: galleryPlot
                Layout.fillWidth: true
                Layout.preferredHeight: 300
                legendVisible: false
                grid.subGridVisible: false

                onMousePressed: event => {
                    if (event.button !== Qt.LeftButton || !isInsidePlotArea(event.x, event.y)) {
                        return;
                    }
                    const column = Math.round(pixelToDataX(event.x));
                    const row = Math.round(pixelToDataY(event.y));
                    if (column < 0 || column >= window.shapes.length || row < 0 || row > 1) {
                        return;
                    }
                    window.selectedColumn = column;
                    window.selectedFilled = row === 1;
                    event.accept();
                }

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }

                xAxis: ExampleAxis {
                    viewportMin: -0.6
                    viewportMax: window.shapes.length - 0.4
                    dataMin: -0.6
                    dataMax: window.shapes.length - 0.4
                    layoutSize: 118
                    axisTitlePadding: 100
                    label: "Shape"
                    // One tick per shape: range / tickCount must stay at or below 1.
                    ticker.tickCount: window.shapes.length + 1
                    ticker.subtickCount: 0
                    ticker.tickLabelRotation: -40
                    ticker.tickLabelPadding: 25
                    ticker.tickLabelFont: window.galleryTickLabelFont
                    ticker.tickLabelFormatter: QAccelPlot.TextTickLabelFormatter {
                        labels: window.shapeLabels
                    }
                }

                yAxis: ExampleAxis {
                    viewportMin: -0.6
                    viewportMax: 1.6
                    dataMin: -0.6
                    dataMax: 1.6
                    layoutSize: 70
                    ticker.tickCount: 3
                    ticker.subtickCount: 0
                    ticker.tickLabelFont: window.galleryTickLabelFont
                    ticker.tickLabelFormatter: QAccelPlot.TextTickLabelFormatter {
                        labels: ["Hollow", "Filled"]
                    }
                }

                Repeater {
                    model: window.galleryCells

                    QAccelPlot.LineCurve {
                        required property var modelData
                        xAxis: galleryPlot.xAxis
                        yAxis: galleryPlot.yAxis
                        color: colorPalette.seriesPrimary
                        lineStyle: QAccelPlot.NoLine {}
                        marker.shape: modelData.shape
                        marker.filled: modelData.filled
                        marker.size: window.markerSize
                        marker.strokeWidth: window.markerStrokeWidth
                        antialiasingEnabled: window.antialiasingEnabled
                        Component.onCompleted: setData([Qt.point(modelData.x, modelData.y)])
                    }
                }

                // The selection frame is itself a marker: a large hollow square around the chosen cell.
                QAccelPlot.LineCurve {
                    readonly property point cell: Qt.point(window.selectedColumn, window.selectedFilled ? 1 : 0)
                    xAxis: galleryPlot.xAxis
                    yAxis: galleryPlot.yAxis
                    color: colorPalette.seriesSecondary
                    lineStyle: QAccelPlot.NoLine {}
                    marker.shape: QAccelPlot.LineCurve.Square
                    marker.filled: false
                    marker.size: window.markerSize + 8
                    marker.strokeWidth: 2
                    antialiasingEnabled: window.antialiasingEnabled
                    onCellChanged: setData([cell])
                    Component.onCompleted: setData([cell])
                }
            }
        }
    }
}
