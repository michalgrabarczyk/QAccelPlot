//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QAccelPlot as QAccelPlot

Window {
    id: window

    // MarketFeed, from main.cpp.
    required property QtObject feed
    readonly property QtObject colorPalette: QAccelPlot.Colors.dark
    readonly property int historyCount: 50000
    // Hourly candles of a 9:30–16:00 session: seven per trading day, Monday to Friday.
    readonly property int candlesPerDay: 7
    // Candles shown at start and after a double-click.
    readonly property int visibleCount: 140
    // True until you pan or zoom: the plot follows the newest candle and fits the value axes.
    property bool autoFit: true
    // True while the example moves the axes itself, so only your pans and zooms turn autoFit off.
    property bool movingAxes: false
    // The candle the readout shows: the hovered one, or the newest.
    readonly property int shownIndex: candles.hoveredIndex >= 0 ? candles.hoveredIndex : candles.count - 1
    readonly property var shownCandle: {
        candles.dataRevision; // Re-read when the live candle changes.
        return candles.candleAt(shownIndex);
    }

    width: 1200
    height: 800
    visible: true
    title: "Candlestick Example"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    // Returns the start time of candle \a index. The index skips nights and weekends, so the
    // plot shows no gaps for them.
    function candleTime(index) {
        const day = Math.floor(index / candlesPerDay);
        const calendarDays = Math.floor(day / 5) * 7 + day % 5;
        // 4 January 1999 is a Monday.
        return new Date(1999, 0, 4 + calendarDays, 9 + index % candlesPerDay, 30);
    }

    function formatPrice(value) {
        return value === undefined ? "–" : value.toFixed(2);
    }

    // Fits the price axis to the visible candles in the upper part of the plot, and the volume
    // axis to their volume in the lower part.
    function fitValueAxes() {
        if (!autoFit) {
            return;
        }
        const extent = feed.extent(plot.xAxis.viewportMin, plot.xAxis.viewportMax);
        if (extent.low === undefined) {
            return;
        }
        const span = Math.max(extent.high - extent.low, 1e-6);
        movingAxes = true;
        plot.yAxis.setViewportRange(extent.low - 0.4 * span, extent.high + 0.05 * span);
        plot.y2Axis.setViewportRange(0, 4.5 * extent.volume);
        movingAxes = false;
    }

    // Shows the newest candles and turns autoFit back on.
    function showNewest() {
        autoFit = true;
        movingAxes = true;
        plot.xAxis.setViewportRange(candles.count - visibleCount, candles.count + 4);
        movingAxes = false;
        fitValueAxes();
    }

    // Turns autoFit off when you pan or zoom the axis, and back on when you double-click it.
    component AxisWatcher: Connections {
        function onRangeChanged() {
            if (!window.movingAxes) {
                window.autoFit = false;
            }
        }

        // Replaces the default rescale to all data.
        function onDoubleClicked(event) {
            event.accepted = true;
            window.showNewest();
        }
    }

    AxisWatcher {
        target: plot.xAxis
    }

    AxisWatcher {
        target: plot.yAxis
    }

    AxisWatcher {
        target: plot.y2Axis
    }

    Connections {
        target: window.feed

        function onCandleAppended() {
            if (!window.autoFit) {
                return;
            }
            window.movingAxes = true;
            plot.xAxis.setViewportRange(plot.xAxis.viewportMin + 1, plot.xAxis.viewportMax + 1);
            window.movingAxes = false;
            window.fitValueAxes();
        }

        function onLastCandleChanged() {
            window.fitValueAxes();
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        ExampleHeader {
            title: "Candlestick"
            subtitle: "Fifty thousand hourly candles in a CandlestickSeries, with volume bars and a live last candle."
            interactions: [
                "Click <b>Candlestick</b> or <b>OHLC</b> to switch how candles are drawn",
                "Hover a candle to read its open, high, low, close, and volume",
                "Pan or zoom to stop following the newest candle; double-click to follow it again",
                "Click <b>Pause</b> to stop the live candle, and <b>Resume</b> to continue"
            ]
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            ButtonGroup {
                id: styleGroup
            }

            Button {
                id: candlestickButton
                text: "Candlestick"
                checkable: true
                checked: true
                ButtonGroup.group: styleGroup
                Material.background: checked ? colorPalette.control : colorPalette.plotArea
            }

            Button {
                text: "OHLC"
                checkable: true
                ButtonGroup.group: styleGroup
                Material.background: checked ? colorPalette.control : colorPalette.plotArea
            }

            Button {
                text: window.feed.live ? "Pause" : "Resume"
                Material.background: colorPalette.plotArea
                onClicked: window.feed.live = !window.feed.live
            }

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                color: colorPalette.text
                font.family: "monospace"
                text: {
                    const candle = window.shownCandle;
                    if (candle.x === undefined) {
                        return "";
                    }
                    const volume = volumeBars.barAt(window.shownIndex).value;
                    return Qt.formatDateTime(window.candleTime(candle.x), "ddd d MMM yyyy HH:mm") + "   O " + window.formatPrice(candle.open) + "  H "
                        + window.formatPrice(candle.high) + "  L " + window.formatPrice(candle.low) + "  C " + window.formatPrice(candle.close) + "  Vol "
                        + (volume === undefined ? "–" : Math.round(volume).toLocaleString(Qt.locale(), "f", 0));
                }
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            plotAreaColor: colorPalette.plotArea
            axesAreaColor: colorPalette.axesArea
            grid.gridColor: colorPalette.grid
            grid.subGridVisible: false

            // Replaces the default rescale to all data.
            onMouseDoubleClicked: event => {
                if (event.button === Qt.LeftButton) {
                    event.accepted = true;
                    window.showNewest();
                }
            }

            xAxis: ExampleAxis {
                label.text: "Time (trading hours only)"
                ticker.tickLabelFormatter: QAccelPlot.NumericTickLabelFormatter {
                    // Labels only the whole indices, at the start time of their candle.
                    tickLabel: function (index, step) {
                        if (Math.abs(index - Math.round(index)) > 1e-6) {
                            return "";
                        }
                        return Qt.formatDateTime(window.candleTime(Math.round(index)), step < window.candlesPerDay ? "d MMM HH:mm" : "d MMM yyyy");
                    }
                }
            }

            yAxis: ExampleAxis {
                label.text: "Price"
            }

            y2Axis: ExampleAxis {
                label.text: "Volume"
            }

            // Drawn first, so that the candles are drawn and hovered on top.
            QAccelPlot.BarSeries {
                id: volumeBars
                name: "Volume"
                xAxis: plot.xAxis
                yAxis: plot.y2Axis
                // As wide as the candles.
                barWidth: candles.candleWidth * candles.spacing
                hoverEnabled: false
                categories: [
                    QAccelPlot.Category { value: 0; name: "Up volume"; color: Qt.rgba(colorPalette.candleUp.r, colorPalette.candleUp.g, colorPalette.candleUp.b, 0.45) },
                    QAccelPlot.Category { value: 1; name: "Down volume"; color: Qt.rgba(colorPalette.candleDown.r, colorPalette.candleDown.g, colorPalette.candleDown.b, 0.45) }
                ]
            }

            QAccelPlot.CandlestickSeries {
                id: candles
                objectName: "candles"
                name: "Price"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                style: candlestickButton.checked ? QAccelPlot.CandlestickSeries.Candlestick : QAccelPlot.CandlestickSeries.Ohlc
                upColor: colorPalette.candleUp
                downColor: colorPalette.candleDown
                hoverColor: colorPalette.hover

                Component.onCompleted: {
                    window.feed.start(candles, volumeBars, window.historyCount);
                    window.showNewest();
                }
            }
        }
    }
}
