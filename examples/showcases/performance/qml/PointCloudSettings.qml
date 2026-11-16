//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
import QtQuick
import QAccelPlot as QAccelPlot

PageSettings {
    // Whether every point carries a colormap value. The raw-copy ingestion carries none.
    property bool values: true
    // Whether the color range is found by scanning the values on every update instead of being set to 0..1.
    property bool autoColorRange: false
    // Share of the points whose X is NaN.
    property real invalidFraction: 0
    property int markerShape: QAccelPlot.MarkerShape.Circle
    property real markerSize: 2
    property bool markerFilled: true
    // Outline width of hollow markers.
    property real markerStrokeWidth: 1
    property bool antialiasing: true
}
