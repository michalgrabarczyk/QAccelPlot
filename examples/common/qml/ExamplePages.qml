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

// Tab bar over a stack of page items. Each tab name is a page name accepted by --page.
ColumnLayout {
    id: root

    // One { name, title } entry per page item, in the same order.
    required property var tabs
    default property alias pages: stack.data

    Layout.fillWidth: true
    Layout.fillHeight: true
    spacing: 8

    TabBar {
        id: tabBar
        objectName: "examplePages"
        Layout.fillWidth: true

        Repeater {
            model: root.tabs

            TabButton {
                required property var modelData
                objectName: modelData.name
                text: modelData.title
            }
        }
    }

    StackLayout {
        id: stack
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tabBar.currentIndex
    }
}
