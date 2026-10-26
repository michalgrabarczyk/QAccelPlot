//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick

// The scripted eight-second loop of recordings and --demo. A pointer inspects the bearing
// fault's peak in the spectrum, then the waveforms zoom into one Pump DE impact down to single
// samples and back out. Everything is a function of the acquisition time, so recorded frames do
// not depend on rendering speed, and the last frame leads seamlessly into the first.
//
// The inspectors are driven without a mouse: followPointer is off, and cursorPixelX and
// cursorPixelY follow the scripted pointer. A pixel cursor stays put while the waveform view
// zooms under it, and the inspector reports the data beneath it.
Item {
    id: timeline

    required property QtObject monitor
    required property Item waveformPanel
    required property Item spectrumPanel
    // Acquisition time of the data on screen, in seconds.
    required property real time
    property bool running: true

    readonly property real loopSeconds: 8
    readonly property real t: time - Math.floor(time / loopSeconds) * loopSeconds
    // A bearing impact two seconds into the loop, slightly after it starts ringing. While zoomed,
    // the view follows it as the trace scrolls, so its samples stand still on screen.
    readonly property real impactTime: Math.round(2 * monitor.bpfoHz) / monitor.bpfoHz + 0.0006
    readonly property real impactX: impactTime - t
    readonly property real impactY: monitor.channelOffset(monitor.faultChannel)
    readonly property real zoom: ease(progress(4.6, 5.8)) - ease(progress(6.5, 7.4))
    readonly property var xRange: zoomedRange(-monitor.windowSeconds, 0, impactX, 0.0018)
    readonly property var yRange: zoomedRange(waveformPanel.defaultYMin, waveformPanel.defaultYMax, impactY, 1)

    // Pointer position in scene coordinates. The waypoints are computed here, on every frame,
    // because item positions in the scene have no change signals to bind to.
    readonly property point pointer: {
        const rest = sceneOfData(spectrumPanel.plot, 760, 0.002);
        const peak = sceneOfData(spectrumPanel.plot, monitor.bpfoHz, 0.12);
        const impact = sceneOfPlotFraction(waveformPanel.plot, xRange.position, yRange.position);
        if (t < 2.8)
            return mixPoints(rest, peak, ease(progress(2.0, 2.8)));
        if (t < 4.6)
            return mixPoints(peak, impact, ease(progress(4.0, 4.6)));
        return impact;
    }
    readonly property real pointerOpacity: progress(1.4, 1.8) - progress(7.5, 7.9)
    readonly property point spectrumPointer: spectrumPanel.plot.mapFromItem(null, pointer.x, pointer.y)
    readonly property point waveformPointer: waveformPanel.plot.mapFromItem(null, pointer.x, pointer.y)
    readonly property bool onSpectrum: pointerOpacity > 0.5 && spectrumPanel.plot.isInsidePlotArea(spectrumPointer.x, spectrumPointer.y)
    readonly property bool onWaveforms: pointerOpacity > 0.5 && waveformPanel.plot.isInsidePlotArea(waveformPointer.x, waveformPointer.y)
    function progress(from, to) {
        return Math.max(0, Math.min(1, (t - from) / (to - from)));
    }

    function ease(x) {
        return x < 0.5 ? 4 * x * x * x : 1 - Math.pow(2 - 2 * x, 3) / 2;
    }

    function mixPoints(a, b, s) {
        return Qt.point(a.x + (b.x - a.x) * s, a.y + (b.y - a.y) * s);
    }

    // Narrows [fullMin, fullMax] toward zoomedSpan around target as zoom goes from 0 to 1. The span
    // shrinks geometrically, so the zoom speed looks steady, and the target glides to the middle.
    function zoomedRange(fullMin, fullMax, target, zoomedSpan) {
        const fullSpan = fullMax - fullMin;
        const span = fullSpan * Math.pow(zoomedSpan / fullSpan, zoom);
        const position = (target - fullMin) / fullSpan + (0.5 - (target - fullMin) / fullSpan) * zoom;
        const min = target - position * span;
        return { "min": min, "max": min + span, "position": position };
    }

    function sceneOfData(plot, x, y) {
        return plot.mapToItem(null, plot.dataToPixelX(x), plot.dataToPixelY(y));
    }

    // The zoom math already knows where the target is, so the pointer never lags the view by a frame.
    function sceneOfPlotFraction(plot, fractionX, fractionY) {
        const area = plot.plotRect;
        return plot.mapToItem(null, area.x + fractionX * area.width, area.y + (1 - fractionY) * area.height);
    }

    Binding {
        target: timeline.waveformPanel.plot.xAxis
        property: "viewportMin"
        value: timeline.xRange.min
        when: timeline.running
    }

    Binding {
        target: timeline.waveformPanel.plot.xAxis
        property: "viewportMax"
        value: timeline.xRange.max
        when: timeline.running
    }

    Binding {
        target: timeline.waveformPanel.plot.yAxis
        property: "viewportMin"
        value: timeline.yRange.min
        when: timeline.running
    }

    Binding {
        target: timeline.waveformPanel.plot.yAxis
        property: "viewportMax"
        value: timeline.yRange.max
        when: timeline.running
    }

    Binding {
        target: timeline.spectrumPanel.inspector
        property: "followPointer"
        value: false
        when: timeline.running
    }

    Binding {
        target: timeline.spectrumPanel.inspector
        property: "cursorPixelX"
        value: timeline.onSpectrum ? timeline.spectrumPointer.x : NaN
        when: timeline.running
    }

    Binding {
        target: timeline.spectrumPanel.inspector
        property: "cursorPixelY"
        value: timeline.onSpectrum ? timeline.spectrumPointer.y : NaN
        when: timeline.running
    }

    Binding {
        target: timeline.spectrumPanel
        property: "peakLabelHighlighted"
        value: timeline.onSpectrum && timeline.t > 2.6 && timeline.t < 4.0
        when: timeline.running
    }

    Binding {
        target: timeline.waveformPanel.inspector
        property: "followPointer"
        value: false
        when: timeline.running
    }

    Binding {
        target: timeline.waveformPanel.inspector
        property: "cursorPixelX"
        value: timeline.onWaveforms ? timeline.waveformPointer.x : NaN
        when: timeline.running
    }

    Binding {
        target: timeline.waveformPanel.inspector
        property: "cursorPixelY"
        value: timeline.onWaveforms ? timeline.waveformPointer.y : NaN
        when: timeline.running
    }

    // An arrow pointer with its tip at the scripted position.
    Canvas {
        x: timeline.pointer.x - 1
        y: timeline.pointer.y - 1
        width: 14
        height: 22
        // Kept visible while hidden by opacity, so it is painted before its first appearance.
        visible: timeline.running
        opacity: timeline.pointerOpacity

        onPaint: {
            const context = getContext("2d");
            context.reset();
            context.beginPath();
            context.moveTo(1, 1);
            context.lineTo(1, 17);
            context.lineTo(5, 13);
            context.lineTo(8, 20);
            context.lineTo(10.5, 19);
            context.lineTo(7.5, 12);
            context.lineTo(12.5, 12);
            context.closePath();
            context.fillStyle = "white";
            context.fill();
            context.lineWidth = 1.2;
            context.strokeStyle = "black";
            context.stroke();
        }
    }
}
