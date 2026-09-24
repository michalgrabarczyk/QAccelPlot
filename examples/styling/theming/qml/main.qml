//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QAccelPlot as QAccelPlot

Window {
    id: window

    property bool isDark: true
    readonly property QtObject colorPalette: isDark ? QAccelPlot.Colors.dark : QAccelPlot.Colors.light

    function seasonalPoints(phase, amplitude, offset) {
        const points = [];
        for (let month = 0; month <= 12; month += 0.25)
            points.push(Qt.point(month, offset + amplitude * Math.sin(2 * Math.PI * (month - phase) / 12)));
        return points;
    }

    width: 900
    height: 640
    visible: true
    title: "QAccelPlot Theming"
    color: colorPalette.window
    Material.theme: isDark ? Material.Dark : Material.Light
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    component ThemedAxis: QAccelPlot.Axis {
        baselineColor: colorPalette.axisLine
        hoverColor: colorPalette.hover
        labelColor: colorPalette.axisLabel
        ticker.tickColor: colorPalette.tick
        ticker.subtickColor: colorPalette.subtick
        ticker.tickLabelColor: colorPalette.axisTickLabel
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Theming"
            description: "Every plot color is a property. Bind them to QAccelPlot.Colors.dark or QAccelPlot.Colors.light, or to your own palette. Unset colors default to the dark palette."
            colorPalette: window.colorPalette

            Switch {
                text: "Dark theme"
                checked: window.isDark
                onToggled: window.isDark = checked
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            plotAreaColor: colorPalette.plotArea
            axesAreaColor: colorPalette.axesArea
            border.color: colorPalette.plotBorder
            border.width: 2
            grid.gridColor: colorPalette.grid
            grid.subGridColor: colorPalette.subGrid
            legend: QAccelPlot.Legend {
                series: plot.series
                color: colorPalette.legendBackground
                textColor: colorPalette.text
                border.color: colorPalette.legendBorder
            }

            xAxis: ThemedAxis {
                viewportMin: 0
                viewportMax: 12
                dataMin: 0
                dataMax: 12
                label: "Month"
            }

            yAxis: ThemedAxis {
                viewportMin: -5
                viewportMax: 30
                dataMin: -5
                dataMax: 30
                axisTitlePadding: 40
                layoutSize: 60
                label: "Temperature (°C)"
            }

            QAccelPlot.LineCurve {
                name: "Coastal"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: colorPalette.seriesPrimary
                lineWidth: 2.5
                Component.onCompleted: setData(window.seasonalPoints(4, 7, 14))
            }

            QAccelPlot.LineCurve {
                name: "Inland"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: colorPalette.seriesSecondary
                lineWidth: 2.5
                Component.onCompleted: setData(window.seasonalPoints(4, 11, 12))
            }

            QAccelPlot.LineCurve {
                name: "Mountain"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: colorPalette.seriesTertiary
                lineWidth: 2.5
                Component.onCompleted: setData(window.seasonalPoints(4.5, 9, 6))
            }
        }
    }
}
