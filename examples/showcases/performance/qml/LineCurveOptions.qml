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

// Controls for the LineCurve page's own settings.
ColumnLayout {
    id: root

    required property QtObject settings

    spacing: 12

    OptionChoice {
        label: "NaN gaps"
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
        value: root.settings.gapFraction
        Layout.fillWidth: true
        onSelected: value => root.settings.gapFraction = value
    }

    OptionSwitch {
        label: "Connect across gaps"
        checked: root.settings.connectGaps
        Layout.fillWidth: true
        onToggled: checked => root.settings.connectGaps = checked
    }

    Label {
        text: "RENDERING"
        color: QAccelPlot.Colors.dark.text
        font.bold: true
    }

    OptionChoice {
        label: "Line width"
        choices: [1, 3, 10].map(width => ({
                    text: width + " px",
                    value: width
                }))
        value: root.settings.lineWidth
        Layout.fillWidth: true
        onSelected: value => root.settings.lineWidth = value
    }

    OptionChoice {
        label: "Line style"
        choices: [
            {
                text: "Solid",
                value: "solid"
            },
            {
                text: "Dash",
                value: "dash"
            },
            {
                text: "No line",
                value: "none"
            }
        ]
        value: root.settings.lineStyle
        Layout.fillWidth: true
        onSelected: value => root.settings.lineStyle = value
    }

    OptionChoice {
        label: "Markers"
        choices: [
            {
                text: "None",
                value: QAccelPlot.LineCurve.None
            },
            {
                text: "Circle",
                value: QAccelPlot.LineCurve.Circle
            },
            {
                text: "Star",
                value: QAccelPlot.LineCurve.Star
            },
            {
                text: "Pixel",
                value: QAccelPlot.LineCurve.Pixel
            }
        ]
        value: root.settings.markerShape
        Layout.fillWidth: true
        onSelected: value => root.settings.markerShape = value
    }

    OptionChoice {
        label: "Marker size"
        choices: [2, 4, 8].map(size => ({
                    text: size + " px",
                    value: size
                }))
        value: root.settings.markerSize
        Layout.fillWidth: true
        onSelected: value => root.settings.markerSize = value
    }

    OptionChoice {
        label: "Effect"
        choices: [
            {
                text: "None",
                value: "none"
            },
            {
                text: "Stroke",
                value: "stroke"
            },
            {
                text: "Fill",
                value: "fill"
            }
        ]
        value: root.settings.effect
        Layout.fillWidth: true
        onSelected: value => root.settings.effect = value
    }

    OptionSwitch {
        label: "Antialiasing"
        checked: root.settings.antialiasing
        Layout.fillWidth: true
        onToggled: checked => root.settings.antialiasing = checked
    }

    OptionChoice {
        label: "Feather"
        choices: [1, 2, 4].map(feather => ({
                    text: feather + " px",
                    value: feather
                }))
        value: root.settings.antialiasingFeather
        Layout.fillWidth: true
        onSelected: value => root.settings.antialiasingFeather = value
    }
}
