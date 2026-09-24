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
    height: 860
    visible: true
    title: "Custom Axes Gallery"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Custom axes gallery"
        }

        ExamplePages {
            tabs: [
                { name: "dualScale", title: "Dual scale" },
                { name: "multiRate", title: "Multi-rate" },
                { name: "sharedTime", title: "Shared time" }
            ]

            SpectrumPage {
                palette: colorPalette
            }
            MultiRatePage {
                palette: colorPalette
            }
            EcgPanelsPage {
                palette: colorPalette
            }
        }
    }
}
