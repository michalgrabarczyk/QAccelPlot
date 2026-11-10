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
    readonly property int minBars: 4
    readonly property int maxBars: 24
    property int duration: 900
    property int easingType: Easing.InOutCubic
    property real stagger: 0.5
    property int fadeDuration: 300
    property var values: []

    // Picks the transitions of the three slots; each slot holds one of the transitions below.
    readonly property var enterTransitions: [growIn, drawIn, null]
    readonly property var updateTransitions: [morphTransition, redrawTransition, null]
    readonly property var exitTransitions: [growOut, drawOut, null]

    function randomValue() {
        return 1.5 + Math.random() * 7.5;
    }

    function showValues(next) {
        values = next;
        bars.setData(next.map((value, index) => ({
                        position: index,
                        value: value
                    })));
    }

    function shuffle() {
        showValues(values.map(() => randomValue()));
    }

    function addBars() {
        const next = values.slice();
        for (let i = 0; i < 4 && next.length < maxBars; ++i) {
            next.push(randomValue());
        }
        showValues(next);
    }

    function removeBars() {
        showValues(values.slice(0, Math.max(minBars, values.length - 4)));
    }

    width: 1000
    height: 640
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
            subtitle: "Enter, update, and exit transitions animate the bars a data change adds, keeps, and removes."
            interactions: [
                "Click <b>Shuffle</b>, <b>Add</b>, or <b>Remove</b> to change the data",
                "Choose how bars enter, update, and exit",
                "Drag <b>Stagger</b> and <b>Fade</b> to spread the animation of each bar and fade it",
                "Drag <b>Duration</b> and choose an easing curve"
            ]
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "Shuffle"
                Material.background: colorPalette.plotArea
                onClicked: window.shuffle()
            }

            Button {
                text: "Add"
                enabled: window.values.length < window.maxBars
                Material.background: colorPalette.plotArea
                onClicked: window.addBars()
            }

            Button {
                text: "Remove"
                enabled: window.values.length > window.minBars
                Material.background: colorPalette.plotArea
                onClicked: window.removeBars()
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                text: "Enter and exit"
            }

            ComboBox {
                id: enterBox
                model: ["Grow", "Draw", "None"]
                Layout.preferredWidth: 110
                Material.background: colorPalette.plotArea
            }

            Label {
                text: "Update"
            }

            ComboBox {
                id: updateBox
                model: ["Morph", "Redraw", "None"]
                Layout.preferredWidth: 120
                Material.background: colorPalette.plotArea
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "Stagger " + window.stagger.toFixed(2)
                Layout.preferredWidth: 90
            }

            Slider {
                from: 0
                to: 1
                stepSize: 0.05
                value: window.stagger
                Layout.preferredWidth: 160
                onMoved: window.stagger = value
            }

            Label {
                text: window.fadeDuration > 0 ? "Fade " + window.fadeDuration + " ms" : "No fade"
                Layout.preferredWidth: 90
            }

            Slider {
                from: 0
                to: 1000
                stepSize: 50
                value: window.fadeDuration
                Layout.preferredWidth: 160
                onMoved: window.fadeDuration = value
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
                Layout.preferredWidth: 160
                onMoved: window.duration = value
            }

            ComboBox {
                model: ["Linear", "In/out cubic", "Out back"]
                currentIndex: 1
                Layout.preferredWidth: 140
                Material.background: colorPalette.plotArea
                onActivated: window.easingType = [Easing.Linear, Easing.InOutCubic, Easing.OutBack][currentIndex]
            }
        }

        QAccelPlot.Plot {
            id: plot
            Layout.fillWidth: true
            Layout.fillHeight: true
            legendVisible: false
            grid.subGridVisible: false

            xAxis: ExampleAxis {
                viewportMin: -0.75
                viewportMax: window.maxBars - 0.25
                label.text: "Bar"
            }

            yAxis: ExampleAxis {
                viewportMin: 0
                viewportMax: 10
                label.text: "Value"
                label.padding: 40
                layoutSize: 60
            }

            QAccelPlot.BarSeries {
                id: bars
                xAxis: plot.xAxis
                yAxis: plot.yAxis
                color: colorPalette.seriesPrimary
                barWidth: 0.7
                enterTransition: window.enterTransitions[enterBox.currentIndex]
                updateTransition: window.updateTransitions[updateBox.currentIndex]
                exitTransition: window.exitTransitions[enterBox.currentIndex]
                Component.onCompleted: window.showValues(Array.from({
                        length: 12
                    }, () => window.randomValue()))
            }
        }
    }

    // Opacity animations run on the timeline of each bar, which starts when the bar starts to move.
    QAccelPlot.GrowTransition {
        id: growIn
        duration: window.duration
        easing.type: window.easingType
        stagger: window.stagger

        NumberAnimation {
            property: "opacity"
            from: 0
            duration: window.fadeDuration
        }
    }

    QAccelPlot.DrawTransition {
        id: drawIn
        duration: window.duration
        easing.type: window.easingType
        stagger: window.stagger

        NumberAnimation {
            property: "opacity"
            from: 0
            duration: window.fadeDuration
        }
    }

    // Exits play the movement backwards, but their opacity animations as written.
    QAccelPlot.GrowTransition {
        id: growOut
        duration: window.duration
        easing.type: window.easingType
        stagger: window.stagger

        NumberAnimation {
            property: "opacity"
            to: window.fadeDuration > 0 ? 0 : 1
            duration: window.fadeDuration
        }
    }

    QAccelPlot.DrawTransition {
        id: drawOut
        duration: window.duration
        easing.type: window.easingType
        stagger: window.stagger

        NumberAnimation {
            property: "opacity"
            to: window.fadeDuration > 0 ? 0 : 1
            duration: window.fadeDuration
        }
    }

    // Kept bars dim while they move, then recover.
    QAccelPlot.MorphTransition {
        id: morphTransition
        duration: window.duration
        easing.type: window.easingType
        stagger: window.stagger

        SequentialAnimation {
            NumberAnimation {
                property: "opacity"
                to: 0.35
                duration: window.fadeDuration / 2
            }
            NumberAnimation {
                property: "opacity"
                to: 1
                duration: window.fadeDuration / 2
            }
        }
    }

    // As an update transition, a GrowTransition regrows every bar from the baseline.
    QAccelPlot.GrowTransition {
        id: redrawTransition
        duration: window.duration
        easing.type: window.easingType
        stagger: window.stagger

        NumberAnimation {
            property: "opacity"
            from: 0
            duration: window.fadeDuration
        }
    }
}
