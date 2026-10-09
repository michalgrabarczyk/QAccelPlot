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

// Controls for the BarSeries page's own settings.
ColumnLayout {
    id: root

    required property QtObject settings
    // The raw-array setters take no categories.
    readonly property bool categoriesAvailable: settings.ingestion !== "floatCopy"

    spacing: 12

    OptionChoice {
        label: "Categories"
        choices: [
            {
                text: "Off",
                value: 0
            },
            {
                text: "4 colors",
                value: 4
            },
            {
                text: "16 colors",
                value: 16
            }
        ]
        value: root.categoriesAvailable ? root.settings.categoryCount : 0
        enabled: root.categoriesAvailable
        Layout.fillWidth: true
        onSelected: value => root.settings.categoryCount = value
    }

    Label {
        text: "RENDERING"
        color: QAccelPlot.Colors.dark.text
        font.bold: true
    }

    OptionChoice {
        label: "Bar width"
        choices: [0.5, 0.8, 1].map(width => ({
                    text: width * 100 + " %",
                    value: width
                }))
        value: root.settings.barWidth
        Layout.fillWidth: true
        onSelected: value => root.settings.barWidth = value
    }

    OptionChoice {
        label: "Border width"
        choices: [0, 1, 2].map(width => ({
                    text: width === 0 ? "None" : width + " px",
                    value: width
                }))
        value: root.settings.borderWidth
        Layout.fillWidth: true
        onSelected: value => root.settings.borderWidth = value
    }

    OptionChoice {
        label: "Minimum width"
        choices: [0, 1].map(width => ({
                    text: width + " px",
                    value: width
                }))
        value: root.settings.minimumWidth
        Layout.fillWidth: true
        onSelected: value => root.settings.minimumWidth = value
    }

    OptionSwitch {
        label: "Hover highlight"
        checked: root.settings.hoverHighlight
        enabled: root.settings.hoverEnabled
        Layout.fillWidth: true
        onToggled: checked => root.settings.hoverHighlight = checked
    }
}
