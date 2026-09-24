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

    readonly property QtObject colorPalette: QAccelPlot.Colors.dark
    property bool antialiasingEnabled: true
    property real antialiasingFeather: 1.0

    function curvePoints(offset) {
        const values = [0.12, 0.32, 0.20, 0.58, 0.42, 0.76, 0.62, 0.88, 0.71, 0.93, 0.80, 1.00];
        const points = [];
        for (let i = 0; i < values.length; ++i)
            points.push(Qt.point(i, values[i] + offset));
        return points;
    }

    width: 900
    height: 760
    visible: true
    title: "QAccelPlot Line Styles"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    component SampleAxis: ExampleAxis {
        viewportMin: 0
        viewportMax: 11
        dataMin: 0
        dataMax: 11
        label: "Sample"
    }

    component OffsetAxis: ExampleAxis {
        viewportMin: -0.1
        viewportMax: 4.1
        dataMin: -0.1
        dataMax: 4.1
        axisTitlePadding: 40
        layoutSize: 60
    }

    component StyledCurve: QAccelPlot.LineCurve {
        antialiasingEnabled: window.antialiasingEnabled
        antialiasingFeather: window.antialiasingFeather
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Line styles"
            description: "Dash patterns and line weights on the same fixed data. The antialiasing feather softens line edges by the given width."
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Switch {
                text: "Antialiasing"
                checked: window.antialiasingEnabled
                onToggled: window.antialiasingEnabled = checked
            }
            Label {
                text: "Feather " + window.antialiasingFeather.toFixed(1) + " px"
                opacity: window.antialiasingEnabled ? 1.0 : 0.45
            }
            Slider {
                from: 0
                to: 4
                stepSize: 0.1
                value: window.antialiasingFeather
                enabled: window.antialiasingEnabled
                Layout.preferredWidth: 120
                onMoved: window.antialiasingFeather = value
            }
            Item {
                Layout.fillWidth: true
            }
        }

        QAccelPlot.Plot {
            id: dashPlot
            Layout.fillWidth: true
            Layout.fillHeight: true
            grid.subGridVisible: false
            legend: QAccelPlot.Legend {
                series: dashPlot.series
                symbolWidth: 34
            }

            xAxis: SampleAxis {}
            yAxis: OffsetAxis {
                label: "Style offset"
            }

            StyledCurve {
                name: "Solid"
                xAxis: dashPlot.xAxis
                yAxis: dashPlot.yAxis
                color: colorPalette.seriesPrimary
                lineWidth: 2
                Component.onCompleted: setData(window.curvePoints(3.0))
            }
            StyledCurve {
                name: "Dash"
                xAxis: dashPlot.xAxis
                yAxis: dashPlot.yAxis
                color: colorPalette.seriesQuaternary
                lineWidth: 2
                lineStyle: QAccelPlot.DashLine {
                    pattern: [12, 6]
                }
                Component.onCompleted: setData(window.curvePoints(2.0))
            }
            StyledCurve {
                name: "Dot-dash"
                xAxis: dashPlot.xAxis
                yAxis: dashPlot.yAxis
                color: colorPalette.seriesTertiary
                lineWidth: 2
                lineStyle: QAccelPlot.DashLine {
                    pattern: [14, 5, 3, 5]
                }
                Component.onCompleted: setData(window.curvePoints(1.0))
            }
            StyledCurve {
                name: "Dots"
                xAxis: dashPlot.xAxis
                yAxis: dashPlot.yAxis
                color: colorPalette.seriesSecondary
                lineWidth: 2
                lineStyle: QAccelPlot.DashLine {
                    pattern: [2, 6]
                }
                Component.onCompleted: setData(window.curvePoints(0.0))
            }
        }

        QAccelPlot.Plot {
            id: weightPlot
            Layout.fillWidth: true
            Layout.fillHeight: true
            grid.subGridVisible: false
            legend: QAccelPlot.Legend {
                series: weightPlot.series
                symbolWidth: 34
            }

            xAxis: SampleAxis {}
            yAxis: OffsetAxis {
                label: "Line weight"
            }

            StyledCurve {
                name: "1 px"
                xAxis: weightPlot.xAxis
                yAxis: weightPlot.yAxis
                color: colorPalette.seriesMuted
                lineWidth: 1
                Component.onCompleted: setData(window.curvePoints(3.0))
            }
            StyledCurve {
                name: "2 px"
                xAxis: weightPlot.xAxis
                yAxis: weightPlot.yAxis
                color: colorPalette.seriesPrimary
                lineWidth: 2
                Component.onCompleted: setData(window.curvePoints(2.0))
            }
            StyledCurve {
                name: "4 px"
                xAxis: weightPlot.xAxis
                yAxis: weightPlot.yAxis
                color: colorPalette.seriesQuaternary
                lineWidth: 4
                Component.onCompleted: setData(window.curvePoints(1.0))
            }
            StyledCurve {
                name: "6 px"
                xAxis: weightPlot.xAxis
                yAxis: weightPlot.yAxis
                color: colorPalette.seriesSecondary
                lineWidth: 6
                Component.onCompleted: setData(window.curvePoints(0.0))
            }
        }
    }
}
