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
