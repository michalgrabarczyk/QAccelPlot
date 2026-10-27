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

// Bold title, a one-sentence subtitle saying what the example shows, and up to
// four one-line bullets saying what you can do with it. Child items are placed
// to the right of the text.
RowLayout {
    id: root

    property alias title: titleLabel.text
    property alias subtitle: subtitleLabel.text
    // Array of strings, one per bullet. Bold control and key names with <b>.
    property var interactions: []
    property QtObject colorPalette: QAccelPlot.Colors.dark

    Layout.fillWidth: true
    spacing: 8

    ColumnLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop
        spacing: 1

        Label {
            id: titleLabel
            color: root.colorPalette.text
            font.bold: true
            font.pixelSize: 20
        }

        Label {
            id: subtitleLabel
            visible: text !== ""
            color: root.colorPalette.text
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        ColumnLayout {
            visible: root.interactions.length > 0
            spacing: 1
            Layout.topMargin: 3
            Layout.fillWidth: true

            Repeater {
                model: root.interactions

                Label {
                    required property string modelData

                    text: "•  " + modelData
                    textFormat: Text.StyledText
                    color: root.colorPalette.textSecondary
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }
    }
}
