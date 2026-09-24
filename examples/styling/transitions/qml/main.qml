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
    property int duration: 900
    property int easingType: Easing.InOutCubic
    property bool nextShapeIsB: true

    function shapeA() {
        const points = [];
        for (let i = 0; i <= 80; ++i) {
            const x = i * 8 / 80;
            points.push(Qt.point(x, Math.sin(x)));
        }
        return points;
    }

    function shapeB() {
        const points = [];
        for (let i = 0; i <= 80; ++i) {
            const x = i * 8 / 80;
            points.push(Qt.point(x, 0.65 * Math.sin(2 * x + 0.4) + 0.25 * Math.cos(0.5 * x)));
        }
        return points;
    }

    function animateTo(transition) {
        transitionCurve.transition = transition;
        transitionCurve.setData(nextShapeIsB ? shapeB() : shapeA());
        nextShapeIsB = !nextShapeIsB;
    }

    width: 900
    height: 620
    visible: true
    title: "QAccelPlot Transitions"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Transitions"
            description: "Click Morph or Draw to animate the curve between two waveform shapes. Adjust the duration and easing to compare the transitions."
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "Morph"
                Material.background: colorPalette.plotArea
                onClicked: window.animateTo(morphTransition)
            }

            Button {
                text: "Draw"
                Material.background: colorPalette.plotArea
                onClicked: window.animateTo(drawTransition)
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                text: "Duration " + window.duration + " ms"
            }

            Slider {
                from: 200
                to: 2400
                stepSize: 100
                value: window.duration
                Layout.preferredWidth: 180
                onMoved: window.duration = value
            }

            ComboBox {
                model: ["Linear", "In/out cubic", "Out elastic"]
                currentIndex: 1
                Layout.preferredWidth: 140
                Material.background: colorPalette.plotArea
                onActivated: window.easingType = [Easing.Linear, Easing.InOutCubic, Easing.OutElastic][currentIndex]
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                viewportMin: 0
                viewportMax: 8
                dataMin: 0
                dataMax: 8
                label: "x"
            }

            yAxis: ExampleAxis {
                viewportMin: -1.2
                viewportMax: 1.2
                dataMin: -1.2
                dataMax: 1.2
                axisTitlePadding: 40
                layoutSize: 60
                label: "Value"
            }

            QAccelPlot.LineCurve {
                id: transitionCurve
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: colorPalette.seriesPrimary
                lineWidth: 3
                transition: morphTransition
                Component.onCompleted: setData(window.shapeA())
            }

            QAccelPlot.MorphTransition {
                id: morphTransition
                duration: window.duration
                easing.type: window.easingType
            }

            QAccelPlot.DrawTransition {
                id: drawTransition
                duration: window.duration
                easing.type: window.easingType
            }
        }
    }
}
