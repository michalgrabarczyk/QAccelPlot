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

// A labeled drop-down that edits one setting.
GridLayout {
    id: root

    required property string label
    // One { text, value } entry per choice.
    required property var choices
    // Current value of the setting.
    required property var value
    // Puts the drop-down below the label, for long choice texts.
    property bool stacked: false

    signal selected(var value)

    columns: stacked ? 1 : 2
    columnSpacing: 8
    rowSpacing: 4

    Label {
        text: root.label
        color: QAccelPlot.Colors.dark.textSecondary
        elide: Text.ElideRight
        Layout.fillWidth: true
    }

    ComboBox {
        model: root.choices
        textRole: "text"
        currentIndex: root.choices.findIndex(choice => choice.value === root.value)
        Layout.fillWidth: root.stacked
        Layout.preferredWidth: 130
        Layout.preferredHeight: 40
        onActivated: index => root.selected(root.choices[index].value)
    }
}
