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

    property bool metricsEnabled: true
    // Created from components, so resetting a page can replace its settings with a fresh object.
    property QtObject lineCurveSettings: lineCurveDefaults.createObject(window)
    property QtObject pointCloudSettings: pointCloudDefaults.createObject(window)
    property QtObject rectangleSeriesSettings: rectangleSeriesDefaults.createObject(window)
    property int fps: 60
    property real averageFrameTimeMs: 16.7
    property int updateRate: 60
    property real longestDataGapMs: 0
    readonly property string activePage: pages.currentName
    readonly property int maximumCount: 10000000
    readonly property var countPresets: [1000, 10000, 100000, 300000, 500000, 1000000, 2000000, 5000000, 10000000]
    readonly property QtObject activeSettings: activePage === "pointCloud" ? pointCloudSettings : activePage === "rectangleSeries" ? rectangleSeriesSettings : lineCurveSettings
    readonly property int activeCount: activeSettings.count
    readonly property real throughputMillions: activeCount * updateRate / 1000000
    readonly property QtObject colorPalette: QAccelPlot.Colors.dark

    function resetActiveSettings() {
        const previous = activeSettings;
        if (activePage === "pointCloud") {
            pointCloudSettings = pointCloudDefaults.createObject(window);
        } else if (activePage === "rectangleSeries") {
            rectangleSeriesSettings = rectangleSeriesDefaults.createObject(window);
        } else {
            lineCurveSettings = lineCurveDefaults.createObject(window);
        }
        previous.destroy();
    }

    width: 1400
    height: 900
    visible: true
    title: "QAccelPlot Performance Showcase"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    Component {
        id: lineCurveDefaults

        LineCurveSettings {}
    }

    Component {
        id: pointCloudDefaults

        PointCloudSettings {}
    }

    Component {
        id: rectangleSeriesDefaults

        RectangleSeriesSettings {}
    }

    FrameDriver {
        running: true
        targetWindow: window
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        ControlPanel {
            Layout.preferredWidth: 240
            Layout.fillHeight: true
            window: window
            colorPalette: window.colorPalette
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            ExampleHeader {
                title: window.activeCount.toLocaleString(Qt.locale("en_US"), "f", 0) + " live " + window.activeSettings.noun
                description: "Display FPS counts frames presented to the screen. Data Update Rate shows how many new datasets are applied to the plot each second."
            }

            GridLayout {
                columns: 4
                columnSpacing: 8
                Layout.fillWidth: true

                Repeater {
                    model: [
                        {
                            label: "DISPLAY FPS",
                            value: window.fps.toString(),
                            detail: "frames presented / second"
                        },
                        {
                            label: "DATA UPDATE RATE",
                            value: window.updateRate.toString() + " Hz",
                            detail: "peak update gap " + window.longestDataGapMs.toFixed(0) + " ms"
                        },
                        {
                            label: "FRAME TIME",
                            value: window.averageFrameTimeMs.toFixed(1) + " ms",
                            detail: "between presents"
                        },
                        {
                            label: "THROUGHPUT",
                            value: window.throughputMillions.toFixed(1) + " M",
                            detail: window.activeSettings.noun + " updated / second"
                        }
                    ]

                    delegate: Rectangle {
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.preferredHeight: 70
                        color: colorPalette.control
                        border.color: colorPalette.outline
                        radius: 4

                        Column {
                            anchors.centerIn: parent
                            spacing: 2

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.label
                                color: colorPalette.textSecondary
                                font.bold: true
                                font.pixelSize: 11
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.value
                                color: colorPalette.text
                                font.bold: true
                                font.pixelSize: 22
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.detail
                                color: colorPalette.textMuted
                                font.pixelSize: 10
                            }
                        }
                    }
                }
            }

            ExamplePages {
                id: pages
                tabs: [
                    { name: "lineCurve", title: "LineCurve" },
                    { name: "pointCloud", title: "PointCloud" },
                    { name: "rectangleSeries", title: "RectangleSeries" }
                ]

                LineCurvePage {
                    settings: window.lineCurveSettings
                }
                PointCloudPage {
                    settings: window.pointCloudSettings
                }
                RectangleSeriesPage {
                    settings: window.rectangleSeriesSettings
                }
            }
        }
    }
}
