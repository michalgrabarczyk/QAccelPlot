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
}
