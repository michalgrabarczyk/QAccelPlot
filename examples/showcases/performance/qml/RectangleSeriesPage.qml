//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

// Filled from C++ with setDataFNoRange(), so the axes keep the data range set here.
QAccelPlot.Plot {
    id: plot

    required property QtObject colorPalette
    required property int rectangleCount
    // Tiles overlap more as the count grows, so each gets fainter and overlap adds up to brightness.
    readonly property real tileOpacity: Math.max(0.015, Math.min(0.7, 100000 / rectangleCount))

    legendVisible: false

    xAxis: ExampleAxis {
        viewportMin: 0
        viewportMax: 160
        dataMin: 0
        dataMax: 160
        label: "x"
    }

    yAxis: ExampleAxis {
        viewportMin: 0
        viewportMax: 100
        dataMin: 0
        dataMax: 100
        axisTitlePadding: 40
        layoutSize: 60
        label: "y"
    }

    QAccelPlot.RectangleSeries {
        objectName: "rectangleSeries"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: Qt.rgba(plot.colorPalette.performanceRectangles.r, plot.colorPalette.performanceRectangles.g, plot.colorPalette.performanceRectangles.b, plot.tileOpacity)
    }
}
