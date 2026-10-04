//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QAccelPlot as QAccelPlot

Item {
    id: root

    required property var colorPalette
    readonly property int lastObservedMonth: 36
    readonly property int forecastMonths: 24
    // A lighter tint of the band color, so the edges read as a quiet outline.
    readonly property color edgeColor: Qt.lighter(colorPalette.seriesPrimary, 1.35)

    function withAlpha(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha);
    }

    // Trend plus a yearly season.
    function expectedDemand(month) {
        return 120 + 1.1 * month + 14 * Math.sin(2 * Math.PI * (month - 2) / 12);
    }

    // Deterministic month-to-month variation around the expected demand.
    function observedDemand(month) {
        return expectedDemand(month) + 5 * Math.sin(month * 2.3) * Math.cos(month * 0.7);
    }

    function historyPoints() {
        const points = [];
        for (let month = 0; month <= lastObservedMonth; ++month)
            points.push(Qt.point(month, observedDemand(month)));
        return points;
    }

    // The forecast starts at the last observation and returns to the expected demand.
    function forecastAt(month) {
        const horizon = month - lastObservedMonth;
        const offset = observedDemand(lastObservedMonth) - expectedDemand(lastObservedMonth);
        return expectedDemand(month) + offset * Math.exp(-horizon / 2);
    }

    function forecastPoints() {
        const points = [];
        for (let month = lastObservedMonth; month <= lastObservedMonth + forecastMonths; ++month)
            points.push(Qt.point(month, forecastAt(month)));
        return points;
    }

    // Fills band with the central prediction interval for normal quantile z. Its width grows
    // with the square root of the horizon.
    function fillInterval(band, z) {
        const xs = [], lows = [], highs = [];
        for (let month = lastObservedMonth; month <= lastObservedMonth + forecastMonths; ++month) {
            const deviation = 6 * Math.sqrt(month - lastObservedMonth);
            xs.push(month);
            lows.push(forecastAt(month) - z * deviation);
            highs.push(forecastAt(month) + z * deviation);
        }
        band.setData(xs, lows, highs);
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: "Three years of monthly demand and a two-year forecast. Two BandSeries show the 50% and 90% prediction intervals; the outer one has dashed edge lines. Hover a band to highlight it and read its range."
            color: root.colorPalette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                viewportMin: 0
                viewportMax: 60
                dataMin: 0
                dataMax: 60
                label: "Month"
            }

            yAxis: ExampleAxis {
                viewportMin: 80
                viewportMax: 280
                dataMin: 80
                dataMax: 280
                axisTitlePadding: 40
                layoutSize: 60
                label: "Demand (units)"
            }

            QAccelPlot.BandSeries {
                id: outerBand
                name: "90% interval"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.withAlpha(root.colorPalette.seriesPrimary, hovered ? 0.3 : 0.16)
                edges.width: hovered ? 1 : 0.75
                edges.color: root.withAlpha(root.edgeColor, hovered ? 0.8 : 0.45)
                edges.lineStyle: QAccelPlot.DashLine {
                    pattern: [4, 3]
                }
                Component.onCompleted: root.fillInterval(this, 1.645)

                Behavior on color {
                    ColorAnimation {
                        duration: 150
                    }
                }
            }

            QAccelPlot.BandSeries {
                id: innerBand
                name: "50% interval"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.withAlpha(root.colorPalette.seriesPrimary, hovered ? 0.55 : 0.32)
                edges.width: hovered ? 0.75 : 0
                edges.color: root.withAlpha(root.edgeColor, 0.6)
                Component.onCompleted: root.fillInterval(this, 0.674)

                Behavior on color {
                    ColorAnimation {
                        duration: 150
                    }
                }
            }

            QAccelPlot.LineCurve {
                name: "Observed"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.colorPalette.seriesSecondary
                lineWidth: 2
                Component.onCompleted: setData(root.historyPoints())
            }

            // Only the topmost series under the cursor is hovered, so the forecast line gives up
            // hover to the bands beneath it.
            QAccelPlot.LineCurve {
                name: "Forecast"
                hoverRadius: 0
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.colorPalette.seriesPrimary
                lineWidth: 2
                lineStyle: QAccelPlot.DashLine {
                    pattern: [8, 4]
                }
                Component.onCompleted: setData(root.forecastPoints())
            }

            HoverHandler {
                id: hover
            }

            Rectangle {
                id: tooltip

                readonly property var band: innerBand.hovered ? innerBand : (outerBand.hovered ? outerBand : null)
                readonly property int month: Math.round(plot.pixelToDataX(hover.point.position.x))
                readonly property var interval: band ? band.valueAt(month) : ({})

                visible: interval.low !== undefined
                x: Math.min(hover.point.position.x + 14, plot.width - width - 4)
                y: Math.max(hover.point.position.y - height - 10, 4)
                z: 2
                width: tooltipText.implicitWidth + 12
                height: tooltipText.implicitHeight + 8
                radius: 3
                color: root.colorPalette.tooltipBackground

                Text {
                    id: tooltipText
                    anchors.centerIn: parent
                    color: root.colorPalette.tooltipText
                    font.pixelSize: 11
                    text: tooltip.visible ? tooltip.band.name + ", month " + tooltip.month + ": " + tooltip.interval.low.toFixed(1) + " to " + tooltip.interval.high.toFixed(1) : ""
                }
            }
        }
    }
}
