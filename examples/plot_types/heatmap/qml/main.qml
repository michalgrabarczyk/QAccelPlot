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
    readonly property var presets: [
        { name: "Viridis", preset: QAccelPlot.Colormap.Viridis },
        { name: "Plasma", preset: QAccelPlot.Colormap.Plasma },
        { name: "Inferno", preset: QAccelPlot.Colormap.Inferno },
        { name: "Magma", preset: QAccelPlot.Colormap.Magma },
        { name: "Grayscale", preset: QAccelPlot.Colormap.Grayscale },
        { name: "Rainbow", preset: QAccelPlot.Colormap.Rainbow }
    ]

    width: 1200
    height: 800
    visible: true
    title: "Heatmap Example"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        ExampleHeader {
            title: "Heatmap"
            description: "Interference of two circular waves on a 400 × 300 grid. Hover a cell to read it, drag or wheel to pan and zoom, and drag or wheel the color bar to change its range."

            Label {
                text: "Colormap"
                color: colorPalette.text
            }
            ComboBox {
                id: presetBox
                model: window.presets.map(entry => entry.name)
                Layout.preferredWidth: 140
                Material.background: colorPalette.plotArea
            }
            CheckBox {
                id: reversedBox
                text: "Reversed"
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
                viewportMin: -10
                viewportMax: 10
                label: "x (m)"
            }

            yAxis: ExampleAxis {
                viewportMin: -7.5
                viewportMax: 7.5
                label: "y (m)"
            }

            // Filled from C++ with setValues(), 400 columns by 300 rows.
            QAccelPlot.HeatmapSeries {
                id: heatmap
                objectName: "heatmap"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                xMin: -10
                xMax: 10
                yMin: -7.5
                yMax: 7.5
                colormap: QAccelPlot.Colormap {
                    preset: window.presets[presetBox.currentIndex].preset
                    reversed: reversedBox.checked
                }
            }

            Item {
                id: overlay
                x: plot.plotRect.x
                y: plot.plotRect.y
                width: plot.plotRect.width
                height: plot.plotRect.height
                clip: true

                HoverHandler {
                    id: hover
                }

                Rectangle {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 10
                    width: amplitudeBar.width + 20
                    height: amplitudeBar.height + 16
                    radius: 4
                    color: colorPalette.legendBackground
                    border.color: colorPalette.legendBorder

                    QAccelPlot.ColorBar {
                        id: amplitudeBar
                        x: 10
                        y: 8
                        height: 260
                        barThickness: 24
                        tickLabelWidth: 30
                        series: heatmap
                        interactive: true
                        label: "Amplitude"
                        labelColor: colorPalette.axisLabel
                        borderColor: hovered ? colorPalette.hover : colorPalette.axisLine
                        ticker.tickColor: colorPalette.tick
                        ticker.tickLabelColor: colorPalette.axisTickLabel
                        ticker.tickCount: 8
                    }
                }

                Rectangle {
                    id: readout

                    // The record of the cell under the pointer; the overlay shares the series' coordinates.
                    readonly property var record: hover.hovered ? heatmap.inspection.recordAtPosition(hover.point.position) : null

                    visible: record !== null && record.valid && !amplitudeBar.hovered
                    x: Math.min(hover.point.position.x + 14, overlay.width - width)
                    y: Math.max(hover.point.position.y - height - 10, 0)
                    width: readoutText.implicitWidth + 12
                    height: readoutText.implicitHeight + 8
                    radius: 3
                    color: colorPalette.tooltipBackground

                    Text {
                        id: readoutText
                        anchors.centerIn: parent
                        color: colorPalette.tooltipText
                        font.pixelSize: 11
                        text: readout.visible ? "(" + readout.record.fields.x.toFixed(2) + ", " + readout.record.fields.y.toFixed(2) + ")  amplitude " + readout.record.fields.value.toFixed(3) : ""
                    }
                }
            }
        }
    }
}
