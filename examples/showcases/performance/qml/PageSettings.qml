//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick

// Options every page offers. Each page adds its own.
QtObject {
    // Plural name of one record of the dataset.
    property string noun: "points"
    // Total number of records across the series of the page.
    property int count: 1000000
    // Largest count the count field accepts.
    property int maximumCount: 10000000
    // Counts offered as buttons.
    property var countPresets: [1000, 10000, 100000, 300000, 500000, 1000000, 2000000, 5000000, 10000000]
    // Number of series the records are split across.
    property int seriesCount: 1
    // Series API that receives the records; a name known to ingestionFromName() in Ingestion.cpp.
    property string ingestion: "floatMove"
    // Whether the series take part in hover hit-testing.
    property bool hoverEnabled: true
    // Whether the page's series type has hover hit-testing at all.
    property bool hoverSupported: true
}
