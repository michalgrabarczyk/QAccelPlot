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
    readonly property var monthNames: ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"]
    readonly property var regionNames: ["North", "South", "West"]
    readonly property var regionColors: [colorPalette.seriesPrimary, colorPalette.seriesSecondary, colorPalette.seriesTertiary]
    // Monthly revenue in thousands of dollars, one list per region.
    readonly property var revenue: [
        [62, 58, 71, 80, 86, 95, 102, 98, 88, 79, 90, 118],
        [45, 49, 55, 61, 66, 72, 75, 77, 70, 64, 69, 84],
        [30, 34, 41, 47, 58, 69, 81, 85, 72, 55, 48, 60]
    ]
    // Monthly profit in thousands of dollars.
    readonly property var profit: [12, 8, -4, 15, 20, -9, 5, 18, -2, 10, 25, 31]
    readonly property bool horizontal: horizontalCheck.checked
    readonly property real groupWidth: groupWidthSlider.value

    width: 1200
    height: 800
    visible: true
    title: "Bar Chart Example"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    // Colors each profit bar by whether it meets the target.
    function updateProfitBars() {
        profitBars.setData(profit.map((value, month) => ({
                        position: month,
                        value: value,
                        category: value >= targetSlider.value ? 0 : 1
                    })));
    }

    QAccelPlot.TextTickLabelFormatter {
        id: monthLabels
        labels: window.monthNames
    }

    QAccelPlot.NumericTickLabelFormatter {
        id: revenueLabels
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        ExampleHeader {
            title: "Bar chart"
            description: "Grouped monthly revenue in three BarSeries that share a position axis through barWidth and barOffset, and monthly profit colored by whether it meets a target set as baselineValue. Hover a profit bar to read it."

            CheckBox {
                id: horizontalCheck
                text: "Horizontal"
            }

            CheckBox {
                id: outlineCheck
                text: "Outlines"
            }

            Label {
                text: "Group width " + groupWidthSlider.value.toFixed(2)
                color: colorPalette.text
            }
            Slider {
                id: groupWidthSlider
                from: 0.3
                to: 1
                value: 0.8
                Layout.preferredWidth: 120
            }

            Label {
                id: targetLabel
                text: "Profit target " + targetSlider.value.toFixed(0) + " k$"
                color: colorPalette.text
                // Sized for the widest value, so the slider doesn't move as the digit count changes.
                Layout.preferredWidth: widestTargetText.advanceWidth

                TextMetrics {
                    id: widestTargetText
                    font: targetLabel.font
                    text: "Profit target " + targetSlider.from.toFixed(0) + " k$"
                }
            }
            Slider {
                id: targetSlider
                from: -10
                to: 20
                stepSize: 1
                value: 0
                Layout.preferredWidth: 120
                onValueChanged: window.updateProfitBars()
            }
        }

        QAccelPlot.Plot {
            id: revenuePlot
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 3
            border.color: colorPalette.plotBorder
            border.width: 2
            grid.subGridVisible: false

            // The month axis is X for vertical bars and Y for horizontal ones.
            xAxis: ExampleAxis {
                viewportMin: window.horizontal ? 0 : -0.5
                viewportMax: window.horizontal ? 160 : 11.5
                label: window.horizontal ? "Revenue (k$)" : "Month"
                ticker.tickCount: window.horizontal ? 9 : 12
                ticker.subtickCount: window.horizontal ? 4 : 0
                ticker.tickLabelFormatter: window.horizontal ? revenueLabels : monthLabels
            }

            yAxis: ExampleAxis {
                viewportMin: window.horizontal ? -0.5 : 0
                viewportMax: window.horizontal ? 11.5 : 160
                layoutSize: 70
                axisTitlePadding: 50
                label: window.horizontal ? "Month" : "Revenue (k$)"
                ticker.tickCount: window.horizontal ? 12 : 9
                ticker.subtickCount: window.horizontal ? 0 : 4
                ticker.tickLabelFormatter: window.horizontal ? monthLabels : revenueLabels
            }

            Repeater {
                model: window.regionNames

                QAccelPlot.BarSeries {
                    required property int index
                    required property string modelData

                    name: modelData
                    xAxis: revenuePlot.xAxis
                    yAxis: revenuePlot.yAxis
                    orientation: window.horizontal ? Qt.Horizontal : Qt.Vertical
                    color: window.regionColors[index]
                    barWidth: window.groupWidth / window.regionNames.length
                    barOffset: (index - (window.regionNames.length - 1) / 2) * barWidth
                    border.width: outlineCheck.checked ? 1 : 0
                    border.color: colorPalette.plotArea

                    Component.onCompleted: setData(window.revenue[index])
                }
            }
        }

        QAccelPlot.Plot {
            id: profitPlot
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 2
            border.color: colorPalette.plotBorder
            border.width: 2
            legendVisible: false
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                viewportMin: -0.5
                viewportMax: 11.5
                label: "Month"
                ticker.tickCount: 12
                ticker.subtickCount: 0
                ticker.tickLabelFormatter: QAccelPlot.TextTickLabelFormatter {
                    labels: window.monthNames
                }
            }

            yAxis: ExampleAxis {
                viewportMin: -15
                viewportMax: 35
                layoutSize: 70
                axisTitlePadding: 50
                label: "Profit (k$)"
            }

            QAccelPlot.BarSeries {
                id: profitBars
                objectName: "profitBars"
                xAxis: profitPlot.xAxis
                yAxis: profitPlot.yAxis
                baselineValue: targetSlider.value
                categoryColors: [colorPalette.statusGood, colorPalette.statusError]
                hoverColor: colorPalette.text
                border.width: outlineCheck.checked ? 1 : 0
                border.color: colorPalette.plotArea

                Component.onCompleted: window.updateProfitBars()
            }

            Item {
                id: overlay
                x: profitPlot.plotRect.x
                y: profitPlot.plotRect.y
                width: profitPlot.plotRect.width
                height: profitPlot.plotRect.height
                clip: true

                QAccelPlot.DataAnchor {
                    id: tooltipAnchor

                    readonly property var hovered: profitBars.hoveredIndex >= 0 ? profitBars.barAt(profitBars.hoveredIndex) : null

                    visible: hovered !== null
                    xAxis: profitPlot.xAxis
                    yAxis: profitPlot.yAxis
                    plotRect: Qt.rect(0, 0, overlay.width, overlay.height)
                    dataX1: hovered ? hovered.position : 0
                    dataX2: dataX1
                    dataY1: hovered ? Math.max(hovered.value, profitBars.baselineValue) : 0
                    dataY2: dataY1

                    Rectangle {
                        x: -width / 2
                        y: -height - 6
                        width: tooltipText.implicitWidth + 12
                        height: tooltipText.implicitHeight + 8
                        radius: 3
                        color: colorPalette.tooltipBackground

                        Text {
                            id: tooltipText
                            anchors.centerIn: parent
                            color: colorPalette.tooltipText
                            font.pixelSize: 11
                            text: {
                                const bar = tooltipAnchor.hovered;
                                if (!bar) {
                                    return "";
                                }
                                const difference = bar.value - profitBars.baselineValue;
                                return window.monthNames[bar.position] + ": " + bar.value + " k$ (" + (difference >= 0 ? "+" : "") + difference
                                    + " vs. target)";
                            }
                        }
                    }
                }
            }
        }
    }
}
