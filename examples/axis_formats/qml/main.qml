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

    Typography {
        id: typography
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Label {
            text: "Axis formats"
            color: colorPalette.text
            font.bold: true
            font.pixelSize: 20
        }

        Label {
            text: "Format numeric coordinates as time, logarithmic powers, or named categories without changing the underlying series data."
            color: colorPalette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        TabBar {
            id: tabs
            Layout.fillWidth: true

            TabButton {
                text: "Date and time"
            }
            TabButton {
                text: "Logarithmic"
            }
            TabButton {
                text: "Categories"
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabs.currentIndex

            DateTimePage {
                palette: colorPalette
                typography: typography
            }
            LogScalePage {
                palette: colorPalette
                typography: typography
            }
            CategoryPage {
                palette: colorPalette
                typography: typography
            }
        }
    }
}
