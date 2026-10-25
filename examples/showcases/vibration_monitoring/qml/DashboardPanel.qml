//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// A titled card that holds one plot of the dashboard.
Rectangle {
    id: panel

    required property QtObject colorPalette
    property alias title: titleLabel.text
    property alias detail: detailLabel.text
    default property alias content: body.data

    color: colorPalette.axesArea
    border.color: colorPalette.plotBorder
    radius: 4

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 2

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 4
            Layout.rightMargin: 4
            spacing: 8

            Label {
                id: titleLabel
                color: panel.colorPalette.text
                font.pixelSize: 13
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: !detailLabel.visible
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                id: detailLabel
                visible: text !== ""
                color: panel.colorPalette.textSecondary
                font.pixelSize: 12
                elide: Text.ElideLeft
                Layout.maximumWidth: panel.width * 0.6
            }
        }

        Item {
            id: body
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
