//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
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
