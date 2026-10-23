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

Item {
    id: root

    required property var colorPalette
    readonly property real tickSeconds: 0.1
    readonly property real windowSeconds: 60
    // Set points in °C, each held for one schedule period.
    readonly property var schedule: [20, 23.5, 21.5, 25, 19, 22.5, 24, 20.5]
    readonly property real schedulePeriod: 7
    property real time: 0
    property real temperature: 18
    // Data points of the set point line: one where the set point changes.
    property var setPointChanges: []

    function setPointAt(t) {
        return schedule[Math.floor(t / schedulePeriod) % schedule.length];
    }

    // Advances the room one tick: it warms or cools toward the set point, with sensor noise.
    function advance() {
        time += tickSeconds;
        const setPoint = setPointAt(time);
        temperature += (setPoint - temperature) * tickSeconds / 4;
        const noise = 0.05 * (Math.sin(13.1 * time) + Math.sin(7.7 * time + 1));
        const last = setPointChanges[setPointChanges.length - 1];
        if (!last || last.y !== setPoint) {
            setPointChanges.push(Qt.point(time, setPoint));
        }
        // Keep the change that holds at the window start, drop the ones before it.
        while (setPointChanges.length > 1 && setPointChanges[1].x < time - windowSeconds) {
            setPointChanges.shift();
        }
        return Qt.point(time, temperature + noise);
    }

    // Draws the set point steps; holding the last one until now is up to the data.
    function showSetPoint() {
        const points = setPointChanges.slice();
        if (holdCheck.checked) {
            points.push(Qt.point(time, setPointAt(time)));
        }
        setPointCurve.setData(points);
    }

    function tick() {
        const reading = advance();
        temperatureCurve.appendData(reading.x, reading.y);
        showSetPoint();
    }

    Component.onCompleted: {
        // Start with 40 seconds of history.
        const readings = [];
        for (let i = 0; i < 400; ++i) {
            readings.push(advance());
        }
        temperatureCurve.setData(readings);
        showSetPoint();
    }

    Timer {
        interval: root.tickSeconds * 1000
        repeat: true
        running: !pauseCheck.checked
        onTriggered: root.tick()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: "A room heater's set point as an After step line over the measured temperature. The set point is stored only where it changes; hover the plot to read the value each step holds."
            color: root.colorPalette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            spacing: 12

            CheckBox {
                id: holdCheck
                text: "Hold the last set point until now"
                checked: true
                onToggled: root.showSetPoint()
            }
            CheckBox {
                id: pauseCheck
                text: "Pause"
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                viewportMin: Math.max(0, root.time - root.windowSeconds)
                viewportMax: Math.max(root.windowSeconds, root.time)
                label: "Time (s)"
            }

            yAxis: ExampleAxis {
                viewportMin: 16
                viewportMax: 27
                label: "Temperature (°C)"
            }

            QAccelPlot.LineCurve {
                id: temperatureCurve
                name: "Temperature"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: root.colorPalette.seriesPrimary
                lineWidth: 1.5
            }

            QAccelPlot.LineCurve {
                id: setPointCurve
                name: "Set point"
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                stepMode: QAccelPlot.StepMode.After
                color: root.colorPalette.seriesSecondary
                lineWidth: 2
            }

            QAccelPlot.PlotInspector {
                plot: plot
                mode: QAccelPlot.PlotInspector.NearestX
                interpolate: true

                QAccelPlot.Crosshair {}

                QAccelPlot.InspectionMarkers {}

                QAccelPlot.InspectionTooltip {}
            }
        }
    }
}
