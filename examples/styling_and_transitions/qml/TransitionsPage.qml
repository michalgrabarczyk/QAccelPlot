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
    required property var palette
    required property var typography
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

    function applyMorph(target) {
        transitionCurve.transition = morphTransition;
        transitionCurve.setData(target);
    }

    function applyDraw(target) {
        transitionCurve.transition = drawTransition;
        transitionCurve.setData(target);
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Label {
            text: "Click Morph or Draw to animate the curve between two waveform shapes. Adjust the duration and easing to compare the transitions."
            color: root.palette.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "Morph"
                Material.background: root.palette.plotArea
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onClicked: {
                    root.applyMorph(root.nextShapeIsB ? root.shapeB() : root.shapeA());
                    root.nextShapeIsB = !root.nextShapeIsB;
                }
            }

            Button {
                text: "Draw"
                Material.background: root.palette.plotArea
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onClicked: {
                    root.applyDraw(root.nextShapeIsB ? root.shapeB() : root.shapeA());
                    root.nextShapeIsB = !root.nextShapeIsB;
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                text: "Duration " + root.duration + " ms"
                color: root.palette.text
            }

            Slider {
                from: 200
                to: 2400
                stepSize: 100
                value: root.duration
                Layout.preferredWidth: 180
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onMoved: root.duration = value
            }

            ComboBox {
                model: ["Linear", "In/out cubic", "Out elastic"]
                currentIndex: 1
                Layout.preferredWidth: 140
                Material.background: root.palette.plotArea
                Material.foreground: root.palette.text
                Material.accent: root.palette.materialAccent
                onActivated: {
                    if (currentIndex === 0)
                        root.easingType = Easing.Linear;
                    else if (currentIndex === 1)
                        root.easingType = Easing.InOutCubic;
                    else
                        root.easingType = Easing.OutElastic;
                }
            }
        }

        QAccelPlot.Plot {
            id: transitionPlot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            plotAreaColor: root.palette.plotArea
            axesAreaColor: root.palette.axesArea
            grid.gridColor: root.palette.grid
            grid.subGridVisible: false

            xAxis: QAccelPlot.Axis {
                viewportMin: 0
                viewportMax: 8
                dataMin: 0
                dataMax: 8
                label: "x"
                baselineColor: root.palette.axisLine
                labelColor: root.palette.axisLabel
                labelFont: root.typography.axisLabel
                hoverColor: root.palette.hover
                ticker.tickColor: root.palette.tick
                ticker.tickLabelColor: root.palette.axisTickLabel
                ticker.tickLabelFont: root.typography.axisTickLabel
                ticker.subtickColor: root.palette.subtick
            }

            yAxis: QAccelPlot.Axis {
                viewportMin: -1.2
                viewportMax: 1.2
                dataMin: -1.2
                dataMax: 1.2
                axisTitlePadding: 40
                layoutSize: 60
                label: "Value"
                baselineColor: root.palette.axisLine
                labelColor: root.palette.axisLabel
                labelFont: root.typography.axisLabel
                hoverColor: root.palette.hover
                ticker.tickColor: root.palette.tick
                ticker.tickLabelColor: root.palette.axisTickLabel
                ticker.tickLabelFont: root.typography.axisTickLabel
                ticker.subtickColor: root.palette.subtick
            }

            QAccelPlot.LineCurve {
                id: transitionCurve
                xAxis: transitionPlot.xAxis
                yAxis: transitionPlot.yAxis
                color: root.palette.seriesPrimary
                lineWidth: 3
                antialiasingEnabled: true
                transition: morphTransition
                Component.onCompleted: setData(root.shapeA())
            }

            QAccelPlot.MorphTransition {
                id: morphTransition
                duration: root.duration
                easing.type: root.easingType
            }

            QAccelPlot.DrawTransition {
                id: drawTransition
                duration: root.duration
                easing.type: root.easingType
            }
        }
    }
}
