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
            description: "ErrorBarSeries draws a line from a low to a high bound at each data point, capped at both ends. Draw the values with a PointCloud, LineCurve, or BarSeries."
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
