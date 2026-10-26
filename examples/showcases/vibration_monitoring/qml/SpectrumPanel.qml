//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

// Averaged amplitude spectrum of the selected channel on a logarithmic scale, with the channel's
// alarm envelope and a label on the bearing fault's peak.
DashboardPanel {
    id: panel

    required property QtObject monitor
    required property color curveColor
    readonly property alias plot: plot
    readonly property alias inspector: inspector
    // Empty space right of the plot, as wide as the spectrogram's color bar below.
    property real rightGutter: 0
    readonly property real defaultYMin: 2e-4
    readonly property real defaultYMax: 3
    // Hover, or the scripted demo, highlights the fault label.
    property bool peakLabelHighlighted: peakHover.hovered

    function withAlpha(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha);
    }

    function resetView() {
        plot.xAxis.viewportMin = 0;
        plot.xAxis.viewportMax = monitor.maxFrequencyHz;
        plot.yAxis.viewportMin = defaultYMin;
        plot.yAxis.viewportMax = defaultYMax;
    }

    title: "Spectrum · " + monitor.channelNames[monitor.selectedChannel]
    detail: "1 s average"

    QAccelPlot.Plot {
        id: plot

        anchors.fill: parent
        anchors.rightMargin: panel.rightGutter
        // The panel already frames the plot; a small padding leaves more room for data.
        padding: 4
        legendVisible: false
        plotAreaColor: panel.colorPalette.plotArea
        axesAreaColor: panel.colorPalette.axesArea
        border.color: panel.colorPalette.plotBorder
        grid.gridColor: panel.colorPalette.grid
        grid.subGridVisible: false

        xAxis: ExampleAxis {
            colorPalette: panel.colorPalette
            viewportMin: 0
            viewportMax: panel.monitor.maxFrequencyHz
            // The spectrogram below names the shared frequency axis.
            layoutSize: 30
        }

        yAxis: ExampleAxis {
            colorPalette: panel.colorPalette
            logScale: true
            viewportMin: panel.defaultYMin
            viewportMax: panel.defaultYMax
            layoutSize: 46
            labelPadding: 36
            label: "g"
        }

        onMouseDoubleClicked: event => {
            event.accepted = true;
            panel.resetView();
        }

        // The selected channel's alarm envelope, 6 dB above its healthy baseline and posted from C++.
        // The band fills from the limit to far above the plot.
        QAccelPlot.BandSeries {
            objectName: "alarmEnvelope"
            xAxis: plot.xAxis
            yAxis: plot.yAxis
            color: panel.withAlpha(panel.colorPalette.statusError, 0.16)
            edges.width: 1
            edges.color: panel.withAlpha(panel.colorPalette.statusError, 0.7)
            hoverEnabled: false
        }

        QAccelPlot.LineCurve {
            id: spectrum

            objectName: "spectrumCurve"
            name: "Spectrum"
            xAxis: plot.xAxis
            yAxis: plot.yAxis
            color: panel.curveColor
            lineWidth: 1.5
            hoverEnabled: false
            effects: [
                QAccelPlot.GradientFill {
                    direction: Qt.Vertical
                    opacity: 0.45
                    gradient: Gradient {
                        GradientStop {
                            position: 0
                            color: panel.withAlpha(panel.curveColor, 0)
                        }
                        GradientStop {
                            position: 1
                            color: panel.curveColor
                        }
                    }
                }
            ]
        }

        // Clips the fault label to the plot area while zoomed.
        Item {
            x: plot.plotRect.x
            y: plot.plotRect.y
            width: plot.plotRect.width
            height: plot.plotRect.height
            clip: true
            z: 1

            QAccelPlot.DataAnchor {
                id: peakAnchor

                xAxis: plot.xAxis
                yAxis: plot.yAxis
                plotRect: Qt.rect(0, 0, parent.width, parent.height)
                dataX1: panel.monitor.bpfoHz
                dataX2: panel.monitor.bpfoHz
                dataY1: 0.6
                dataY2: 0.6
                hoverThreshold: 40

                HoverHandler {
                    id: peakHover
                }

                Rectangle {
                    id: peakLabel

                    x: -4
                    // Above the anchor, but never past the top of the plot area, where it would be clipped.
                    y: Math.max(-height, 1 - peakAnchor.y)
                    width: peakText.implicitWidth + 12
                    height: peakText.implicitHeight + 6
                    radius: 3
                    color: panel.peakLabelHighlighted ? panel.colorPalette.statusError : panel.colorPalette.tooltipBackground
                    border.color: panel.colorPalette.statusError

                    Text {
                        id: peakText

                        anchors.centerIn: parent
                        font.pixelSize: 12
                        font.bold: panel.peakLabelHighlighted
                        color: panel.peakLabelHighlighted ? panel.colorPalette.textOnAccent : panel.colorPalette.tooltipText
                        text: "Bearing fault · " + panel.monitor.bpfoHz.toFixed(0) + " Hz"
                    }
                }

                // A leader line from the label down to the peak.
                Rectangle {
                    x: -0.5
                    y: Math.max(0, peakLabel.y + peakLabel.height)
                    width: 1
                    height: Math.max(0, plot.yAxis ? plot.dataToPixelY(0.16) - plot.dataToPixelY(0.6) - y : 0)
                    color: panel.colorPalette.statusError
                }
            }
        }

        QAccelPlot.PlotInspector {
            id: inspector

            plot: plot
            includedSeries: [spectrum]
            snapToDataPoint: true

            QAccelPlot.Crosshair {
                color: panel.colorPalette.hover
                horizontal: false
            }

            QAccelPlot.InspectionMarkers {
                borderColor: panel.colorPalette.plotArea
            }

            QAccelPlot.InspectionTooltip {
                color: panel.colorPalette.tooltipBackground
                border.color: panel.colorPalette.legendBorder
                textColor: panel.colorPalette.tooltipText
                rowDelegate: Text {
                    required property bool valid
                    required property real dataPointX
                    required property real dataPointY

                    visible: valid
                    font.pixelSize: 12
                    color: panel.colorPalette.tooltipText
                    text: dataPointX.toFixed(1) + " Hz  ·  " + dataPointY.toPrecision(3) + " g"
                }
            }
        }
    }
}
