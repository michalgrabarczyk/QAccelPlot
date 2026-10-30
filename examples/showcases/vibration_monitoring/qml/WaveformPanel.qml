//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

// Every channel stacked in one plot: the last ten seconds at full sample rate, about four
// million points replaced every frame. Click a channel to analyze it; wheel to zoom down to
// single samples; double-click to show everything again.
DashboardPanel {
    id: panel

    required property QtObject monitor
    readonly property alias plot: plot
    readonly property alias inspector: inspector
    readonly property int channelCount: monitor.channelNames.length
    // Channel 0 is drawn on top, so its center has the highest Y value.
    readonly property real defaultYMin: -0.6
    readonly property real defaultYMax: channelCount - 0.4
    // The channel under the inspector's cursor, or the selected one.
    readonly property int inspectedChannel: isNaN(inspector.cursorY) ? monitor.selectedChannel : channelAt(inspector.cursorY)
    // Zoomed in this far, the inspected channel marks its samples. Markers are drawn for every
    // sample, also outside the view, so the other channels go without.
    readonly property bool samplesVisible: (plot.xAxis.viewportMax - plot.xAxis.viewportMin) * monitor.sampleRate < 120

    function channelAt(plotY) {
        return Math.max(0, Math.min(channelCount - 1, channelCount - 1 - Math.round(plotY)));
    }

    function channelColor(channel) {
        const machineColors = [colorPalette.seriesPrimary, colorPalette.seriesPrimary, colorPalette.seriesCyan, colorPalette.seriesCyan,
                               colorPalette.seriesQuaternary, colorPalette.seriesQuaternary, colorPalette.seriesYellow, colorPalette.seriesMuted];
        return machineColors[channel];
    }

    function resetView() {
        plot.xAxis.viewportMin = -monitor.windowSeconds;
        plot.xAxis.viewportMax = 0;
        plot.yAxis.viewportMin = defaultYMin;
        plot.yAxis.viewportMax = defaultYMax;
    }

    title: "Waveforms"
    detail: channelCount + " channels × " + monitor.windowSeconds + " s at " + (monitor.sampleRate / 1000).toFixed(1) + " kHz"

    QAccelPlot.Plot {
        id: plot

        property point pressPosition

        anchors.fill: parent
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
            viewportMin: -panel.monitor.windowSeconds
            viewportMax: 0
            layoutSize: 46
            label: "Time (s)"
        }

        yAxis: ExampleAxis {
            colorPalette: panel.colorPalette
            viewportMin: panel.defaultYMin
            viewportMax: panel.defaultYMax
            layoutSize: 82
            // Twice the rows, so that the ticker steps by one channel rather than two.
            ticker.tickCount: panel.channelCount * 2
            ticker.subtickCount: 0
            // Semibold keeps thin strokes, such as the bowls of "B", visible at this small size.
            ticker.tickLabelFont: Qt.font({
                pixelSize: 12,
                weight: Font.DemiBold
            })
            ticker.tickLabelFormatter: QAccelPlot.TextTickLabelFormatter {
                labels: panel.monitor.channelNames.slice().reverse()
            }
        }

        onMousePressed: event => pressPosition = Qt.point(event.x, event.y)
        onMouseReleased: event => {
            const moved = Math.abs(event.x - pressPosition.x) + Math.abs(event.y - pressPosition.y);
            if (event.button === Qt.LeftButton && moved < 4 && isInsidePlotArea(event.x, event.y))
                panel.monitor.selectedChannel = panel.channelAt(pixelToDataY(event.y));
        }
        onMouseDoubleClicked: event => {
            event.accepted = true;
            panel.resetView();
        }

        // Shades the row of the channel shown in the spectrum and the spectrogram.
        Item {
            x: plot.plotRect.x
            y: plot.plotRect.y
            width: plot.plotRect.width
            height: plot.plotRect.height
            clip: true

            QAccelPlot.DataAnchor {
                readonly property real center: panel.monitor.channelOffset(panel.monitor.selectedChannel)

                xAxis: plot.xAxis
                yAxis: plot.yAxis
                plotRect: Qt.rect(0, 0, parent.width, parent.height)
                dataX1: plot.xAxis.viewportMin
                dataX2: plot.xAxis.viewportMax
                dataY1: center - 0.5
                dataY2: center + 0.5

                Rectangle {
                    anchors.fill: parent
                    color: panel.colorPalette.focus
                    opacity: 0.1
                }
            }
        }

        Repeater {
            id: waveforms

            model: panel.channelCount

            QAccelPlot.LineCurve {
                required property int index

                objectName: "waveform" + index
                name: panel.monitor.channelNames[index]
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: panel.channelColor(index)
                lineWidth: 1
                hoverEnabled: false
                marker.shape: panel.samplesVisible && index === panel.inspectedChannel ? QAccelPlot.MarkerShape.Circle : QAccelPlot.MarkerShape.None
                marker.size: 2.5
            }
        }

        QAccelPlot.PlotInspector {
            id: inspector

            plot: plot
            includedSeries: waveforms.count === panel.channelCount ? [waveforms.itemAt(panel.inspectedChannel)] : []

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
                rowDelegate: Row {
                    required property bool valid
                    required property string seriesName
                    required property color seriesColor
                    required property real dataPointX
                    required property real dataPointY

                    visible: valid
                    spacing: 6

                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: seriesColor
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        font.pixelSize: 12
                        color: panel.colorPalette.tooltipText
                        text: seriesName + ": " + panel.monitor.acceleration(panel.inspectedChannel, dataPointY).toPrecision(3) + " g at " + dataPointX.toFixed(5) + " s"
                    }
                }
            }
        }
    }
}
