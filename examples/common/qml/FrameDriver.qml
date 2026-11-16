//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
import QtQuick

FrameAnimation {
    property var targetWindow

    // Without vsync Qt advances animations from a timer that requests no frames, so ask for one.
    onTriggered: {
        if (targetWindow) {
            targetWindow.update();
        }
    }
}
