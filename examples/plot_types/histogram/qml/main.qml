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
    readonly property int binCount: Math.round(binSlider.value)
    readonly property bool tickBins: tickBinsCheck.checked
    // Sensor noise in millivolts with two peaks, and request latencies in milliseconds.
    readonly property var noise: randomSamples(20000, 1, (normal, uniform) => uniform() < 0.6 ? -1 + 0.6 * normal() : 1.4 + 0.4 * normal())
    readonly property var latencies: randomSamples(20000, 2, normal => 80 * Math.exp(0.9 * normal()))
    readonly property var noiseHistogram: QAccelPlot.Histogram.fromSamples(noise, binCount, -3, 3)
    // Bins of equal drawn width show counts. Bins between the axis ticks differ in width, so they show density.
    readonly property var latencyHistogram: tickBins ? QAccelPlot.Histogram.fromSamples(latencies, QAccelPlot.Histogram.decadeEdges(1, 10000)).density() :
        QAccelPlot.Histogram.fromSamples(latencies, QAccelPlot.Histogram.logEdges(1, 10000, binCount))

    width: 1200
    height: 800
    visible: true
    title: "Histogram Example"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    // Returns count samples of transform(normal, uniform), which draw from the standard normal
    // distribution and from 0 to 1. The generator is seeded, so every run shows the same data.
    function randomSamples(count, seed, transform) {
        let state = seed;
        const uniform = () => {
            state = state + 0x6D2B79F5 | 0;
            let mixed = Math.imul(state ^ state >>> 15, 1 | state);
            mixed = mixed + Math.imul(mixed ^ mixed >>> 7, 61 | mixed) ^ mixed;
            return ((mixed ^ mixed >>> 14) >>> 0) / 4294967296;
        };
        const normal = () => Math.sqrt(-2 * Math.log(1 - uniform())) * Math.cos(2 * Math.PI * uniform());
        const samples = [];
        for (let i = 0; i < count; ++i) {
            samples.push(transform(normal, uniform));
        }
        return samples;
    }

    function largest(values) {
        let result = 0;
        for (let i = 0; i < values.length; ++i) {
            result = Math.max(result, values[i]);
        }
        return result;
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        ExampleHeader {
            title: "Histogram"
            description: "Histograms of sensor noise and request latencies. Change the bin count, or align the latency bins with the axis ticks."

            CheckBox {
                id: tickBinsCheck
                text: "Bins on ticks"
            }

            Label {
                text: "Bins " + window.binCount
                color: colorPalette.text
                Layout.preferredWidth: 60
            }
            Slider {
                id: binSlider
                from: 5
                to: 100
                stepSize: 1
                value: 40
                Layout.preferredWidth: 160
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 6

            QAccelPlot.Plot {
                id: noisePlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                border.color: colorPalette.plotBorder
                border.width: 2
                legendVisible: false
                grid.subGridVisible: false

                xAxis: ExampleAxis {
                    viewportMin: -3
                    viewportMax: 3
                    label: "Sensor noise (mV), equal bins"
                }

                yAxis: ExampleAxis {
                    viewportMin: 0
                    viewportMax: 1.1 * window.largest(window.noiseHistogram.values) || 1
                    layoutSize: 70
                    axisTitlePadding: 50
                    label: "Count"
                }

                QAccelPlot.BarSeries {
                    objectName: "noiseBins"

                    readonly property var histogram: window.noiseHistogram

                    xAxis: noisePlot.xAxis
                    yAxis: noisePlot.yAxis
                    color: colorPalette.seriesPrimary
                    border.width: 1
                    border.color: colorPalette.plotArea

                    onHistogramChanged: setData(histogram.bars())
                    Component.onCompleted: setData(histogram.bars())
                }
            }

            QAccelPlot.Plot {
                id: latencyPlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                border.color: colorPalette.plotBorder
                border.width: 2
                legendVisible: false

                xAxis: ExampleAxis {
                    logScale: true
                    viewportMin: 1
                    viewportMax: 10000
                    label: "Request latency (ms), " + (window.tickBins ? "bins on the axis ticks" : "logarithmic bins")
                }

                yAxis: ExampleAxis {
                    viewportMin: 0
                    viewportMax: 1.1 * window.largest(window.latencyHistogram.values) || 1
                    layoutSize: 70
                    axisTitlePadding: 50
                    label: window.tickBins ? "Density (1/ms)" : "Count"
                }

                QAccelPlot.BarSeries {
                    objectName: "latencyBins"

                    readonly property var histogram: window.latencyHistogram

                    xAxis: latencyPlot.xAxis
                    yAxis: latencyPlot.yAxis
                    color: colorPalette.seriesSecondary
                    border.width: 1
                    border.color: colorPalette.plotArea

                    onHistogramChanged: setData(histogram.bars())
                    Component.onCompleted: setData(histogram.bars())
                }
            }
        }
    }
}
