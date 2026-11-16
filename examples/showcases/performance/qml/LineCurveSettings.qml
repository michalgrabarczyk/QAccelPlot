//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
import QtQuick
import QAccelPlot as QAccelPlot

PageSettings {
    // Share of the samples replaced with NaN.
    property real gapFraction: 0
    // Whether the line joins the samples around a gap instead of breaking.
    property bool connectGaps: false
    property real lineWidth: 3
    // "solid", "dash", or "none".
    property string lineStyle: "solid"
    property int markerShape: QAccelPlot.MarkerShape.None
    property real markerSize: 4
    // "none", "stroke", or "fill".
    property string effect: "none"
    property bool antialiasing: true
    property real antialiasingFeather: 1
}
