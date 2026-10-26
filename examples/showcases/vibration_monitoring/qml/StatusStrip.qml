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

// Acquisition state, frame rate, and the hold and theme switches.
RowLayout {
    id: strip

    required property QtObject monitor
    required property QtObject colorPalette
    property bool darkTheme: true

    signal darkThemeToggled(bool dark)

    spacing: 12

    Rectangle {
        implicitWidth: 9
        implicitHeight: 9
        radius: 4.5
        color: strip.monitor.paused ? strip.colorPalette.statusWarning : strip.colorPalette.statusGood
    }

    Label {
        text: strip.monitor.paused ? "HOLD" : "LIVE"
        color: strip.colorPalette.text
        font.pixelSize: 12
        font.bold: true
    }

    Item {
        Layout.fillWidth: true
    }

    // The frame rate is measured live; recordings and screenshots step time instead.
    Label {
        visible: !strip.monitor.recording
        text: strip.monitor.framesPerSecond + " FPS"
        color: strip.colorPalette.text
        font.pixelSize: 12
        font.bold: true
    }

    Button {
        visible: !strip.monitor.recording
        flat: true
        text: strip.monitor.paused ? "Resume" : "Hold"
        font.pixelSize: 12
        onClicked: strip.monitor.paused = !strip.monitor.paused
    }

    Switch {
        text: "Dark"
        font.pixelSize: 12
        checked: strip.darkTheme
        onToggled: strip.darkThemeToggled(checked)
    }
}
