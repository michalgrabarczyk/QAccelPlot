//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

PageSettings {
    // Whether every point carries a colormap value. The raw-copy ingestion carries none.
    property bool values: true
    // Whether the colormap range is fixed instead of scanned from the values on every update.
    property bool fixedColormapRange: true
    // Share of the points whose X is NaN.
    property real invalidFraction: 0
    property int markerShape: QAccelPlot.PointCloud.Circle
    property real markerSize: 2
    property bool markerFilled: true
    // Outline width of hollow markers.
    property real markerStrokeWidth: 1
    property bool antialiasing: true
}
