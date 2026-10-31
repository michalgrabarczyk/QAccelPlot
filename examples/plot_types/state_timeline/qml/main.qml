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
    // Set from C++ before loading: milliseconds since the Unix epoch.
    required property real timelineStart
    required property real timelineEnd
    readonly property real dayMs: 24 * 60 * 60 * 1000
    readonly property var machineNames: ["Press 1", "Press 2", "Press 3", "Lathe 1", "Lathe 2", "Mill 1", "Mill 2", "Mill 3", "Welder 1", "Welder 2", "Packer 1", "Packer 2"]
    // Set from C++: the value of each state in its State enum, which the data carries as the category.
    required property var stateValues

    width: 1200
    height: 800
    visible: true
    title: "State Timeline Example"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    function formatTime(ms) {
        return Qt.formatDateTime(new Date(ms), "ddd d MMM HH:mm");
    }

    function formatDuration(ms) {
        const minutes = Math.round(ms / 60000);
        return minutes >= 60 ? Math.floor(minutes / 60) + " h " + minutes % 60 + " min" : minutes + " min";
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        ExampleHeader {
            title: "State timeline"
            subtitle: "Two weeks of machine states in one RectangleSeries, colored by category, over maintenance windows."
            interactions: [
                "Hover a state to read it",
                "Drag <b>Minimum width</b> to set how narrow short alarms get when zoomed out",
                "Toggle <b>Outlines</b> or the full-height <b>Maintenance</b> windows"
            ]
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            // One row per category of the states series.
            Repeater {
                model: QAccelPlot.LegendModel {
                    series: [states]
                }

                RowLayout {
                    required property string name
                    required property var entry
                    spacing: 4

                    Rectangle {
                        implicitWidth: 12
                        implicitHeight: 12
                        color: parent.entry.color
                    }
                    Label {
                        text: parent.name
                        color: colorPalette.text
                    }
                }
            }

            CheckBox {
                id: outlineCheck
                text: "Outlines"
                checked: true
            }

            CheckBox {
                id: maintenanceCheck
                text: "Maintenance"
                checked: true
            }

            Label {
                text: "Minimum width " + minimumWidthSlider.value.toFixed(1) + " px"
                color: colorPalette.text
            }
            Slider {
                id: minimumWidthSlider
                from: 0
                to: 4
                stepSize: 0.5
                value: 1
                Layout.preferredWidth: 140
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            border.color: colorPalette.plotBorder
            border.width: 2
            legendVisible: false
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                viewportMin: window.timelineStart
                viewportMax: window.timelineEnd
                label: "Time"
                ticker.tickLabelFormatter: QAccelPlot.DateTimeTickLabelFormatter {
                    dateTimeFormat: "ddd d MMM"
                }
            }

            yAxis: ExampleAxis {
                viewportMin: -0.5
                viewportMax: window.machineNames.length - 0.5
                labelPadding: 60
                layoutSize: 80
                label: "Machine"
                ticker.tickCount: window.machineNames.length
                ticker.subtickCount: 0
                ticker.tickLabelFormatter: QAccelPlot.TextTickLabelFormatter {
                    labels: window.machineNames
                }
            }

            // Omitting y1 and y2 makes each window span the full plot height.
            QAccelPlot.RectangleSeries {
                id: maintenance
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                visible: maintenanceCheck.checked
                color: colorPalette.annotationRangeFill
                hoverColor: Qt.lighter(colorPalette.annotationRangeFill, 1.6)

                Component.onCompleted: {
                    const windows = [];
                    for (let day = 2; day < 14; day += 4) {
                        const dayStart = window.timelineStart + day * window.dayMs;
                        windows.push({ x1: dayStart + 5 * 3600000, x2: dayStart + 9 * 3600000 });
                    }
                    setData(windows);
                }
            }

            // Filled from C++ with setData(), which also passes one State value per interval as its category.
            QAccelPlot.RectangleSeries {
                id: states
                objectName: "states"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: colorPalette.seriesMuted
                categories: [
                    QAccelPlot.Category { value: window.stateValues.Running; name: "Running"; color: colorPalette.statusGood },
                    QAccelPlot.Category { value: window.stateValues.Idle; name: "Idle"; color: colorPalette.seriesMuted },
                    QAccelPlot.Category { value: window.stateValues.Setup; name: "Setup"; color: colorPalette.seriesPrimary },
                    QAccelPlot.Category { value: window.stateValues.Alarm; name: "Alarm"; color: colorPalette.statusError }
                ]
                border.width: outlineCheck.checked ? 1 : 0
                border.color: colorPalette.plotArea
                minimumWidth: minimumWidthSlider.value
                hoverColor: colorPalette.text
            }

            Item {
                id: overlay
                x: plot.plotRect.x
                y: plot.plotRect.y
                width: plot.plotRect.width
                height: plot.plotRect.height
                clip: true

                QAccelPlot.DataAnchor {
                    id: tooltipAnchor

                    readonly property var hovered: states.hoveredIndex >= 0 ? states.rectangleAt(states.hoveredIndex) : null
                    // Center of the part of the interval that is in view.
                    readonly property real centerX: hovered ? (Math.max(hovered.x1, plot.xAxis.viewportMin) + Math.min(hovered.x2, plot.xAxis.viewportMax)) / 2 : 0

                    visible: hovered !== null
                    xAxis: plot.xAxis
                    yAxis: plot.yAxis
                    plotRect: Qt.rect(0, 0, overlay.width, overlay.height)
                    dataX1: centerX
                    dataX2: centerX
                    dataY1: hovered ? hovered.y2 : 0
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
                                const interval = tooltipAnchor.hovered;
                                if (!interval) {
                                    return "";
                                }
                                const machine = window.machineNames[Math.round((interval.y1 + interval.y2) / 2)];
                                return machine + " · " + interval.categoryName + "\n" + window.formatTime(interval.x1) + " – "
                                    + window.formatTime(interval.x2) + " (" + window.formatDuration(interval.x2 - interval.x1) + ")";
                            }
                        }
                    }
                }
            }
        }
    }
}
