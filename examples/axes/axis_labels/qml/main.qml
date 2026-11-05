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
    title: "QAccelPlot Axis Labels"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Axis labels"
            subtitle: "Axis labels placed with alignment, rotation, and padding, or replaced by an Axis label delegate."
            interactions: [
                "Choose an <b>Alignment</b> and drag <b>Padding</b> on <b>Painted label</b>",
                "Clear <b>Auto rotation</b> and drag <b>Rotation</b> on <b>Painted label</b>"
            ]
        }

        ExamplePages {
            tabs: [
                { name: "painted", title: "Painted label" },
                { name: "delegate", title: "Label delegate" }
            ]

            PaintedLabelPage {
                colorPalette: window.colorPalette
            }
            LabelDelegatePage {
                colorPalette: window.colorPalette
            }
        }
    }
}
