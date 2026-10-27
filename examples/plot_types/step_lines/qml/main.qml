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
    title: "QAccelPlot Step Lines"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Step lines"
            subtitle: "LineCurve.stepMode draws a line as steps: each value holds until the line jumps to the next one."
            interactions: [
                "Toggle <b>Markers</b>, <b>Fill</b>, and <b>Dashed</b> on <b>Step modes</b>; the fill and dashes follow the steps",
                "Hover the plot on <b>Live set point</b> to read the value each step holds",
                "Check <b>Pause</b> to stop the live data"
            ]
        }

        ExamplePages {
            tabs: [
                { name: "modes", title: "Step modes" },
                { name: "setPoint", title: "Live set point" }
            ]

            ModesPage {
                colorPalette: root.colorPalette
            }
            SetPointPage {
                colorPalette: root.colorPalette
            }
        }
    }
}
