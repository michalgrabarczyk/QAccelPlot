//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
import QtQuick

PageSettings {
    noun: "bars"
    count: 100000

    // Number of category colors; 0 gives the bars no category. The raw-copy ingestion carries none.
    property int categoryCount: 0
    // Bar width as a share of the distance between neighboring bars.
    property real barWidth: 0.8
    property real borderWidth: 0
    // Minimum drawn width of a bar, in pixels.
    property real minimumWidth: 1
    // Whether the bar under the cursor is highlighted.
    property bool hoverHighlight: false
}
