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
    required property var palette
    required property var typography
    property bool synchronizingScales: false

    function synchronizeFrequencyFromWavelength() {
        if (synchronizingScales) {
            return;
        }
        synchronizingScales = true;
        frequencyAxis.viewportMin = wavelengthAxis.viewportMin;
        frequencyAxis.viewportMax = wavelengthAxis.viewportMax;
        synchronizingScales = false;
    }

    function synchronizeWavelengthFromFrequency() {
        if (synchronizingScales) {
            return;
        }
        synchronizingScales = true;
        wavelengthAxis.viewportMin = frequencyAxis.viewportMin;
        wavelengthAxis.viewportMax = frequencyAxis.viewportMax;
        synchronizingScales = false;
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: "Wavelength and frequency on one spectrum"
            color: root.palette.text
            font.bold: true
            font.pixelSize: 14
        }

        Label {
            text: "Both axes label the same screen positions, so they pan in the same visual direction. Their values are inverted: wavelength increases from left to right, while frequency (c / wavelength) decreases. Pan or zoom either axis and the other follows automatically."
            color: root.palette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            plotAreaColor: root.palette.plotArea
            axesAreaColor: root.palette.axesArea
            grid.gridColor: root.palette.grid
            grid.subGridVisible: false

            xAxis: QAccelPlot.Axis {
                id: wavelengthAxis
                viewportMin: 400
                viewportMax: 700
                dataMin: 400
                dataMax: 700
                side: QAccelPlot.Axis.Bottom
                label: "Wavelength (nm)"
                baselineColor: root.palette.seriesPrimary
                baselineWidth: 2
                labelColor: root.palette.axisLabel
                labelFont: root.typography.axisLabel
                hoverColor: root.palette.hover
                ticker.tickColor: root.palette.seriesPrimary
                ticker.tickLabelColor: root.palette.axisTickLabel
                ticker.tickLabelFont: root.typography.axisTickLabel
                ticker.subtickColor: root.palette.subtick
            }

            yAxis: QAccelPlot.Axis {
                viewportMin: 0
                viewportMax: 1.1
                dataMin: 0
                dataMax: 1.1
                side: QAccelPlot.Axis.Left
                axisTitlePadding: 40
                label: "Relative intensity"
                baselineColor: root.palette.axisLine
                baselineWidth: 2
                labelColor: root.palette.axisLabel
                labelFont: root.typography.axisLabel
                hoverColor: root.palette.hover
                ticker.tickColor: root.palette.tick
                ticker.tickLabelColor: root.palette.axisTickLabel
                ticker.tickLabelFont: root.typography.axisTickLabel
                ticker.subtickColor: root.palette.subtick
            }

            extraAxes: [
                QAccelPlot.Axis {
                    id: frequencyAxis
                    // Keep wavelength as the coordinate domain so each frequency
                    // label is placed at the physically corresponding wavelength.
                    viewportMin: 400
                    viewportMax: 700
                    dataMin: 400
                    dataMax: 700
                    orientation: QAccelPlot.Axis.Horizontal
                    side: QAccelPlot.Axis.Bottom
                    label: "Frequency (THz)"
                    baselineColor: root.palette.seriesCyan
                    baselineWidth: 2
                    labelColor: root.palette.axisLabel
                    labelFont: root.typography.axisLabel
                    hoverColor: root.palette.hover
                    ticker.tickColor: root.palette.seriesCyan
                    ticker.tickLabelColor: root.palette.axisTickLabel
                    ticker.tickLabelFont: root.typography.axisTickLabel
                    ticker.subtickColor: root.palette.subtick
                    ticker.tickCount: 6
                    ticker.tickLabelFormatter: QAccelPlot.NumericTickLabelFormatter {
                        tickLabel: function (wavelength) {
                            return (299792.458 / wavelength).toFixed(0);
                        }
                    }
                }
            ]

            Connections {
                target: wavelengthAxis
                function onRangeChanged() {
                    root.synchronizeFrequencyFromWavelength();
                }
            }

            Connections {
                target: frequencyAxis
                function onRangeChanged() {
                    root.synchronizeWavelengthFromFrequency();
                }
            }

            QAccelPlot.LineCurve {
                name: "Emission spectrum"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.palette.seriesPrimary
                lineWidth: 3
                antialiasingEnabled: true

                Component.onCompleted: {
                    const points = [];
                    for (let i = 0; i <= 600; ++i) {
                        const wavelength = 400 + i * 0.5;
                        const intensity = 0.75 * Math.exp(-Math.pow((wavelength - 450) / 12, 2)) + Math.exp(-Math.pow((wavelength - 532) / 10, 2)) + 0.65 * Math.exp(-Math.pow((wavelength - 635) / 18, 2));
                        points.push(Qt.point(wavelength, intensity));
                    }
                    setData(points);
                }
            }
        }
    }
}
