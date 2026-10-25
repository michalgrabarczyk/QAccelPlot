//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

// Overall RMS level of every channel over the last second, one bar beside each waveform row,
// colored green, amber, or red by its value. Click a bar to analyze its channel.
DashboardPanel {
    id: panel

    required property QtObject monitor
    // The waveform plot's channel axis; the bars follow its range to stay beside their rows.
    required property QAccelPlot.Axis channelAxis
    readonly property real warningLevel: 0.2
    readonly property real alarmLevel: 0.35
    readonly property real fullScale: 0.6

    title: "RMS"
    detail: "g, 1 s"

    QAccelPlot.Plot {
        id: plot

        anchors.fill: parent
        // The panel already frames the plot; a small padding leaves more room for data.
        padding: 4
        legendVisible: false
        plotAreaColor: panel.colorPalette.plotArea
        axesAreaColor: panel.colorPalette.axesArea
        border.color: panel.colorPalette.plotBorder
        grid.gridColor: panel.colorPalette.grid
        grid.subGridVisible: false
        grid.gridHorizontalLinesVisible: false

        xAxis: ExampleAxis {
            colorPalette: panel.colorPalette
            viewportMin: 0
            // Headroom past the color scale's end leaves room for the label after the longest bar.
            viewportMax: 0.8
            layoutSize: 46
            ticker.tickCount: 2
            ticker.subtickCount: 0
        }

        yAxis: ExampleAxis {
            colorPalette: panel.colorPalette
            visible: false
            layoutSize: 0
            viewportMin: panel.channelAxis.viewportMin
            viewportMax: panel.channelAxis.viewportMax
        }

        QAccelPlot.BarSeries {
            id: rms

            objectName: "rmsSeries"
            xAxis: plot.xAxis
            yAxis: plot.yAxis
            orientation: Qt.Horizontal
            barWidth: 0.55
            hoverEnabled: false
            colormap: QAccelPlot.Colormap {
                min: 0
                max: panel.fullScale
                // Green below the warning level, amber below the alarm level, red above.
                stops: [
                    GradientStop {
                        position: 0
                        color: panel.colorPalette.statusGood
                    },
                    GradientStop {
                        position: (panel.warningLevel - 0.02) / panel.fullScale
                        color: panel.colorPalette.statusGood
                    },
                    GradientStop {
                        position: panel.warningLevel / panel.fullScale
                        color: panel.colorPalette.statusWarning
                    },
                    GradientStop {
                        position: (panel.alarmLevel - 0.02) / panel.fullScale
                        color: panel.colorPalette.statusWarning
                    },
                    GradientStop {
                        position: panel.alarmLevel / panel.fullScale
                        color: panel.colorPalette.statusError
                    },
                    GradientStop {
                        position: 1
                        color: panel.colorPalette.statusError
                    }
                ]
            }
            transition: QAccelPlot.MorphTransition {
                duration: 90
            }
        }

        // The level of each bar, after its end, or inside it when the bar reaches the right edge.
        Repeater {
            model: rms.count

            Text {
                required property int index
                readonly property var bar: rms.dataRevision >= 0 ? rms.barAt(index) : ({})
                readonly property real barEnd: plot.dataToPixelX(bar.value)
                readonly property bool fitsAfter: barEnd + 4 + width <= plot.plotRect.right - 2

                x: fitsAfter ? barEnd + 4 : barEnd - width - 4
                y: plot.dataToPixelY(bar.position) - height / 2
                visible: y > plot.plotRect.top && y + height < plot.plotRect.bottom
                font.pixelSize: 11
                color: fitsAfter ? panel.colorPalette.text : panel.colorPalette.textOnAccent
                text: bar.value.toFixed(2)
            }
        }

        // The strip only shows levels: clicks select a channel, and the wheel does not zoom it.
        MouseArea {
            x: plot.plotRect.x
            y: plot.plotRect.y
            width: plot.plotRect.width
            height: plot.plotRect.height
            onClicked: mouse => {
                const channel = panel.monitor.channelNames.length - 1 - Math.round(plot.pixelToDataY(mouse.y + y));
                panel.monitor.selectedChannel = channel;
            }
            onWheel: wheel => wheel.accepted = true
        }
    }
}
