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

    width: 1000
    height: 720
    visible: true
    title: "QAccelPlot Tick Formats"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Tick formats"
            subtitle: "Numeric coordinates labeled as dates and times, powers of ten, or named categories."
            interactions: [
                "Choose a <b>Label format</b> on <b>Date and time</b>",
                "Toggle <b>Logarithmic axis</b> on <b>Logarithmic</b> to compare it with a linear scale"
            ]
        }

        ExamplePages {
            tabs: [
                { name: "dateTime", title: "Date and time" },
                { name: "logarithmic", title: "Logarithmic" },
                { name: "categories", title: "Categories" }
            ]

            DateTimePage {
                colorPalette: window.colorPalette
            }
            LogScalePage {
                colorPalette: window.colorPalette
            }
            CategoryPage {
                colorPalette: window.colorPalette
            }
        }
    }
}
