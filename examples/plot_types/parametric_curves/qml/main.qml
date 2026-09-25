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
    // Shorter than the screenshot delay, so visual tests capture fully drawn curves.
    readonly property int drawDuration: 1200

    property int spiralTurns: 5
    property int lissajousRatioIndex: 2
    property real lissajousPhaseDegrees: 45
    property int spirographIndex: 0
    property real vanDerPolMu: 1.0

    // qmlformat off
    readonly property var lissajousRatios: [
        { a: 1, b: 2 },
        { a: 3, b: 2 },
        { a: 3, b: 4 },
        { a: 5, b: 4 },
        { a: 5, b: 6 }
    ]
    readonly property var spirographPresets: [
        { outer: 11, inner: 4, pen: 8 },
        { outer: 9, inner: 2, pen: 6 },
        { outer: 12, inner: 7, pen: 10 }
    ]
    // qmlformat on
    readonly property var defaultStarts: [Qt.point(0.1, 0.0), Qt.point(-0.6, -0.8), Qt.point(3.5, 2.5), Qt.point(-3.5, -2.5), Qt.point(0.8, 4.5)]
    readonly property int maxStarts: 12
    property int nextColorIndex: 0
    readonly property var trajectoryColors: [colorPalette.seriesPrimary, colorPalette.seriesSecondary, colorPalette.seriesTertiary, colorPalette.seriesQuaternary, colorPalette.seriesCyan, colorPalette.seriesRose, colorPalette.seriesYellow]

    readonly property var limitCycle: vanDerPolLimitCycle(vanDerPolMu)
    readonly property real vanDerPolYSpan: Math.max(5.5, 1.3 * limitCycle.maxY)

    signal replayRequested

    function drawCurve(curve, points, animated) {
        curve.transition.enabled = animated;
        curve.setData(points);
    }

    // Archimedean spiral arm with one unit of radius per turn.
    function spiralArm(turns, rotation) {
        const points = [];
        const steps = turns * 240;
        for (let i = 0; i <= steps; ++i) {
            const angle = 2 * Math.PI * turns * i / steps;
            const radius = angle / (2 * Math.PI);
            points.push(Qt.point(radius * Math.cos(angle + rotation), radius * Math.sin(angle + rotation)));
        }
        return points;
    }

    function lissajous(ratio, phaseDegrees) {
        const points = [];
        const steps = 2000;
        const phase = phaseDegrees * Math.PI / 180;
        for (let i = 0; i <= steps; ++i) {
            const t = 2 * Math.PI * i / steps;
            points.push(Qt.point(Math.sin(ratio.a * t + phase), Math.sin(ratio.b * t)));
        }
        return points;
    }

    function greatestCommonDivisor(a, b) {
        return b === 0 ? a : greatestCommonDivisor(b, a % b);
    }

    // Hypotrochoid: a pen at distance `pen` from the center of a circle rolling inside a fixed circle.
    function spirograph(preset) {
        const points = [];
        const difference = preset.outer - preset.inner;
        const revolutions = preset.inner / greatestCommonDivisor(preset.outer, preset.inner);
        const steps = revolutions * 1200;
        for (let i = 0; i <= steps; ++i) {
            const t = 2 * Math.PI * revolutions * i / steps;
            const rollAngle = difference / preset.inner * t;
            points.push(Qt.point(difference * Math.cos(t) + preset.pen * Math.cos(rollAngle), difference * Math.sin(t) - preset.pen * Math.sin(rollAngle)));
        }
        return points;
    }

    function spirographSpan(preset) {
        return preset.outer - preset.inner + preset.pen;
    }

    // One RK4 step of x'' - mu (1 - x²) x' + x = 0, written as a first-order system.
    function vanDerPolStep(state, mu, dt) {
        const derivative = (x, v) => [v, mu * (1 - x * x) * v - x];
        const k1 = derivative(state[0], state[1]);
        const k2 = derivative(state[0] + dt / 2 * k1[0], state[1] + dt / 2 * k1[1]);
        const k3 = derivative(state[0] + dt / 2 * k2[0], state[1] + dt / 2 * k2[1]);
        const k4 = derivative(state[0] + dt * k3[0], state[1] + dt * k3[1]);
        return [state[0] + dt / 6 * (k1[0] + 2 * k2[0] + 2 * k3[0] + k4[0]), state[1] + dt / 6 * (k1[1] + 2 * k2[1] + 2 * k3[1] + k4[1])];
    }

    function vanDerPolTrajectory(startX, startY, mu) {
        const dt = 0.02;
        const steps = 2000;
        let state = [startX, startY];
        const points = [Qt.point(state[0], state[1])];
        for (let i = 0; i < steps; ++i) {
            state = vanDerPolStep(state, mu, dt);
            if (Math.abs(state[0]) > 100 || Math.abs(state[1]) > 100) {
                break;
            }
            points.push(Qt.point(state[0], state[1]));
        }
        return points;
    }

    // Settles onto the limit cycle, then records exactly one period between two upward crossings of v = 0.
    function vanDerPolLimitCycle(mu) {
        const dt = 0.01;
        let state = [2.0, 0.0];
        for (let i = 0; i < 6000; ++i) {
            state = vanDerPolStep(state, mu, dt);
        }
        let crossings = 0;
        let maxY = 0;
        const points = [];
        for (let i = 0; i < 6000 && crossings < 2; ++i) {
            const next = vanDerPolStep(state, mu, dt);
            if (state[1] < 0 && next[1] >= 0) {
                ++crossings;
            }
            if (crossings === 1 || (crossings === 2 && points.length > 0)) {
                points.push(Qt.point(next[0], next[1]));
                maxY = Math.max(maxY, Math.abs(next[1]));
            }
            state = next;
        }
        return {
            points: points,
            maxY: maxY
        };
    }

    function resetStarts() {
        startsModel.clear();
        nextColorIndex = 0;
        for (const start of defaultStarts) {
            appendStart(start.x, start.y);
        }
        updateStartMarkers();
    }

    function addStart(x, y) {
        if (startsModel.count >= maxStarts) {
            startsModel.remove(defaultStarts.length);
        }
        appendStart(x, y);
        updateStartMarkers();
    }

    // Stores the color with the start, so removing an older trajectory does not recolor the rest.
    function appendStart(x, y) {
        startsModel.append({
            startX: x,
            startY: y,
            colorIndex: nextColorIndex
        });
        nextColorIndex = (nextColorIndex + 1) % trajectoryColors.length;
    }

    function updateStartMarkers() {
        const points = [];
        for (let i = 0; i < startsModel.count; ++i) {
            const start = startsModel.get(i);
            points.push(Qt.point(start.startX, start.startY));
        }
        startMarkers.setData(points);
    }

    width: 1200
    height: 900
    visible: true
    title: "QAccelPlot Parametric Curves"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent
    Material.foreground: colorPalette.text

    ListModel {
        id: startsModel
    }

    component StyledPlot: QAccelPlot.Plot {
        Layout.fillWidth: true
        Layout.fillHeight: true
        legendVisible: false
        border.color: window.colorPalette.plotBorder
        border.width: 2
    }

    // Keeps one data unit the same length on both axes, so circles stay round.
    component EqualAspectPlot: StyledPlot {
        id: equalAspectPlot

        // Half the visible Y range in data units.
        property real span: 1
        property string xLabel
        property string yLabel
        readonly property real aspect: plotRect.height > 0 ? plotRect.width / plotRect.height : 1

        xAxis: ExampleAxis {
            viewportMin: -equalAspectPlot.span * equalAspectPlot.aspect
            viewportMax: equalAspectPlot.span * equalAspectPlot.aspect
            dataMin: -equalAspectPlot.span * equalAspectPlot.aspect
            dataMax: equalAspectPlot.span * equalAspectPlot.aspect
            label: equalAspectPlot.xLabel
        }

        yAxis: ExampleAxis {
            viewportMin: -equalAspectPlot.span
            viewportMax: equalAspectPlot.span
            dataMin: -equalAspectPlot.span
            dataMax: equalAspectPlot.span
            axisTitlePadding: 34
            layoutSize: 54
            label: equalAspectPlot.yLabel
        }
    }

    component Panel: ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.preferredWidth: 1
        Layout.preferredHeight: 1
        spacing: 2
    }

    component PanelHeader: RowLayout {
        property alias title: titleLabel.text

        Layout.fillWidth: true
        Layout.preferredHeight: 40
        spacing: 8

        Label {
            id: titleLabel
            color: window.colorPalette.text
            font.bold: true
            font.pixelSize: 13
        }
        Item {
            Layout.fillWidth: true
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ExampleHeader {
            title: "Parametric curves"
            description: "LineCurve connects samples in the order they are given, so x does not have to increase: curves can loop, spiral, and cross themselves. Hover a curve to highlight it. Double-click the phase portrait to start a new trajectory there."

            Button {
                text: "Replay drawing"
                Material.background: colorPalette.plotArea
                onClicked: window.replayRequested()
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 2
            rowSpacing: 8
            columnSpacing: 12

            // ── Archimedean spiral: many y values for every x ─────────────────
            Panel {
                PanelHeader {
                    title: "Archimedean spiral"

                    Label {
                        text: "Turns " + window.spiralTurns
                    }
                    Slider {
                        from: 2
                        to: 10
                        stepSize: 1
                        value: window.spiralTurns
                        Layout.preferredWidth: 150
                        onMoved: window.spiralTurns = value
                    }
                }

                EqualAspectPlot {
                    id: spiralPlot
                    span: window.spiralTurns + 0.6
                    xLabel: "x"
                    yLabel: "y"

                    Repeater {
                        // qmlformat off
                        model: [
                            { rotation: 0, color: window.colorPalette.seriesPrimary },
                            { rotation: Math.PI, color: window.colorPalette.seriesSecondary }
                        ]
                        // qmlformat on

                        delegate: QAccelPlot.LineCurve {
                            id: armCurve

                            required property var modelData

                            function refresh(animated) {
                                window.drawCurve(armCurve, window.spiralArm(window.spiralTurns, modelData.rotation), animated);
                            }

                            xAxis: spiralPlot.xAxis
                            yAxis: spiralPlot.yAxis
                            color: modelData.color
                            lineWidth: hovered ? 4 : 2
                            transition: QAccelPlot.DrawTransition {
                                duration: window.drawDuration
                            }
                            Component.onCompleted: refresh(true)

                            Connections {
                                target: window
                                function onSpiralTurnsChanged() {
                                    armCurve.refresh(false);
                                }
                                function onReplayRequested() {
                                    armCurve.refresh(true);
                                }
                            }
                        }
                    }
                }
            }

            // ── Lissajous figure: an oscilloscope in XY mode ──────────────────
            Panel {
                PanelHeader {
                    title: "Lissajous figure"

                    ComboBox {
                        model: window.lissajousRatios.map(ratio => ratio.a + " : " + ratio.b)
                        currentIndex: window.lissajousRatioIndex
                        Layout.preferredWidth: 100
                        Layout.preferredHeight: 40
                        Material.background: colorPalette.plotArea
                        onActivated: window.lissajousRatioIndex = currentIndex
                    }
                    Label {
                        text: "Phase " + window.lissajousPhaseDegrees.toFixed(0) + "°"
                    }
                    Slider {
                        from: 0
                        to: 180
                        stepSize: 1
                        value: window.lissajousPhaseDegrees
                        Layout.preferredWidth: 150
                        onMoved: window.lissajousPhaseDegrees = value
                    }
                }

                StyledPlot {
                    id: lissajousPlot

                    xAxis: ExampleAxis {
                        viewportMin: -1.2
                        viewportMax: 1.2
                        dataMin: -1.2
                        dataMax: 1.2
                        label: "Channel X (V)"
                    }

                    yAxis: ExampleAxis {
                        viewportMin: -1.2
                        viewportMax: 1.2
                        dataMin: -1.2
                        dataMax: 1.2
                        axisTitlePadding: 34
                        layoutSize: 54
                        label: "Channel Y (V)"
                    }

                    QAccelPlot.LineCurve {
                        id: lissajousCurve

                        function refresh(animated) {
                            window.drawCurve(lissajousCurve, window.lissajous(window.lissajousRatios[window.lissajousRatioIndex], window.lissajousPhaseDegrees), animated);
                        }

                        xAxis: lissajousPlot.xAxis
                        yAxis: lissajousPlot.yAxis
                        color: window.colorPalette.text
                        lineWidth: hovered ? 4 : 2
                        effects: [
                            QAccelPlot.GradientStroke {
                                direction: QAccelPlot.GradientDirection.Vertical
                                gradientValueMinSource: QAccelPlot.GradientValueSource.Fixed
                                gradientValueMin: -1
                                gradientValueMaxSource: QAccelPlot.GradientValueSource.Fixed
                                gradientValueMax: 1
                                colormap: QAccelPlot.Colormap {
                                    preset: QAccelPlot.Colormap.Plasma
                                }
                            }
                        ]
                        transition: QAccelPlot.DrawTransition {
                            duration: window.drawDuration
                        }
                        Component.onCompleted: refresh(true)

                        Connections {
                            target: window
                            function onLissajousRatioIndexChanged() {
                                lissajousCurve.refresh(true);
                            }
                            function onLissajousPhaseDegreesChanged() {
                                lissajousCurve.refresh(false);
                            }
                            function onReplayRequested() {
                                lissajousCurve.refresh(true);
                            }
                        }
                    }
                }
            }

            // ── Spirograph: a curve that crosses itself many times ────────────
            Panel {
                PanelHeader {
                    title: "Spirograph (hypotrochoid)"

                    ComboBox {
                        model: window.spirographPresets.map(preset => "R " + preset.outer + " · r " + preset.inner + " · d " + preset.pen)
                        currentIndex: window.spirographIndex
                        Layout.preferredWidth: 170
                        Layout.preferredHeight: 40
                        Material.background: colorPalette.plotArea
                        onActivated: window.spirographIndex = currentIndex
                    }
                }

                EqualAspectPlot {
                    id: spirographPlot

                    readonly property real curveSpan: window.spirographSpan(window.spirographPresets[window.spirographIndex])

                    span: curveSpan * 1.08
                    xLabel: "x"
                    yLabel: "y"

                    QAccelPlot.LineCurve {
                        id: spirographCurve

                        function refresh(animated) {
                            window.drawCurve(spirographCurve, window.spirograph(window.spirographPresets[window.spirographIndex]), animated);
                        }

                        xAxis: spirographPlot.xAxis
                        yAxis: spirographPlot.yAxis
                        color: window.colorPalette.text
                        lineWidth: hovered ? 3 : 1.5
                        effects: [
                            QAccelPlot.GradientStroke {
                                direction: QAccelPlot.GradientDirection.Vertical
                                gradientValueMinSource: QAccelPlot.GradientValueSource.Fixed
                                gradientValueMin: -spirographPlot.curveSpan
                                gradientValueMaxSource: QAccelPlot.GradientValueSource.Fixed
                                gradientValueMax: spirographPlot.curveSpan
                                colormap: QAccelPlot.Colormap {
                                    preset: QAccelPlot.Colormap.Viridis
                                }
                            }
                        ]
                        transition: QAccelPlot.DrawTransition {
                            duration: window.drawDuration
                        }
                        Component.onCompleted: refresh(true)

                        Connections {
                            target: window
                            function onSpirographIndexChanged() {
                                spirographCurve.refresh(true);
                            }
                            function onReplayRequested() {
                                spirographCurve.refresh(true);
                            }
                        }
                    }
                }
            }

            // ── Van der Pol phase portrait: trajectories spiral onto a limit cycle ──
            Panel {
                PanelHeader {
                    title: "Van der Pol phase portrait"

                    Label {
                        text: "μ " + window.vanDerPolMu.toFixed(1)
                    }
                    Slider {
                        from: 0.1
                        to: 4.0
                        stepSize: 0.1
                        value: window.vanDerPolMu
                        Layout.preferredWidth: 150
                        onMoved: window.vanDerPolMu = value
                    }
                    Button {
                        text: "Reset"
                        Layout.preferredHeight: 40
                        Material.background: colorPalette.plotArea
                        onClicked: window.resetStarts()
                    }
                }

                StyledPlot {
                    id: vanDerPolPlot

                    xAxis: ExampleAxis {
                        viewportMin: -4
                        viewportMax: 4
                        dataMin: -4
                        dataMax: 4
                        label: "Position x"
                    }

                    yAxis: ExampleAxis {
                        viewportMin: -window.vanDerPolYSpan
                        viewportMax: window.vanDerPolYSpan
                        dataMin: -window.vanDerPolYSpan
                        dataMax: window.vanDerPolYSpan
                        axisTitlePadding: 34
                        layoutSize: 54
                        label: "Velocity dx/dt"
                    }

                    onMouseDoubleClicked: event => {
                        if (event.button !== Qt.LeftButton || !isInsidePlotArea(event.x, event.y)) {
                            return;
                        }
                        event.accept();
                        window.addStart(pixelToDataX(event.x), pixelToDataY(event.y));
                    }

                    Repeater {
                        model: startsModel

                        delegate: QAccelPlot.LineCurve {
                            id: trajectory

                            required property real startX
                            required property real startY
                            required property int colorIndex

                            function refresh(animated) {
                                window.drawCurve(trajectory, window.vanDerPolTrajectory(startX, startY, window.vanDerPolMu), animated);
                            }

                            xAxis: vanDerPolPlot.xAxis
                            yAxis: vanDerPolPlot.yAxis
                            color: window.trajectoryColors[colorIndex]
                            lineWidth: hovered ? 3.5 : 1.5
                            transition: QAccelPlot.DrawTransition {
                                duration: window.drawDuration
                            }
                            Component.onCompleted: refresh(true)

                            Connections {
                                target: window
                                function onVanDerPolMuChanged() {
                                    trajectory.refresh(false);
                                }
                                function onReplayRequested() {
                                    trajectory.refresh(true);
                                }
                            }
                        }
                    }

                    QAccelPlot.LineCurve {
                        id: limitCycleCurve

                        function refresh(animated) {
                            window.drawCurve(limitCycleCurve, window.limitCycle.points, animated);
                        }

                        xAxis: vanDerPolPlot.xAxis
                        yAxis: vanDerPolPlot.yAxis
                        color: window.colorPalette.text
                        lineWidth: 1.5
                        lineStyle: QAccelPlot.DashLine {
                            pattern: [8, 5]
                        }
                        transition: QAccelPlot.DrawTransition {
                            duration: window.drawDuration
                        }
                        Component.onCompleted: refresh(true)

                        Connections {
                            target: window
                            function onLimitCycleChanged() {
                                limitCycleCurve.refresh(false);
                            }
                            function onReplayRequested() {
                                limitCycleCurve.refresh(true);
                            }
                        }
                    }

                    QAccelPlot.PointCloud {
                        id: startMarkers
                        xAxis: vanDerPolPlot.xAxis
                        yAxis: vanDerPolPlot.yAxis
                        color: window.colorPalette.text
                        marker.shape: QAccelPlot.PointCloud.Circle
                        marker.size: 4
                    }

                    Component.onCompleted: window.resetStarts()
                }
            }
        }
    }
}
