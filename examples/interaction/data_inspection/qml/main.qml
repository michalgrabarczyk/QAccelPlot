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
    id: root

    // Set for screenshots: the cursor is placed from code instead of following the mouse.
    property bool pinnedCursor: false
    readonly property QtObject colorPalette: QAccelPlot.Colors.dark

    width: 1100
    height: 700
    visible: true
    title: "Data Inspection Example"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Data inspection"
            subtitle: "Two noisy signals, one million samples in total, with a crosshair, a tooltip, and region selection."
            interactions: [
                "Hover to read the minimum … maximum and count of the samples under the cursor",
                "Uncheck <b>Min … max under the cursor</b> to read the nearest sample alone",
                "Choose how <b>Inspect</b> finds samples: by X, by Y, or the nearest point",
                "Shift + drag to select a region shaped by <b>Select</b> and read its statistics; <b>Escape</b> clears it"
            ]
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            CheckBox {
                id: summariesToggle

                text: "Min … max under the cursor"
                checked: true
            }

            CheckBox {
                id: snapping

                text: "Snap crosshair"
            }

            Label {
                text: "Inspect"
            }

            ComboBox {
                id: queryMode

                Layout.preferredWidth: 200
                Material.background: colorPalette.plotArea
                model: ["Compare by X", "Compare by Y", "Pick nearest point"]
            }

            Label {
                text: "Select"
            }

            ComboBox {
                id: selectionMode

                Layout.preferredWidth: 160
                Material.background: colorPalette.plotArea
                model: ["Box", "X range", "Y range"]
            }

            Item {
                Layout.fillWidth: true
            }
        }

        QAccelPlot.Plot {
            id: plotView

            Layout.fillWidth: true
            Layout.fillHeight: true

            xAxis: ExampleAxis {
                viewportMin: 0
                viewportMax: 50
                label: "Time (s)"
            }

            yAxis: ExampleAxis {
                viewportMin: -2
                viewportMax: 5
                label: "Amplitude"
            }

            QAccelPlot.LineCurve {
                objectName: "signalA"
                name: "Signal A"
                xAxis: plotView.xAxis
                yAxis: plotView.yAxis
                color: colorPalette.seriesPrimary
            }

            QAccelPlot.LineCurve {
                objectName: "signalB"
                name: "Signal B"
                xAxis: plotView.xAxis
                yAxis: plotView.yAxis
                color: colorPalette.seriesSecondary
            }

            QAccelPlot.PlotInspector {
                id: inspector

                plot: plotView
                mode: [QAccelPlot.PlotInspector.NearestX, QAccelPlot.PlotInspector.NearestY, QAccelPlot.PlotInspector.NearestXY][queryMode.currentIndex]
                summaries: summariesToggle.checked
                snapToDataPoint: snapping.checked
                enabled: !selection.selecting
                followPointer: !root.pinnedCursor
                cursorX: 23.4
                cursorY: 2.6

                // Stacked in declaration order: the tooltip is drawn on top.
                QAccelPlot.Crosshair {
                    axisLabels: true
                }

                QAccelPlot.InspectionMarkers {}

                QAccelPlot.InspectionTooltip {}
            }

            QAccelPlot.SelectionTool {
                id: selection

                plot: plotView
                mode: [QAccelPlot.SelectionTool.Box, QAccelPlot.SelectionTool.XRange, QAccelPlot.SelectionTool.YRange][selectionMode.currentIndex]
            }
        }

        RowLayout {
            spacing: 24

            Label {
                visible: !selection.hasSelection
                color: colorPalette.textMuted
                text: "No selection"
            }

            Repeater {
                model: selection.model

                Label {
                    required property string seriesName
                    required property int summaryCount
                    required property string minimumText
                    required property string maximumText
                    required property string meanText

                    text: summaryCount > 0 ? seriesName + ": " + summaryCount + " samples, " + minimumText + " … " + maximumText + ", mean " + meanText :
                                             seriesName + ": no samples"
                }
            }
        }
    }
}
