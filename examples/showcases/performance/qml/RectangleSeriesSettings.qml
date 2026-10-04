//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick

PageSettings {
    noun: "rectangles"
    count: 100000

    // Multiplies the side of every tile.
    property real tileScale: 1
    // Number of category colors; 0 gives the rectangles no category. The raw-copy ingestion carries none.
    property int categoryCount: 0
    property real borderWidth: 0
    // Minimum drawn width and height of a rectangle, in pixels.
    property real minimumSize: 1
    // Whether the rectangle under the cursor is highlighted.
    property bool hoverHighlight: false
}
