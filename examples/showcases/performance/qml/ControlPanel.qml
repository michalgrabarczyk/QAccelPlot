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

// Controls for the settings of the active page.
ScrollView {
    id: root
    required property var window
    required property var colorPalette
    readonly property QtObject settings: window.activeSettings
    clip: true
    // The content follows the panel width, so it only scrolls vertically.
    contentWidth: availableWidth
    // The vertical scroll bar sits in the padding, in a lane of its own beside the content,
    // and stays visible while there is something to scroll.
    rightPadding: ScrollBar.vertical.width + 4
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.vertical.policy: contentHeight > availableHeight ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff

    function countLabel(count) {
        return count >= 1000000 ? count / 1000000 + "M" : count / 1000 + "K";
    }

    ColumnLayout {
        width: root.availableWidth
        spacing: 12

        Label {
            text: "LIVE DATASET"
            color: root.colorPalette.text
            font.bold: true
        }

        Label {
            text: root.settings.noun.charAt(0).toUpperCase() + root.settings.noun.slice(1) + " count"
            color: root.colorPalette.textSecondary
            Layout.fillWidth: true
        }

        TextField {
            text: root.settings.count.toString()
            validator: IntValidator {
                bottom: 1
                top: root.window.maximumCount
            }
            Layout.fillWidth: true
            onEditingFinished: root.settings.count = parseInt(text) || 1000
        }

        GridLayout {
            columns: 3
            Layout.fillWidth: true

            Repeater {
                model: root.window.countPresets

                delegate: Button {
                    required property var modelData
                    text: root.countLabel(modelData)
                    Layout.fillWidth: true
                    onClicked: root.settings.count = modelData
                }
            }
        }

        Label {
            text: "Up to " + root.countLabel(root.window.maximumCount) + " " + root.settings.noun + ". Only the visible tab generates data."
            color: root.colorPalette.textMuted
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Label {
            text: "DATA"
            color: root.colorPalette.text
            font.bold: true
        }

        OptionChoice {
            label: "Series"
            choices: [1, 2, 10, 100].map(count => ({
                        text: count.toString(),
                        value: count
                    }))
            value: root.settings.seriesCount
            Layout.fillWidth: true
            onSelected: value => root.settings.seriesCount = value
        }

        OptionChoice {
            label: "Ingestion API"
            stacked: true
            choices: [
                {
                    text: "setDataF (move)",
                    value: "floatMove"
                },
                {
                    text: "setDataF (raw copy)",
                    value: "floatCopy"
                },
                {
                    text: "setData (doubles, move)",
                    value: "doubleMove"
                },
                {
                    text: "postData (move)",
                    value: "floatPost"
                }
            ]
            value: root.settings.ingestion
            Layout.fillWidth: true
            onSelected: value => root.settings.ingestion = value
        }

        OptionSwitch {
            label: "Hover hit-testing"
            checked: root.settings.hoverEnabled
            Layout.fillWidth: true
            onToggled: checked => root.settings.hoverEnabled = checked
        }

        // The active page's own options.
        Loader {
            sourceComponent: root.window.activePage === "pointCloud" ? pointCloudOptions : root.window.activePage === "rectangleSeries" ? rectangleSeriesOptions : root.window.activePage === "barSeries" ? barSeriesOptions : lineCurveOptions
            Layout.fillWidth: true
        }

        Component {
            id: lineCurveOptions

            LineCurveOptions {
                settings: root.window.lineCurveSettings
            }
        }

        Component {
            id: pointCloudOptions

            PointCloudOptions {
                settings: root.window.pointCloudSettings
            }
        }

        Component {
            id: rectangleSeriesOptions

            RectangleSeriesOptions {
                settings: root.window.rectangleSeriesSettings
            }
        }

        Component {
            id: barSeriesOptions

            BarSeriesOptions {
                settings: root.window.barSeriesSettings
            }
        }

        Button {
            text: "Reset to defaults"
            Layout.fillWidth: true
            onClicked: root.window.resetActiveSettings()
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
