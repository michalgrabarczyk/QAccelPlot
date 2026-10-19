//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QtQuick.Layouts

// Controls for the HeatmapSeries page's own settings.
ColumnLayout {
    id: root

    required property QtObject settings

    spacing: 12

    OptionSwitch {
        label: "Auto color range"
        checked: root.settings.autoColorRange
        Layout.fillWidth: true
        onToggled: checked => root.settings.autoColorRange = checked
    }
}
