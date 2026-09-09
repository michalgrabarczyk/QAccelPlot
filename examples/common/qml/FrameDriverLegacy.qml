//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQml

QtObject {
    id: root

    property bool running: true
    property bool paused: false
    property real elapsedTime: 0
    property var targetWindow

    signal triggered()

    property Timer timer: Timer {
        interval: 16
        repeat: true
        running: root.running && !root.paused
        onTriggered: {
            root.elapsedTime += interval / 1000;
            if (root.targetWindow)
                root.targetWindow.update();
            root.triggered();
        }
    }
}
