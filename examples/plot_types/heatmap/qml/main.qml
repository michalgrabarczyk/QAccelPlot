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
    readonly property var presets: [
        { name: "Viridis", preset: QAccelPlot.Colormap.Viridis },
        { name: "Plasma", preset: QAccelPlot.Colormap.Plasma },
        { name: "Inferno", preset: QAccelPlot.Colormap.Inferno },
        { name: "Magma", preset: QAccelPlot.Colormap.Magma },
        { name: "Grayscale", preset: QAccelPlot.Colormap.Grayscale },
        { name: "Rainbow", preset: QAccelPlot.Colormap.Rainbow }
    ]

    width: 1200
    height: 800
    visible: true
    title: "Heatmap Example"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        ExampleHeader {
            title: "Heatmap"
            description: "HeatmapSeries colors a grid of values. Interference: two circular waves on a 400 × 300 grid, keyed by a histogram of the cells in view whose amplitude axis sets the color range. Log-spaced bins: a frequency sweep in rows of equal frequency ratio, with ySpacing: HeatmapSeries.Log on a log axis. Hover a cell to read it, drag or wheel to pan and zoom, and drag or wheel the color bar or the histogram to change the color range."

            Label {
                text: "Colormap"
                color: colorPalette.text
            }
            ComboBox {
                id: presetBox
                model: window.presets.map(entry => entry.name)
                Layout.preferredWidth: 140
                Material.background: colorPalette.plotArea
            }
            CheckBox {
                id: reversedBox
                text: "Reversed"
            }
        }

        ExamplePages {
            tabs: [
                { name: "interference", title: "Interference" },
                { name: "logBins", title: "Log-spaced bins" }
            ]

            InterferencePage {
                colorPalette: window.colorPalette
                preset: window.presets[presetBox.currentIndex].preset
                reversed: reversedBox.checked
            }
            LogBinsPage {
                colorPalette: window.colorPalette
                preset: window.presets[presetBox.currentIndex].preset
                reversed: reversedBox.checked
            }
        }
    }
}
