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

// Title and description; child items are placed to the right of the text.
RowLayout {
    id: root

    property alias title: titleLabel.text
    property alias description: descriptionLabel.text
    property QtObject colorPalette: QAccelPlot.Colors.dark

    Layout.fillWidth: true
    spacing: 8

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 1

        Label {
            id: titleLabel
            color: root.colorPalette.text
            font.bold: true
            font.pixelSize: 20
        }

        Label {
            id: descriptionLabel
            visible: text !== ""
            color: root.colorPalette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}
