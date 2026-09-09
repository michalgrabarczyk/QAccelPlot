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

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 900
    minimumWidth: 900
    minimumHeight: 600
    title: "QAccelPlot Benchmark"
    color: colorPalette.window
    Material.theme: Material.Dark
    Material.accent: colorPalette.materialAccent

    // ── Design tokens ────────────────────────────────────────────────────────
    readonly property QtObject colorPalette: QAccelPlot.Colors.dark
    readonly property color bgBase: colorPalette.window
    readonly property color bgSurface: colorPalette.axesArea
    readonly property color bgCard: colorPalette.control
    readonly property color bgHover: colorPalette.controlHover
    readonly property color borderColor: colorPalette.outline
    readonly property color accent: colorPalette.materialAccent
    readonly property color accentDim: Qt.darker(colorPalette.materialAccent, 1.25)
    readonly property color textPrimary: colorPalette.text
    readonly property color textSecond: colorPalette.textSecondary
    readonly property color textMuted: colorPalette.textMuted
    readonly property color colorGood: colorPalette.statusGood
    readonly property color colorWarn: colorPalette.statusWarning
    readonly property color colorBad: colorPalette.statusError

    readonly property int fontSizeSm: 11
    readonly property int fontSizeMd: 13
    readonly property int fontSizeLg: 15
    readonly property int fontSizeHero: 52
    readonly property int radiusSm: 4
    readonly property int radiusMd: 4
    readonly property int radiusLg: 4

    // ── FPS history for the sparkline ─────────────────────────────────────
    property var fpsHistory: []
    readonly property int fpsHistoryMax: 120

    Connections {
        target: benchmarkRunner
        function onLiveFpsChanged() {
            const h = root.fpsHistory.slice();
            h.push(benchmarkRunner.liveFps);
            if (h.length > root.fpsHistoryMax)
                h.shift();
            root.fpsHistory = h;
            sparklineCanvas.requestPaint();
        }
        function onAllCompleted() {
            root.fpsHistory = [];
            sparklineCanvas.requestPaint();
        }
    }

    // ── Helpers ───────────────────────────────────────────────────────────
    function fpsColor(fps) {
        if (fps <= 0)
            return root.textMuted;
        if (fps >= 55)
            return root.colorGood;
        if (fps >= 30)
            return root.colorWarn;
        return root.colorBad;
    }

    function formatNum(n, decimals) {
        if (n === undefined || n === null)
            return "—";
        return Number(n).toFixed(decimals !== undefined ? decimals : 1);
    }

    function doExport() {
        const path = "qaccelplot_benchmark_results.json";
        if (benchmarkRunner.exportJson(path)) {
            exportToast.show("Saved to " + path);
        } else {
            exportToast.show("Export failed!");
        }
    }

    // ── Root layout ───────────────────────────────────────────────────────
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ════════════════════════════════════════════════════════════════════
        //  SIDEBAR
        // ════════════════════════════════════════════════════════════════════
        Rectangle {
            id: sidebar
            Layout.preferredWidth: 240
            Layout.fillHeight: true
            color: root.bgSurface
            layer.enabled: true
            layer.effect: null

            // right border line
            Rectangle {
                anchors {
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                }
                width: 1
                color: root.borderColor
            }

            ColumnLayout {
                anchors {
                    fill: parent
                    margins: 12
                }
                spacing: 0

                // Logo / title
                ColumnLayout {
                    spacing: 2
                    Layout.bottomMargin: 20

                    Text {
                        text: "QAccelPlot"
                        font {
                            pixelSize: root.fontSizeLg
                            weight: Font.Bold
                            letterSpacing: 1.5
                        }
                        color: root.accent
                    }
                    Text {
                        text: "Benchmark Suite"
                        font.pixelSize: root.fontSizeSm
                        color: root.textMuted
                    }
                }

                // Action buttons
                ColumnLayout {
                    spacing: 8
                    Layout.fillWidth: true
                    Layout.bottomMargin: 20

                    BenchButton {
                        id: runAllBtn
                        Layout.fillWidth: true
                        label: benchmarkRunner.running ? "Running…" : "Run All Scenarios"
                        primary: true
                        interactive: !benchmarkRunner.running
                        onClicked: benchmarkRunner.runAll()
                    }

                    BenchButton {
                        id: stopBtn
                        Layout.fillWidth: true
                        label: "Stop"
                        primary: false
                        interactive: benchmarkRunner.running
                        onClicked: benchmarkRunner.stop()
                    }
                }

                // Divider
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: root.borderColor
                    Layout.bottomMargin: 16
                }

                Text {
                    text: "SCENARIOS"
                    font {
                        pixelSize: 10
                        weight: Font.Bold
                        letterSpacing: 2
                    }
                    color: root.textMuted
                    Layout.bottomMargin: 8
                }

                // Scenario list
                ListView {
                    id: scenarioList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 2

                    model: benchmarkRunner.availableScenarios

                    delegate: Rectangle {
                        id: scenarioDelegate

                        required property var modelData

                        width: scenarioList.width
                        height: Math.max(48, contentLayout.implicitHeight + 16)
                        radius: root.radiusSm
                        color: {
                            const isActive = benchmarkRunner.currentScenarioName === scenarioDelegate.modelData.name;
                            if (isActive)
                                return Qt.alpha(root.accent, 0.15);
                            return scenarioHover.containsMouse ? root.bgHover : "transparent";
                        }

                        RowLayout {
                            id: contentLayout
                            anchors {
                                fill: parent
                                leftMargin: 10
                                rightMargin: 10
                                topMargin: 8
                                bottomMargin: 8
                            }
                            spacing: 8

                            // Active indicator dot
                            Rectangle {
                                width: 6
                                height: 6
                                radius: 3
                                color: root.accent
                                opacity: benchmarkRunner.currentScenarioName === scenarioDelegate.modelData.name ? 1.0 : 0.0
                                Behavior on opacity {
                                    NumberAnimation {
                                        duration: 200
                                    }
                                }
                                Layout.alignment: Qt.AlignTop
                                Layout.topMargin: 4
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                Text {
                                    text: scenarioDelegate.modelData.label
                                    font.pixelSize: root.fontSizeSm
                                    font.weight: Font.DemiBold
                                    color: root.textPrimary
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }

                                Text {
                                    text: scenarioDelegate.modelData.desc
                                    font.pixelSize: 10
                                    color: root.textSecond
                                    Layout.fillWidth: true
                                    wrapMode: Text.WordWrap
                                }
                            }

                            // Completed checkmark
                            Text {
                                text: "✓"
                                font.pixelSize: root.fontSizeSm
                                color: root.colorGood
                                Layout.alignment: Qt.AlignVCenter
                                opacity: {
                                    const results = benchmarkRunner.results;
                                    for (var i = 0; i < results.length; ++i) {
                                        if (results[i].name === scenarioDelegate.modelData.name)
                                            return 1.0;
                                    }
                                    return 0.0;
                                }
                            }
                        }

                        MouseArea {
                            id: scenarioHover
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (!benchmarkRunner.running) {
                                    benchmarkRunner.runScenario(scenarioDelegate.modelData.name);
                                }
                            }
                        }

                        Behavior on color {
                            ColorAnimation {
                                duration: 120
                            }
                        }
                    }
                }
            }
        }

        // ════════════════════════════════════════════════════════════════════
        //  MAIN CONTENT
        // ════════════════════════════════════════════════════════════════════
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // ── Top bar ──────────────────────────────────────────────────
            Rectangle {
                Layout.fillWidth: true
                height: 52
                color: root.bgSurface

                Rectangle {
                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }
                    height: 1
                    color: root.borderColor
                }

                RowLayout {
                    anchors {
                        fill: parent
                        leftMargin: 12
                        rightMargin: 12
                    }

                    Text {
                        text: benchmarkRunner.running ? "Running — " + benchmarkRunner.currentScenarioLabel : benchmarkRunner.results.length > 0 ? "Results" : "Ready"
                        font {
                            pixelSize: root.fontSizeMd
                            weight: Font.Medium
                        }
                        color: root.textPrimary
                        Layout.fillWidth: true
                    }

                    // Progress bar
                    Rectangle {
                        id: progressBarContainer

                        visible: benchmarkRunner.running
                        width: 160
                        height: 4
                        radius: 2
                        color: root.bgCard
                        Layout.rightMargin: 16

                        Rectangle {
                            width: progressBarContainer.width * benchmarkRunner.progress
                            height: progressBarContainer.height
                            radius: progressBarContainer.radius
                            color: root.accent
                            Behavior on width {
                                NumberAnimation {
                                    duration: 300
                                    easing.type: Easing.OutCubic
                                }
                            }
                        }
                    }
                }
            }

            // ── Live dashboard ────────────────────────────────────────────
            Rectangle {
                Layout.fillWidth: true
                height: 200
                color: root.bgBase

                RowLayout {
                    anchors {
                        fill: parent
                        margins: 12
                    }
                    spacing: 12

                    // Hero FPS counter
                    Rectangle {
                        Layout.preferredWidth: 220
                        Layout.fillHeight: true
                        radius: root.radiusLg
                        color: root.bgCard
                        border.color: root.borderColor
                        border.width: 1

                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 4

                            Text {
                                text: "AVG FPS"
                                font {
                                    pixelSize: 10
                                    weight: Font.Bold
                                    letterSpacing: 2
                                }
                                color: root.textMuted
                                Layout.alignment: Qt.AlignHCenter
                            }

                            Text {
                                id: heroFps
                                text: benchmarkRunner.running ? root.formatNum(benchmarkRunner.liveFps, 0) : "—"
                                font {
                                    pixelSize: root.fontSizeHero
                                    weight: Font.Bold
                                    letterSpacing: -2
                                }
                                color: root.fpsColor(benchmarkRunner.liveFps)
                                Layout.alignment: Qt.AlignHCenter

                                Behavior on color {
                                    ColorAnimation {
                                        duration: 300
                                    }
                                }
                            }

                            Text {
                                text: "frames / sec"
                                font.pixelSize: root.fontSizeSm
                                color: root.textMuted
                                Layout.alignment: Qt.AlignHCenter
                            }
                        }
                    }

                    // Secondary metrics column
                    ColumnLayout {
                        Layout.preferredWidth: 160
                        Layout.fillHeight: true
                        spacing: 8

                        MetricTile {
                            label: "FRAME TIME"
                            value: benchmarkRunner.running ? root.formatNum(benchmarkRunner.liveFrameTimeMs, 2) + " ms" : "—"
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }

                        MetricTile {
                            label: "SCENARIO"
                            value: benchmarkRunner.currentScenarioLabel || "—"
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }
                    }

                    // Sparkline / FPS history chart
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: root.radiusLg
                        color: root.bgCard
                        border.color: root.borderColor
                        border.width: 1
                        clip: true

                        Text {
                            anchors {
                                top: parent.top
                                left: parent.left
                                margins: 12
                            }
                            text: "FPS HISTORY"
                            font {
                                pixelSize: 10
                                weight: Font.Bold
                                letterSpacing: 2
                            }
                            color: root.textMuted
                        }

                        Canvas {
                            id: sparklineCanvas
                            anchors {
                                fill: parent
                                margins: 1
                            }

                            onPaint: {
                                const ctx = getContext("2d");
                                ctx.clearRect(0, 0, width, height);
                                const h = root.fpsHistory;
                                if (h.length < 2)
                                    return;
                                const maxFps = Math.max(120, ...h);
                                const padT = 30, padB = 8, padL = 8, padR = 8;
                                const chartW = width - padL - padR;
                                const chartH = height - padT - padB;

                                // Grid line at 60 fps
                                const y60 = padT + chartH - (60 / maxFps) * chartH;
                                ctx.strokeStyle = root.borderColor.toString();
                                ctx.lineWidth = 1;
                                ctx.setLineDash([4, 4]);
                                ctx.beginPath();
                                ctx.moveTo(padL, y60);
                                ctx.lineTo(padL + chartW, y60);
                                ctx.stroke();
                                ctx.setLineDash([]);

                                // Gradient fill
                                const grad = ctx.createLinearGradient(0, padT, 0, padT + chartH);
                                grad.addColorStop(0, Qt.rgba(0.23, 0.51, 0.96, 0.35));
                                grad.addColorStop(1, Qt.rgba(0.23, 0.51, 0.96, 0.0));
                                ctx.fillStyle = grad;
                                ctx.beginPath();
                                for (var i = 0; i < h.length; ++i) {
                                    const x = padL + (i / (root.fpsHistoryMax - 1)) * chartW;
                                    const y = padT + chartH - Math.min(h[i] / maxFps, 1.0) * chartH;
                                    if (i === 0)
                                        ctx.moveTo(x, y);
                                    else
                                        ctx.lineTo(x, y);
                                }
                                ctx.lineTo(padL + ((h.length - 1) / (root.fpsHistoryMax - 1)) * chartW, padT + chartH);
                                ctx.lineTo(padL, padT + chartH);
                                ctx.closePath();
                                ctx.fill();

                                // Line
                                ctx.strokeStyle = root.accent.toString();
                                ctx.lineWidth = 2;
                                ctx.lineJoin = "round";
                                ctx.beginPath();
                                for (var j = 0; j < h.length; ++j) {
                                    const x2 = padL + (j / (root.fpsHistoryMax - 1)) * chartW;
                                    const y2 = padT + chartH - Math.min(h[j] / maxFps, 1.0) * chartH;
                                    if (j === 0)
                                        ctx.moveTo(x2, y2);
                                    else
                                        ctx.lineTo(x2, y2);
                                }
                                ctx.stroke();
                            }
                        }
                    }
                }
            }

            // Divider
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: root.borderColor
            }

            // ── Results table ─────────────────────────────────────────────
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: root.bgBase

                ColumnLayout {
                    anchors {
                        fill: parent
                        margins: 12
                    }
                    spacing: 12

                    Text {
                        text: "RESULTS"
                        font {
                            pixelSize: 10
                            weight: Font.Bold
                            letterSpacing: 2
                        }
                        color: root.textMuted
                    }

                    // Header row
                    Rectangle {
                        Layout.fillWidth: true
                        height: 32
                        radius: root.radiusSm
                        color: root.bgCard
                        border.color: root.borderColor
                        border.width: 1

                        RowLayout {
                            anchors {
                                fill: parent
                                leftMargin: 12
                                rightMargin: 12
                            }
                            spacing: 0

                            Repeater {
                                model: [
                                    {
                                        label: "Scenario",
                                        flex: 3,
                                        align: Text.AlignLeft
                                    },
                                    {
                                        label: "Avg FPS",
                                        flex: 1,
                                        align: Text.AlignHCenter
                                    },
                                    {
                                        label: "Min FPS",
                                        flex: 1,
                                        align: Text.AlignHCenter
                                    },
                                    {
                                        label: "P50 (ms)",
                                        flex: 1,
                                        align: Text.AlignHCenter
                                    },
                                    {
                                        label: "P95 (ms)",
                                        flex: 1,
                                        align: Text.AlignHCenter
                                    },
                                    {
                                        label: "P99 (ms)",
                                        flex: 1,
                                        align: Text.AlignHCenter
                                    },
                                    {
                                        label: "Frames",
                                        flex: 1,
                                        align: Text.AlignHCenter
                                    },
                                    {
                                        label: "Mpts/s",
                                        flex: 1,
                                        align: Text.AlignHCenter
                                    },
                                ]

                                delegate: Text {
                                    required property var modelData

                                    text: modelData.label
                                    font {
                                        pixelSize: 10
                                        weight: Font.Bold
                                        letterSpacing: 1.5
                                    }
                                    color: root.textMuted
                                    horizontalAlignment: modelData.align
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: modelData.flex * 60
                                }
                            }
                        }
                    }

                    // Result rows
                    ListView {
                        id: resultsList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 4
                        model: benchmarkRunner.results

                        delegate: Rectangle {
                            id: resultDelegate

                            required property var modelData

                            width: resultsList.width
                            height: 38
                            radius: root.radiusSm
                            color: resultHover.containsMouse ? root.bgHover : root.bgCard
                            border.color: root.borderColor
                            border.width: 1

                            // Coloured left accent bar keyed to FPS quality
                            Rectangle {
                                anchors {
                                    left: parent.left
                                    top: parent.top
                                    bottom: parent.bottom
                                    topMargin: 6
                                    bottomMargin: 6
                                }
                                width: 3
                                radius: 2
                                color: root.fpsColor(resultDelegate.modelData.fpsAvg)
                            }

                            RowLayout {
                                anchors {
                                    fill: parent
                                    leftMargin: 16
                                    rightMargin: 12
                                }
                                spacing: 0

                                Text {
                                    text: resultDelegate.modelData.label
                                    font {
                                        pixelSize: root.fontSizeSm
                                        weight: Font.Medium
                                    }
                                    color: root.textPrimary
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 3 * 60
                                    elide: Text.ElideRight
                                }

                                Text {
                                    text: root.formatNum(resultDelegate.modelData.fpsAvg, 1)
                                    font {
                                        pixelSize: root.fontSizeSm
                                        weight: Font.Bold
                                    }
                                    color: root.fpsColor(resultDelegate.modelData.fpsAvg)
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 60
                                }

                                Text {
                                    text: root.formatNum(resultDelegate.modelData.fpsMin, 1)
                                    font.pixelSize: root.fontSizeSm
                                    color: root.textSecond
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 60
                                }

                                Text {
                                    text: root.formatNum(resultDelegate.modelData.frameTimeP50, 2)
                                    font.pixelSize: root.fontSizeSm
                                    color: root.textSecond
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 60
                                }

                                Text {
                                    text: root.formatNum(resultDelegate.modelData.frameTimeP95, 2)
                                    font.pixelSize: root.fontSizeSm
                                    color: root.textSecond
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 60
                                }

                                Text {
                                    text: root.formatNum(resultDelegate.modelData.frameTimeP99, 2)
                                    font {
                                        pixelSize: root.fontSizeSm
                                        weight: Font.Medium
                                    }
                                    color: resultDelegate.modelData.frameTimeP99 > 50 ? root.colorWarn : root.textSecond
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 60
                                }

                                Text {
                                    text: resultDelegate.modelData.totalFrames !== undefined ? resultDelegate.modelData.totalFrames : "—"
                                    font.pixelSize: root.fontSizeSm
                                    color: root.textMuted
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 60
                                }

                                Text {
                                    text: root.formatNum(resultDelegate.modelData.throughputMpps, 1)
                                    font.pixelSize: root.fontSizeSm
                                    color: root.textSecond
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 60
                                }
                            }

                            MouseArea {
                                id: resultHover
                                anchors.fill: parent
                                hoverEnabled: true
                            }

                            Behavior on color {
                                ColorAnimation {
                                    duration: 100
                                }
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            visible: resultsList.count === 0
                            text: "Run a scenario to see results here"
                            font.pixelSize: root.fontSizeMd
                            color: root.textMuted
                        }
                    }
                }
            }

            // ── System info bar ───────────────────────────────────────────
            Rectangle {
                Layout.fillWidth: true
                height: 36
                color: root.bgSurface

                Rectangle {
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: parent.top
                    }
                    height: 1
                    color: root.borderColor
                }

                RowLayout {
                    anchors {
                        fill: parent
                        leftMargin: 12
                        rightMargin: 12
                    }

                    Text {
                        text: benchmarkRunner.systemInfo
                        font.pixelSize: 11
                        color: root.textMuted
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    Text {
                        text: "QAccelPlot v0.1"
                        font {
                            pixelSize: 11
                            weight: Font.Medium
                        }
                        color: root.textSecond
                    }
                }
            }
        }
    }

    // ── Toast notification ────────────────────────────────────────────────
    Rectangle {
        id: exportToast
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            bottomMargin: 24
        }
        width: toastText.implicitWidth + 32
        height: 40
        radius: root.radiusMd
        color: root.bgCard
        border.color: root.borderColor
        border.width: 1
        opacity: 0
        z: 100

        property alias text: toastText.text

        function show(msg) {
            exportToast.text = msg;
            showAnim.restart();
        }

        Text {
            id: toastText
            anchors.centerIn: parent
            font.pixelSize: root.fontSizeSm
            color: root.textPrimary
        }

        SequentialAnimation {
            id: showAnim
            NumberAnimation {
                target: exportToast
                property: "opacity"
                to: 1
                duration: 200
            }
            PauseAnimation {
                duration: 2400
            }
            NumberAnimation {
                target: exportToast
                property: "opacity"
                to: 0
                duration: 300
            }
        }
    }

    // ── Inline components ─────────────────────────────────────────────────

    component BenchButton: Rectangle {
        id: btn

        // Use `interactive` instead of `enabled` to avoid shadowing Item.enabled,
        // which would suppress mouse events and break hover detection.
        property string label: ""
        property bool primary: false
        property bool interactive: true
        signal clicked

        height: 34
        radius: root.radiusSm
        color: {
            if (!btn.interactive)
                return root.bgCard;
            if (btn.primary)
                return btnMouse.pressed ? root.accentDim : btnMouse.containsMouse ? Qt.lighter(root.accent, 1.1) : root.accent;
            return btnMouse.containsMouse ? root.bgHover : root.bgCard;
        }
        border.color: btn.primary ? "transparent" : root.borderColor
        border.width: 1
        opacity: btn.interactive ? 1.0 : 0.4

        Text {
            anchors.centerIn: parent
            text: btn.label
            font {
                pixelSize: root.fontSizeSm
                weight: Font.Medium
            }
            color: btn.primary ? root.colorPalette.textOnAccent : root.textSecond
        }

        MouseArea {
            id: btnMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: btn.interactive ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: if (btn.interactive)
                btn.clicked()
        }

        Behavior on color {
            ColorAnimation {
                duration: 100
            }
        }
    }

    component MetricTile: Rectangle {
        id: tile
        property string label: ""
        property string value: "—"

        height: 60
        radius: root.radiusMd
        color: root.bgCard
        border.color: root.borderColor
        border.width: 1

        ColumnLayout {
            anchors {
                fill: parent
                margins: 10
            }
            spacing: 4

            Text {
                text: tile.label
                font {
                    pixelSize: 10
                    weight: Font.Bold
                    letterSpacing: 1.5
                }
                color: root.textMuted
            }
            Text {
                text: tile.value
                font {
                    pixelSize: root.fontSizeLg
                    weight: Font.Medium
                }
                color: root.textPrimary
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }
    }
}
