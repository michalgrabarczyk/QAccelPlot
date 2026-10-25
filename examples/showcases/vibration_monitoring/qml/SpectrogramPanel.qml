//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick
import QAccelPlot as QAccelPlot

// Short-time spectra of the selected channel over the same ten seconds as the waveforms, ten
// per second, in decibels relative to 1 g. Frequency runs along X, matching the spectrum above,
// so a peak can be followed straight down into its history; the newest spectrum is at the top.
DashboardPanel {
    id: panel

    required property QtObject monitor
    // The spectrum's frequency axis. Zooming or panning either plot's frequencies moves both.
    required property QAccelPlot.Axis frequencyAxis
    readonly property alias plot: plot
    // Space right of the plot for the level key. The spectrum above keeps the same space, so the
    // frequency axes of both plots line up.
    readonly property real keyWidth: levelKey.width + 6
    readonly property real defaultLevelMin: -60
    readonly property real defaultLevelMax: -10
    // Shared by the spectrogram and its key. The key's level axis sets the range, so dragging or
    // wheeling that axis recolors both. A set range also spares a scan of every cell per update.
    readonly property QAccelPlot.Colormap levels: QAccelPlot.Colormap {
        preset: QAccelPlot.Colormap.Inferno
        min: levelKey.yAxis.viewportMin
        max: levelKey.yAxis.viewportMax
    }

    function resetView() {
        plot.xAxis.viewportMin = 0;
        plot.xAxis.viewportMax = monitor.maxFrequencyHz;
        plot.yAxis.viewportMin = -monitor.windowSeconds;
        plot.yAxis.viewportMax = 0;
    }

    title: "Spectrogram · " + monitor.channelNames[monitor.selectedChannel]
    detail: "dB re 1 g"

    // The color key: how many spectrogram cells in view fall in each 2 dB band, each bar in its
    // band's color. Drag or wheel it to change the color range; double-click to reset it.
    QAccelPlot.Plot {
        id: levelKey

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        // Ends where the spectrogram's frequency axis starts, so both plot areas span the same height.
        anchors.bottomMargin: plot.xAxis.layoutSize
        anchors.right: parent.right
        width: 86
        padding: 4
        legendVisible: false
        plotAreaColor: panel.colorPalette.plotArea
        axesAreaColor: panel.colorPalette.axesArea
        border.color: panel.colorPalette.plotBorder
        grid.gridVisible: false
        grid.subGridVisible: false

        // Log counts keep the few tonal cells visible next to the many at the noise floor. The
        // counts only shape the key, so their axis is hidden.
        xAxis: ExampleAxis {
            colorPalette: panel.colorPalette
            visible: false
            layoutSize: 0
            logScale: true
            viewportMin: 1
            viewportMax: 2e4
        }

        yAxis: ExampleAxis {
            colorPalette: panel.colorPalette
            viewportMin: panel.defaultLevelMin
            viewportMax: panel.defaultLevelMax
            layoutSize: 32
        }

        onMouseDoubleClicked: event => {
            event.accepted = true;
            levelKey.xAxis.viewportMin = 1;
            levelKey.xAxis.viewportMax = 2e4;
            levelKey.yAxis.viewportMin = panel.defaultLevelMin;
            levelKey.yAxis.viewportMax = panel.defaultLevelMax;
        }

        // Filled from C++ by LevelHistogram, which recounts as the spectrogram updates, pans, and zooms.
        QAccelPlot.BarSeries {
            objectName: "levelHistogram"
            xAxis: levelKey.xAxis
            yAxis: levelKey.yAxis
            orientation: Qt.Horizontal
            colorSource: QAccelPlot.BarSeries.Position
            colormap: panel.levels
            hoverEnabled: false
        }
    }

    QAccelPlot.Plot {
        id: plot

        anchors.fill: parent
        anchors.rightMargin: panel.keyWidth
        // The panel already frames the plot; a small padding leaves more room for data.
        padding: 4
        legendVisible: false
        plotAreaColor: panel.colorPalette.plotArea
        axesAreaColor: panel.colorPalette.axesArea
        border.color: panel.colorPalette.plotBorder
        grid.gridVisible: false
        grid.subGridVisible: false

        xAxis: ExampleAxis {
            colorPalette: panel.colorPalette
            viewportMin: 0
            viewportMax: panel.monitor.maxFrequencyHz
            layoutSize: 46
            label: "Frequency (Hz)"
        }

        // As wide as the spectrum's level axis, so both plot areas line up.
        yAxis: ExampleAxis {
            colorPalette: panel.colorPalette
            viewportMin: -panel.monitor.windowSeconds
            viewportMax: 0
            layoutSize: 46
            labelPadding: 36
            label: "s"
        }

        onMouseDoubleClicked: event => {
            event.accepted = true;
            panel.resetView();
        }

        QAccelPlot.HeatmapSeries {
            id: spectrogram

            objectName: "spectrogramSeries"
            xAxis: plot.xAxis
            yAxis: plot.yAxis
            // Each column is centered on its frequency bin; each row ends at its analysis tick.
            xMin: -panel.monitor.spectrogramBinHz / 2
            xMax: panel.monitor.maxFrequencyHz + panel.monitor.spectrogramBinHz / 2
            yMin: -panel.monitor.windowSeconds
            yMax: 0
            colormap: panel.levels
        }
    }

    // Copies each frequency range to the other axis; setting an equal value emits nothing, so this settles.
    Connections {
        target: panel.frequencyAxis

        function onViewportMinChanged() {
            plot.xAxis.viewportMin = panel.frequencyAxis.viewportMin;
        }

        function onViewportMaxChanged() {
            plot.xAxis.viewportMax = panel.frequencyAxis.viewportMax;
        }
    }

    Connections {
        target: plot.xAxis

        function onViewportMinChanged() {
            panel.frequencyAxis.viewportMin = plot.xAxis.viewportMin;
        }

        function onViewportMaxChanged() {
            panel.frequencyAxis.viewportMax = plot.xAxis.viewportMax;
        }
    }
}
