//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QAccelPlot as QAccelPlot

Window {
    id: window

    readonly property QtObject colorPalette: QAccelPlot.Colors.dark

    width: 1000
    height: 720
    visible: true
    title: "QAccelPlot Unit Tick Labels"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Unit tick labels"
            subtitle: "UnitTickLabelFormatter writing SI unit prefixes that follow the visible range, in the axis label or on the tick labels."
            interactions: [
                "Zoom in on an axis to watch its unit prefix change",
                "Choose a <b>Unit format</b> on <b>Axis label</b>",
                "Choose the <b>Allowed prefixes</b> on <b>Tick labels</b>",
                "Toggle <b>Logarithmic axis</b> on <b>Per tick label</b>"
            ]
        }

        ExamplePages {
            tabs: [
                { name: "axisLabel", title: "Axis label" },
                { name: "tickLabels", title: "Tick labels" },
                { name: "perTick", title: "Per tick label" }
            ]

            AxisLabelPage {
                colorPalette: window.colorPalette
            }
            TickLabelsPage {
                colorPalette: window.colorPalette
            }
            PerTickPage {
                colorPalette: window.colorPalette
            }
        }
    }
}
