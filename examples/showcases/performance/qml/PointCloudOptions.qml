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

// Controls for the PointCloud page's own settings.
ColumnLayout {
    id: root

    required property QtObject settings
    // The raw-array setters take no values.
    readonly property bool valuesAvailable: settings.ingestion !== "floatNoRangeCopy"

    spacing: 12

    OptionSwitch {
        label: "Per-point values"
        checked: root.settings.values && root.valuesAvailable
        enabled: root.valuesAvailable
        Layout.fillWidth: true
        onToggled: checked => root.settings.values = checked
    }

    OptionSwitch {
        label: "Fixed colormap range"
        checked: root.settings.fixedColormapRange
        enabled: root.settings.values && root.valuesAvailable
        Layout.fillWidth: true
        onToggled: checked => root.settings.fixedColormapRange = checked
    }

    OptionChoice {
        label: "Invalid points"
        choices: [
            {
                text: "None",
                value: 0
            },
            {
                text: "1 %",
                value: 0.01
            },
            {
                text: "10 %",
                value: 0.1
            }
        ]
        value: root.settings.invalidFraction
        Layout.fillWidth: true
        onSelected: value => root.settings.invalidFraction = value
    }

    Label {
        text: "RENDERING"
        color: QAccelPlot.Colors.dark.text
        font.bold: true
    }

    OptionChoice {
        label: "Marker shape"
        choices: [
            {
                text: "Circle",
                value: QAccelPlot.PointCloud.Circle
            },
            {
                text: "Square",
                value: QAccelPlot.PointCloud.Square
            },
            {
                text: "Star",
                value: QAccelPlot.PointCloud.Star
            },
            {
                text: "Pixel",
                value: QAccelPlot.PointCloud.Pixel
            }
        ]
        value: root.settings.markerShape
        Layout.fillWidth: true
        onSelected: value => root.settings.markerShape = value
    }

    OptionChoice {
        label: "Marker size"
        choices: [1, 2, 5, 10, 20].map(size => ({
                    text: size + " px",
                    value: size
                }))
        value: root.settings.markerSize
        Layout.fillWidth: true
        onSelected: value => root.settings.markerSize = value
    }

    OptionSwitch {
        label: "Filled markers"
        checked: root.settings.markerFilled
        Layout.fillWidth: true
        onToggled: checked => root.settings.markerFilled = checked
    }

    OptionChoice {
        label: "Outline width"
        choices: [1, 2].map(width => ({
                    text: width + " px",
                    value: width
                }))
        value: root.settings.markerStrokeWidth
        enabled: !root.settings.markerFilled
        Layout.fillWidth: true
        onSelected: value => root.settings.markerStrokeWidth = value
    }

    OptionSwitch {
        label: "Antialiasing"
        checked: root.settings.antialiasing
        Layout.fillWidth: true
        onToggled: checked => root.settings.antialiasing = checked
    }
}
