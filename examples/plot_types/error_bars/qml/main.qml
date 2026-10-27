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

    readonly property QtObject colorPalette: QAccelPlot.Colors.dark

    width: 1100
    height: 800
    visible: true
    title: "QAccelPlot Error Bars"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Error bars"
            subtitle: "ErrorBarSeries draws a capped line from a low to a high bound at each data point."
            interactions: [
                "Hover a point or an error bar on <b>Measurements</b> to read the measurement",
                "Click <b>Morph</b> or <b>Draw</b> to animate to a new sample",
                "Drag <b>Cap length</b> and <b>Line width</b>, or uncheck <b>Reference errors</b>",
                "Check <b>Horizontal</b> on <b>Grouped bars</b> to turn the bars and error bars sideways"
            ]
        }

        ExamplePages {
            tabs: [
                { name: "measurements", title: "Measurements" },
                { name: "groupedBars", title: "Grouped bars" }
            ]

            MeasurementsPage {
                colorPalette: root.colorPalette
            }
            GroupedBarsPage {
                colorPalette: root.colorPalette
            }
        }
    }
}
