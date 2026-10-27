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
import QtQuick.Dialogs
import QtQuick.Layouts
import QAccelPlot as QAccelPlot

Window {
    id: window

    readonly property QtObject colorPalette: QAccelPlot.Colors.dark
    property string status: ""

    // Daily irradiance and panel output of a small solar installation.
    function solarPoints(peak, width, noise) {
        const points = [];
        for (let hour = 0; hour <= 24; hour += 0.25) {
            const daylight = Math.max(0, Math.exp(-Math.pow((hour - 13) / width, 2)) - 0.02);
            points.push(Qt.point(hour, peak * daylight * (1 + noise * Math.sin(hour * 7.1))));
        }
        return points;
    }

    width: 900
    height: 640
    visible: true
    title: "QAccelPlot Export"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    QAccelPlot.PlotExporter {
        id: exporter
        plot: plot
        scale: scaleBox.currentValue
        quality: Math.round(qualitySlider.value)
        transparentBackground: transparentBox.checked
        onFinished: (ok, path, error) => window.status = !ok ? error : path !== "" ? "Saved " + path : "Copied to the clipboard"
    }

    FileDialog {
        id: saveDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["PNG image (*.png)", "JPEG image (*.jpg)", "WebP image (*.webp)", "TIFF image (*.tif)"]
        // save() picks the format from the suffix, so a name typed without one gets the filter's.
        defaultSuffix: selectedNameFilter.extensions[0]
        onAccepted: exporter.save(selectedFile)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Export"
            subtitle: "PlotExporter saves a plot as a raster image or copies it to the clipboard."
            interactions: [
                "Click <b>Save as…</b> to save the plot as an image, or <b>Copy image</b> to copy it",
                "Set <b>Scale</b> to the image pixels per logical pixel; <b>Screen</b> matches the display",
                "Check <b>Transparent background</b> to leave out the plot and axes area colors",
                "Drag <b>Quality</b> to set the compression of lossy formats such as JPEG and WebP"
            ]
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "Scale"
            }
            ComboBox {
                id: scaleBox
                textRole: "text"
                valueRole: "value"
                model: [
                    { text: "Screen", value: 0 },
                    { text: "1×", value: 1 },
                    { text: "2×", value: 2 },
                    { text: "4×", value: 4 }
                ]
                Layout.preferredWidth: 110
                Material.background: colorPalette.plotArea
            }
            CheckBox {
                id: transparentBox
                text: "Transparent background"
            }
            Label {
                text: "Quality " + Math.round(qualitySlider.value)
            }
            Slider {
                id: qualitySlider
                from: 10
                to: 100
                stepSize: 1
                value: 90
                Layout.preferredWidth: 140
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                text: "Save as…"
                enabled: !exporter.busy
                onClicked: saveDialog.open()
            }
            Button {
                text: "Copy image"
                enabled: !exporter.busy
                onClicked: exporter.copyToClipboard()
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                viewportMin: 0
                viewportMax: 24
                label: "Time of day (h)"
                ticker.tickCount: 7
            }

            yAxis: ExampleAxis {
                viewportMin: 0
                viewportMax: 1000
                label: "Power (W)"
            }

            QAccelPlot.LineCurve {
                name: "Irradiance (W/m²)"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: colorPalette.seriesSecondary
                lineWidth: 2
                lineStyle: QAccelPlot.DashLine {
                    pattern: [8, 5]
                }
                Component.onCompleted: setData(window.solarPoints(920, 3.6, 0))
            }

            QAccelPlot.LineCurve {
                name: "Panel output"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: colorPalette.seriesPrimary
                lineWidth: 3
                effects: QAccelPlot.GradientFill {
                    direction: Qt.Vertical
                    baseline: QAccelPlot.GradientFillBaseline.Value
                    baselineValue: 0
                    opacity: 0.4
                    gradient: Gradient {
                        GradientStop { position: 0; color: "transparent" }
                        GradientStop { position: 1; color: colorPalette.seriesPrimary }
                    }
                }
                Component.onCompleted: setData(window.solarPoints(760, 3.2, 0.04))
            }
        }

        Label {
            text: window.status
            color: colorPalette.textSecondary
            elide: Text.ElideMiddle
            Layout.fillWidth: true
        }
    }
}
