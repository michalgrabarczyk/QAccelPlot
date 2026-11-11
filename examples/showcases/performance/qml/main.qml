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
    property QtObject barSeriesSettings: barSeriesDefaults.createObject(window)
    property QtObject heatmapSeriesSettings: heatmapSeriesDefaults.createObject(window)
    property int fps: 60
    property real averageFrameTimeMs: 16.7
    property int updateRate: 60
    property real longestDataGapMs: 0
    property real dataCopyMs: 0
    // Set while the visible page waits for data generated for its current settings.
    property bool loading: true
    readonly property string activePage: pages.currentName
    readonly property QtObject activeSettings: activePage === "pointCloud" ? pointCloudSettings : activePage === "rectangleSeries" ? rectangleSeriesSettings : activePage === "barSeries" ? barSeriesSettings : activePage === "heatmapSeries" ? heatmapSeriesSettings : lineCurveSettings
    readonly property int activeCount: activeSettings.count
    readonly property real throughputMillions: activeCount * updateRate / 1000000
    readonly property QtObject colorPalette: QAccelPlot.Colors.dark

    function resetActiveSettings() {
        const previous = activeSettings;
        if (activePage === "pointCloud") {
            pointCloudSettings = pointCloudDefaults.createObject(window);
        } else if (activePage === "rectangleSeries") {
            rectangleSeriesSettings = rectangleSeriesDefaults.createObject(window);
        } else if (activePage === "barSeries") {
            barSeriesSettings = barSeriesDefaults.createObject(window);
        } else if (activePage === "heatmapSeries") {
            heatmapSeriesSettings = heatmapSeriesDefaults.createObject(window);
        } else {
            lineCurveSettings = lineCurveDefaults.createObject(window);
        }
        previous.destroy();
    }

    width: 1400
    height: 900
    visible: true
    title: "QAccelPlot Performance Lab"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    component MetricCard: Rectangle {
        id: card

        required property string label
        required property string value
        required property string detail

        // Equal shares of the row, whatever the text widths.
        Layout.fillWidth: true
        Layout.preferredWidth: 1
        Layout.preferredHeight: 70
        color: QAccelPlot.Colors.dark.control
        border.color: QAccelPlot.Colors.dark.outline
        radius: 4

        Column {
            anchors.centerIn: parent
            spacing: 2

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: card.label
                color: QAccelPlot.Colors.dark.textSecondary
                font.bold: true
                font.pixelSize: 11
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: card.value
                color: QAccelPlot.Colors.dark.text
                font.bold: true
                font.pixelSize: 22
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: card.detail
                color: QAccelPlot.Colors.dark.textMuted
                font.pixelSize: 10
            }
        }
    }

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

    Component {
        id: barSeriesDefaults

        BarSeriesSettings {}
    }

    Component {
        id: heatmapSeriesDefaults

        HeatmapSeriesSettings {}
    }

    FrameDriver {
        running: true
        targetWindow: window
    }

    // Changing text or pressing a control adds and removes scene graph nodes, and that makes the
    // window's renderer re-upload every series. The control panel, the header, and the metrics
    // render in layers, each with a renderer of its own.
    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        ControlPanel {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            layer.enabled: true
            window: window
            colorPalette: window.colorPalette
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            ExampleHeader {
                layer.enabled: true
                title: window.activeCount.toLocaleString(Qt.locale("en_US"), "f", 0) + " live " + window.activeSettings.noun
                subtitle: "Display FPS counts frames presented to the screen. Data Update Rate shows how many new datasets are applied to the plot each second."
            }

            RowLayout {
                spacing: 8
                layer.enabled: true
                Layout.fillWidth: true

                MetricCard {
                    label: "DISPLAY FPS"
                    value: window.fps.toString()
                    detail: "frames presented / second"
                }

                MetricCard {
                    label: "DATA UPDATE RATE"
                    value: window.updateRate.toString() + " Hz"
                    detail: "peak update gap " + window.longestDataGapMs.toFixed(0) + " ms"
                }

                MetricCard {
                    label: "FRAME TIME"
                    value: window.averageFrameTimeMs.toFixed(1) + " ms"
                    detail: "between presents"
                }

                MetricCard {
                    label: "THROUGHPUT"
                    value: window.throughputMillions.toFixed(1) + " M"
                    detail: window.activeSettings.noun + " updated / second"
                }

                MetricCard {
                    label: "DATA COPY"
                    value: window.dataCopyMs.toFixed(1) + " ms"
                    detail: "worker time per dataset"
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ExamplePages {
                    id: pages
                    anchors.fill: parent
                    tabBackground: plainTabBackground
                    tabs: [
                        { name: "lineCurve", title: "LineCurve" },
                        { name: "pointCloud", title: "PointCloud" },
                        { name: "rectangleSeries", title: "RectangleSeries" },
                        { name: "barSeries", title: "BarSeries" },
                        { name: "heatmapSeries", title: "HeatmapSeries" }
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
                    BarSeriesPage {
                        settings: window.barSeriesSettings
                    }
                    HeatmapSeriesPage {
                        settings: window.heatmapSeriesSettings
                    }
                }

                // Covers the page while it waits for data generated for its current settings. The
                // cover is created and destroyed in a layer: hiding or showing an item, even in a
                // layer, would make the window's renderer re-upload every series.
                Item {
                    x: pages.pageArea.x
                    y: pages.pageArea.y
                    width: pages.pageArea.width
                    height: pages.pageArea.height
                    layer.enabled: true

                    Loader {
                        anchors.fill: parent
                        active: window.loading
                        sourceComponent: loadingCover
                    }
                }

                // Replaces the Material ripple, whose hover adds scene nodes, which re-uploads every
                // series, and animates them on the render thread, which without vsync draws
                // thousands of frames per second. Both colors are opaque: a transparent Rectangle
                // removes its node.
                Component {
                    id: plainTabBackground

                    Rectangle {
                        readonly property Item button: parent

                        implicitHeight: button.Material.touchTarget
                        color: button.hovered || button.down ? Qt.tint(button.Material.backgroundColor, button.Material.rippleColor) : button.Material.backgroundColor
                    }
                }

                Component {
                    id: loadingCover

                    Rectangle {
                        color: Qt.rgba(window.colorPalette.window.r, window.colorPalette.window.g, window.colorPalette.window.b, 0.75)

                        Column {
                            anchors.centerIn: parent
                            spacing: 12

                            BusyIndicator {
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Label {
                                text: "Generating " + window.activeCount.toLocaleString(Qt.locale("en_US"), "f", 0) + " " + window.activeSettings.noun
                                color: window.colorPalette.textSecondary
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }
                }
            }
        }
    }
}
