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
    // Multiplies the number of wave cycles across the plot.
    property real frequencyScale: 1
    // Share of the samples replaced with NaN.
    property real gapFraction: 0
    // Whether the line joins the samples around a gap instead of breaking.
    property bool connectGaps: false
    property real lineWidth: 3
    // "solid", "dash", or "none".
    property string lineStyle: "solid"
    property int markerShape: QAccelPlot.LineCurve.None
    property real markerSize: 4
    // "none", "stroke", or "fill".
    property string effect: "none"
    property bool antialiasing: true
    property real antialiasingFeather: 1
}
