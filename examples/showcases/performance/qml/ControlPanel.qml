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

ScrollView {
    id: root
    required property var window
    required property var palette
    clip: true

    ColumnLayout {
        width: root.availableWidth
        spacing: 12

        Label {
            text: "LIVE DATASET"
            color: root.palette.text
            font.bold: true
        }

        Label {
            text: "Data point count"
            color: root.palette.textSecondary
            Layout.fillWidth: true
        }

        TextField {
            text: root.window.pointCount.toString()
            validator: IntValidator {
                bottom: 1
                top: 10000000
            }
            Layout.fillWidth: true
            onEditingFinished: root.window.pointCount = parseInt(text) || 1000
        }

        GridLayout {
            columns: 3
            Layout.fillWidth: true

            Repeater {
                model: [1000, 10000, 100000, 300000, 500000, 1000000, 2000000, 5000000]

                delegate: Button {
                    required property var modelData
                    text: modelData >= 1000000 ? modelData / 1000000 + "M" : modelData / 1000 + "K"
                    Layout.fillWidth: true
                    onClicked: root.window.pointCount = modelData
                }
            }
        }

        Rectangle {
            color: root.palette.outline
            height: 1
            opacity: 0.4
            Layout.fillWidth: true
        }

        Switch {
            text: "Annotate with rectangles"
            checked: root.window.rectanglesVisible
            Layout.fillWidth: true
            onToggled: root.window.rectanglesVisible = checked
        }

        Label {
            text: "Rectangle count"
            color: root.palette.textSecondary
            enabled: root.window.rectanglesVisible
            Layout.fillWidth: true
        }

        TextField {
            text: root.window.rectangleCount.toString()
            validator: IntValidator {
                bottom: 1
                top: 10000000
            }
            enabled: root.window.rectanglesVisible
            Layout.fillWidth: true
            onEditingFinished: root.window.rectangleCount = parseInt(text) || 1000
        }

        GridLayout {
            columns: 3
            enabled: root.window.rectanglesVisible
            Layout.fillWidth: true

            Repeater {
                model: [1000, 10000, 100000, 300000, 500000, 1000000]

                delegate: Button {
                    required property var modelData
                    text: modelData >= 1000000 ? "1M" : modelData / 1000 + "K"
                    Layout.fillWidth: true
                    onClicked: root.window.rectangleCount = modelData
                }
            }
        }

        Label {
            text: "Rectangles use fixed data-space dimensions and sample the same signal as the curve."
            color: root.window.rectanglesVisible ? root.palette.textSecondary : root.palette.textMuted
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
