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

    Typography {
        id: typography
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Label {
            text: "Custom axes gallery"
            color: colorPalette.text
            font.bold: true
            font.pixelSize: 18
        }

        TabBar {
            id: tabs
            Layout.fillWidth: true
            TabButton {
                text: "Dual scale"
            }
            TabButton {
                text: "Multi-rate"
            }
            TabButton {
                text: "Shared time"
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabs.currentIndex

            SpectrumPage {
                palette: colorPalette
                typography: typography
            }
            MultiRatePage {
                palette: colorPalette
                typography: typography
            }
            EcgPanelsPage {
                palette: colorPalette
                typography: typography
            }
        }
    }
}
