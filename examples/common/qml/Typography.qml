//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick

QtObject {
    readonly property font axisLabel: Qt.font({
        pixelSize: 13,
        weight: Font.Normal
    })

    readonly property font axisTickLabel: Qt.font({
        pixelSize: 12,
        weight: Font.Normal
    })
}
