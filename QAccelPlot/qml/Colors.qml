pragma Singleton

//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
import QtQuick

QtObject {
    readonly property QtObject dark: QtObject {
        readonly property color window: "#0c1117"
        readonly property color plotArea: "#161d26"
        readonly property color axesArea: "#10161e"
        readonly property color control: "#1b2530"
        readonly property color controlHover: "#253344"
        readonly property color outline: "#344357"
        readonly property color plotBorder: "#2a3746"

        readonly property color axisLine: "#8d9bad"
        readonly property color tick: "#7f8ea1"
        readonly property color subtick: "#536174"
        readonly property color grid: "#2d3947"
        readonly property color subGrid: "#212b36"
        readonly property color axisLabel: textSecondary
        readonly property color axisTickLabel: "#f1f5f9"

        readonly property color text: "#f1f5f9"
        readonly property color textSecondary: "#b5c0ce"
        readonly property color textMuted: "#7f8b9b"
        readonly property color textOnAccent: "#07111d"
        readonly property color materialAccent: "#22c7d9"
        readonly property color hover: "#63aef7"
        readonly property color focus: "#8cc6ff"
        readonly property color statusGood: "#54d99a"
        readonly property color statusWarning: "#ffb454"
        readonly property color statusError: "#ff727d"

        readonly property color legendBackground: "#ed17202b"
        readonly property color legendBorder: "#526176"
        readonly property color tooltipBackground: "#ed080d13"
        readonly property color tooltipText: "#f8fafc"
        readonly property color handleBorder: "#0a0f15"
        readonly property color transparent: "#00000000"

        readonly property color seriesPrimary: "#65b5ff"
        readonly property color seriesSecondary: "#ffb454"
        readonly property color seriesTertiary: "#5bd6a2"
        readonly property color seriesQuaternary: "#d6a0ff"
        readonly property color seriesCyan: "#59d5e8"
        readonly property color seriesRose: "#ff7f91"
        readonly property color seriesYellow: "#e8d35d"
        readonly property color seriesMuted: "#a9b8cc"

        readonly property color annotationEvent: "#ff727d"
        readonly property color annotationPeak: "#d9a3ff"
        readonly property color annotationValley: "#65b5ff"
        readonly property color annotationRange: "#5bd486"
        readonly property color annotationRangeFill: "#335bd486"
        readonly property color annotationMarkerOutline: "#f8fafc"

        readonly property color toolRuler: "#e0b85e"
        readonly property color toolAngle: "#61c3df"
        readonly property color toolPoint: "#f08398"
        readonly property color toolRegion: "#69c493"
        readonly property color toolOverlay: "#335bd6a2"

        readonly property color performanceCurve: "#54d99a"
        readonly property color performanceRectangles: "#cc65a9ff"
    }

    readonly property QtObject light: QtObject {
        readonly property color window: "#f2f6fa"
        readonly property color plotArea: "#ffffff"
        readonly property color axesArea: "#f7f9fc"
        readonly property color control: "#ffffff"
        readonly property color controlHover: "#e7eef6"
        readonly property color outline: "#c6d1de"
        readonly property color plotBorder: "#b7c4d2"

        readonly property color axisLine: "#526174"
        readonly property color tick: "#64748b"
        readonly property color subtick: "#94a3b8"
        readonly property color grid: "#d2dbe5"
        readonly property color subGrid: "#e8edf2"
        readonly property color axisLabel: textSecondary
        readonly property color axisTickLabel: "#405064"

        readonly property color text: "#182233"
        readonly property color textSecondary: "#4d5b6d"
        readonly property color textMuted: "#778497"
        readonly property color textOnAccent: "#ffffff"
        readonly property color materialAccent: "#00798a"
        readonly property color hover: "#176bb7"
        readonly property color focus: "#005fae"
        readonly property color statusGood: "#007c59"
        readonly property color statusWarning: "#a95700"
        readonly property color statusError: "#c63242"

        readonly property color legendBackground: "#f2ffffff"
        readonly property color legendBorder: "#aebdcc"
        readonly property color tooltipBackground: "#ed182233"
        readonly property color tooltipText: "#ffffff"
        readonly property color handleBorder: "#ffffff"
        readonly property color transparent: "#00000000"

        readonly property color seriesPrimary: "#006fbb"
        readonly property color seriesSecondary: "#a95700"
        readonly property color seriesTertiary: "#007c59"
        readonly property color seriesQuaternary: "#7747bd"
        readonly property color seriesCyan: "#007987"
        readonly property color seriesRose: "#bf3853"
        readonly property color seriesYellow: "#776500"
        readonly property color seriesMuted: "#526174"

        readonly property color annotationEvent: "#c63242"
        readonly property color annotationPeak: "#7d45b1"
        readonly property color annotationValley: "#006fbb"
        readonly property color annotationRange: "#087a45"
        readonly property color annotationRangeFill: "#2b087a45"
        readonly property color annotationMarkerOutline: "#ffffff"

        readonly property color toolRuler: "#8a6500"
        readonly property color toolAngle: "#007589"
        readonly property color toolPoint: "#b63858"
        readonly property color toolRegion: "#087a45"
        readonly property color toolOverlay: "#2b007c59"

        readonly property color performanceCurve: "#007c59"
        readonly property color performanceRectangles: "#a6006fbb"
    }
}
