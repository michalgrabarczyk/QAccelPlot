//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "theme/ColorPalette.hpp"

#include <QJSEngine>

namespace QAccelPlot {

// Palettes are shared across QML engines. They are heap-allocated and never deleted as a
// precaution against static destruction order: late readers and QML teardown hooks at exit
// never see a destroyed palette.
const ColorPalette& ColorPalette::dark()
{
    static const auto* const palette = createDark();
    return *palette;
}

const ColorPalette& ColorPalette::light()
{
    static const auto* const palette = createLight();
    return *palette;
}

ColorPalette* ColorPalette::createDark()
{
    auto* p = new ColorPalette;
    QJSEngine::setObjectOwnership(p, QJSEngine::CppOwnership);

    p->window = QColor("#0c1117");
    p->plotArea = QColor("#161d26");
    p->axesArea = QColor("#10161e");
    p->control = QColor("#1b2530");
    p->controlHover = QColor("#253344");
    p->outline = QColor("#344357");
    p->plotBorder = QColor("#2a3746");

    p->axisLine = QColor("#8d9bad");
    p->tick = QColor("#7f8ea1");
    p->subtick = QColor("#536174");
    p->grid = QColor("#2d3947");
    p->subGrid = QColor("#212b36");
    p->axisTickLabel = QColor("#f1f5f9");

    p->text = QColor("#f1f5f9");
    p->textSecondary = QColor("#b5c0ce");
    p->textMuted = QColor("#7f8b9b");
    p->textOnAccent = QColor("#07111d");
    p->materialAccent = QColor("#22c7d9");
    p->hover = QColor("#63aef7");
    p->focus = QColor("#8cc6ff");
    p->statusGood = QColor("#54d99a");
    p->statusWarning = QColor("#ffb454");
    p->statusError = QColor("#ff727d");
    p->axisLabel = p->textSecondary;

    p->legendBackground = QColor("#ed17202b");
    p->legendBorder = QColor("#526176");
    p->tooltipBackground = QColor("#ed080d13");
    p->tooltipText = QColor("#f8fafc");
    p->handleBorder = QColor("#0a0f15");
    p->transparent = QColor(Qt::transparent);

    p->seriesPrimary = QColor("#65b5ff");
    p->seriesSecondary = QColor("#ffb454");
    p->seriesTertiary = QColor("#5bd6a2");
    p->seriesQuaternary = QColor("#d6a0ff");
    p->seriesCyan = QColor("#59d5e8");
    p->seriesRose = QColor("#ff7f91");
    p->seriesYellow = QColor("#e8d35d");
    p->seriesMuted = QColor("#a9b8cc");

    p->annotationEvent = QColor("#ff727d");
    p->annotationPeak = QColor("#d9a3ff");
    p->annotationValley = QColor("#65b5ff");
    p->annotationRange = QColor("#5bd486");
    p->annotationRangeFill = QColor("#335bd486");
    p->annotationMarkerOutline = QColor("#f8fafc");

    p->toolRuler = QColor("#e0b85e");
    p->toolAngle = QColor("#61c3df");
    p->toolPoint = QColor("#f08398");
    p->toolRegion = QColor("#69c493");
    p->toolOverlay = QColor("#335bd6a2");

    p->performanceCurve = QColor("#54d99a");
    p->performanceRectangles = QColor("#cc65a9ff");

    return p;
}

ColorPalette* ColorPalette::createLight()
{
    auto* p = new ColorPalette;
    QJSEngine::setObjectOwnership(p, QJSEngine::CppOwnership);

    p->window = QColor("#f2f6fa");
    p->plotArea = QColor("#ffffff");
    p->axesArea = QColor("#f7f9fc");
    p->control = QColor("#ffffff");
    p->controlHover = QColor("#e7eef6");
    p->outline = QColor("#c6d1de");
    p->plotBorder = QColor("#b7c4d2");

    p->axisLine = QColor("#526174");
    p->tick = QColor("#64748b");
    p->subtick = QColor("#94a3b8");
    p->grid = QColor("#d2dbe5");
    p->subGrid = QColor("#e8edf2");
    p->axisTickLabel = QColor("#405064");

    p->text = QColor("#182233");
    p->textSecondary = QColor("#4d5b6d");
    p->textMuted = QColor("#778497");
    p->textOnAccent = QColor("#ffffff");
    p->materialAccent = QColor("#00798a");
    p->hover = QColor("#176bb7");
    p->focus = QColor("#005fae");
    p->statusGood = QColor("#007c59");
    p->statusWarning = QColor("#a95700");
    p->statusError = QColor("#c63242");
    p->axisLabel = p->textSecondary;

    p->legendBackground = QColor("#f2ffffff");
    p->legendBorder = QColor("#aebdcc");
    p->tooltipBackground = QColor("#ed182233");
    p->tooltipText = QColor("#ffffff");
    p->handleBorder = QColor("#ffffff");
    p->transparent = QColor(Qt::transparent);

    p->seriesPrimary = QColor("#006fbb");
    p->seriesSecondary = QColor("#a95700");
    p->seriesTertiary = QColor("#007c59");
    p->seriesQuaternary = QColor("#7747bd");
    p->seriesCyan = QColor("#007987");
    p->seriesRose = QColor("#bf3853");
    p->seriesYellow = QColor("#776500");
    p->seriesMuted = QColor("#526174");

    p->annotationEvent = QColor("#c63242");
    p->annotationPeak = QColor("#7d45b1");
    p->annotationValley = QColor("#006fbb");
    p->annotationRange = QColor("#087a45");
    p->annotationRangeFill = QColor("#2b087a45");
    p->annotationMarkerOutline = QColor("#ffffff");

    p->toolRuler = QColor("#8a6500");
    p->toolAngle = QColor("#007589");
    p->toolPoint = QColor("#b63858");
    p->toolRegion = QColor("#087a45");
    p->toolOverlay = QColor("#2b007c59");

    p->performanceCurve = QColor("#007c59");
    p->performanceRectangles = QColor("#a6006fbb");

    return p;
}

} // namespace QAccelPlot
