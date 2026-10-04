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

// A labeled switch that edits one boolean setting.
RowLayout {
    id: root

    required property string label
    // Current value of the setting.
    required property bool checked

    signal toggled(bool checked)

    spacing: 8

    Label {
        text: root.label
        color: QAccelPlot.Colors.dark.textSecondary
        elide: Text.ElideRight
        Layout.fillWidth: true
    }

    Switch {
        checked: root.checked
        Layout.preferredHeight: 40
        onToggled: root.toggled(checked)
    }
}
