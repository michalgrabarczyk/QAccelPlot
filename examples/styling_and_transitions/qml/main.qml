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

    Typography {
        id: typography
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 1

                Label {
                    text: "Styling and transitions"
                    color: colorPalette.text
                    font.bold: true
                    font.pixelSize: 20
                }

                Label {
                    text: "Explore curve styling, data-driven fills and gradients, and animated data transitions."
                    color: colorPalette.textSecondary
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Switch {
                text: "Dark theme"
                checked: window.isDark
                Material.foreground: colorPalette.text
                Material.accent: colorPalette.materialAccent
                onToggled: window.isDark = checked
            }
        }

        TabBar {
            id: tabBar
            Layout.fillWidth: true

            TabButton {
                text: "Styling"
            }
            TabButton {
                text: "Fills and Gradients"
            }
            TabButton {
                text: "Transitions"
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            StylingPage {
                palette: colorPalette
                typography: typography
            }
            FillsGradientsPage {
                palette: colorPalette
                typography: typography
            }
            TransitionsPage {
                palette: colorPalette
                typography: typography
            }
        }
    }
}
