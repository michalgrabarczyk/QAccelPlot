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
    title: "QAccelPlot Bands"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Bands"
            description: "BandSeries fills the area between a low and a high value at each X, with optional edge lines. Draw the center line with a LineCurve."
        }

        ExamplePages {
            tabs: [
                { name: "forecast", title: "Forecast" },
                { name: "rollingStats", title: "Rolling statistics" }
            ]

            ForecastPage {
                palette: root.colorPalette
            }
            RollingStatsPage {
                palette: root.colorPalette
            }
        }
    }
}
