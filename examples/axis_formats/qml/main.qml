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
    title: "QAccelPlot Axis Formats"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Axis formats"
            description: "Format numeric coordinates as time, logarithmic powers, or named categories without changing the underlying series data."
        }

        ExamplePages {
            tabs: [
                { name: "dateTime", title: "Date and time" },
                { name: "logarithmic", title: "Logarithmic" },
                { name: "categories", title: "Categories" }
            ]

            DateTimePage {
                palette: colorPalette
            }
            LogScalePage {
                palette: colorPalette
            }
            CategoryPage {
                palette: colorPalette
            }
        }
    }
}
