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

    property bool isDark: true
    readonly property QtObject colorPalette: isDark ? QAccelPlot.Colors.dark : QAccelPlot.Colors.light

    width: 880
    height: 1120
    visible: true
    title: "QAccelPlot Styling and Transitions"
    color: colorPalette.window
    Material.theme: isDark ? Material.Dark : Material.Light
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Styling and transitions"
            description: "Explore curve styling, data-driven fills and gradients, and animated data transitions."
            colorPalette: window.colorPalette

            Switch {
                text: "Dark theme"
                checked: window.isDark
                onToggled: window.isDark = checked
            }
        }

        ExamplePages {
            tabs: [
                { name: "styling", title: "Styling" },
                { name: "fillsGradients", title: "Fills and Gradients" },
                { name: "transitions", title: "Transitions" },
                { name: "gaps", title: "Gaps" },
                { name: "markers", title: "Markers" }
            ]

            StylingPage {
                palette: colorPalette
            }
            FillsGradientsPage {
                palette: colorPalette
            }
            TransitionsPage {
                palette: colorPalette
            }
            GapsPage {
                palette: colorPalette
            }
            MarkersPage {
                palette: colorPalette
            }
        }
    }
}
