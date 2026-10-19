//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick

PageSettings {
    noun: "cells"
    count: 1000000
    hoverEnabled: false
    hoverSupported: false

    // Whether the color range is found by scanning the values on every update instead of being set to 0..1.
    property bool autoColorRange: false
}
