

# File ColorPalette.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**theme**](dir_5e767e95370086c11dfb89b66970881a.md) **>** [**ColorPalette.hpp**](ColorPalette_8hpp.md)

[Go to the documentation of this file](ColorPalette_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>
#include <QObject>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

namespace QAccelPlot {

class ColorPalette : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(ColorPalette)
    QML_UNCREATABLE("ColorPalette instances are provided by the Colors singleton.")

    // Surfaces
    Q_PROPERTY(QColor window MEMBER window CONSTANT)
    Q_PROPERTY(QColor plotArea MEMBER plotArea CONSTANT)
    Q_PROPERTY(QColor axesArea MEMBER axesArea CONSTANT)
    Q_PROPERTY(QColor control MEMBER control CONSTANT)
    Q_PROPERTY(QColor controlHover MEMBER controlHover CONSTANT)
    Q_PROPERTY(QColor outline MEMBER outline CONSTANT)
    Q_PROPERTY(QColor plotBorder MEMBER plotBorder CONSTANT)

    // Axes and grid
    Q_PROPERTY(QColor axisLine MEMBER axisLine CONSTANT)
    Q_PROPERTY(QColor tick MEMBER tick CONSTANT)
    Q_PROPERTY(QColor subtick MEMBER subtick CONSTANT)
    Q_PROPERTY(QColor grid MEMBER grid CONSTANT)
    Q_PROPERTY(QColor subGrid MEMBER subGrid CONSTANT)
    Q_PROPERTY(QColor axisLabel MEMBER axisLabel CONSTANT)
    Q_PROPERTY(QColor axisTickLabel MEMBER axisTickLabel CONSTANT)

    // Text and interaction states
    Q_PROPERTY(QColor text MEMBER text CONSTANT)
    Q_PROPERTY(QColor textSecondary MEMBER textSecondary CONSTANT)
    Q_PROPERTY(QColor textMuted MEMBER textMuted CONSTANT)
    Q_PROPERTY(QColor textOnAccent MEMBER textOnAccent CONSTANT)
    Q_PROPERTY(QColor materialAccent MEMBER materialAccent CONSTANT)
    Q_PROPERTY(QColor hover MEMBER hover CONSTANT)
    Q_PROPERTY(QColor focus MEMBER focus CONSTANT)
    Q_PROPERTY(QColor statusGood MEMBER statusGood CONSTANT)
    Q_PROPERTY(QColor statusWarning MEMBER statusWarning CONSTANT)
    Q_PROPERTY(QColor statusError MEMBER statusError CONSTANT)

    // Overlays
    Q_PROPERTY(QColor legendBackground MEMBER legendBackground CONSTANT)
    Q_PROPERTY(QColor legendBorder MEMBER legendBorder CONSTANT)
    Q_PROPERTY(QColor tooltipBackground MEMBER tooltipBackground CONSTANT)
    Q_PROPERTY(QColor tooltipText MEMBER tooltipText CONSTANT)
    Q_PROPERTY(QColor handleBorder MEMBER handleBorder CONSTANT)
    Q_PROPERTY(QColor transparent MEMBER transparent CONSTANT)

    // Series
    Q_PROPERTY(QColor seriesPrimary MEMBER seriesPrimary CONSTANT)
    Q_PROPERTY(QColor seriesSecondary MEMBER seriesSecondary CONSTANT)
    Q_PROPERTY(QColor seriesTertiary MEMBER seriesTertiary CONSTANT)
    Q_PROPERTY(QColor seriesQuaternary MEMBER seriesQuaternary CONSTANT)
    Q_PROPERTY(QColor seriesCyan MEMBER seriesCyan CONSTANT)
    Q_PROPERTY(QColor seriesRose MEMBER seriesRose CONSTANT)
    Q_PROPERTY(QColor seriesYellow MEMBER seriesYellow CONSTANT)
    Q_PROPERTY(QColor seriesMuted MEMBER seriesMuted CONSTANT)

    // Annotations
    Q_PROPERTY(QColor annotationEvent MEMBER annotationEvent CONSTANT)
    Q_PROPERTY(QColor annotationPeak MEMBER annotationPeak CONSTANT)
    Q_PROPERTY(QColor annotationValley MEMBER annotationValley CONSTANT)
    Q_PROPERTY(QColor annotationRange MEMBER annotationRange CONSTANT)
    Q_PROPERTY(QColor annotationRangeFill MEMBER annotationRangeFill CONSTANT)
    Q_PROPERTY(QColor annotationMarkerOutline MEMBER annotationMarkerOutline CONSTANT)

    // Interactive tools
    Q_PROPERTY(QColor toolRuler MEMBER toolRuler CONSTANT)
    Q_PROPERTY(QColor toolAngle MEMBER toolAngle CONSTANT)
    Q_PROPERTY(QColor toolPoint MEMBER toolPoint CONSTANT)
    Q_PROPERTY(QColor toolRegion MEMBER toolRegion CONSTANT)
    Q_PROPERTY(QColor toolOverlay MEMBER toolOverlay CONSTANT)

    // Performance showcase
    Q_PROPERTY(QColor performanceCurve MEMBER performanceCurve CONSTANT)
    Q_PROPERTY(QColor performanceRectangles MEMBER performanceRectangles CONSTANT)

public:
    static const ColorPalette& dark();
    static const ColorPalette& light();

    QColor window;
    QColor plotArea;
    QColor axesArea;
    QColor control;
    QColor controlHover;
    QColor outline;
    QColor plotBorder;

    QColor axisLine;
    QColor tick;
    QColor subtick;
    QColor grid;
    QColor subGrid;
    QColor axisLabel;
    QColor axisTickLabel;

    QColor text;
    QColor textSecondary;
    QColor textMuted;
    QColor textOnAccent;
    QColor materialAccent;
    QColor hover;
    QColor focus;
    QColor statusGood;
    QColor statusWarning;
    QColor statusError;

    QColor legendBackground;
    QColor legendBorder;
    QColor tooltipBackground;
    QColor tooltipText;
    QColor handleBorder;
    QColor transparent;

    QColor seriesPrimary;
    QColor seriesSecondary;
    QColor seriesTertiary;
    QColor seriesQuaternary;
    QColor seriesCyan;
    QColor seriesRose;
    QColor seriesYellow;
    QColor seriesMuted;

    QColor annotationEvent;
    QColor annotationPeak;
    QColor annotationValley;
    QColor annotationRange;
    QColor annotationRangeFill;
    QColor annotationMarkerOutline;

    QColor toolRuler;
    QColor toolAngle;
    QColor toolPoint;
    QColor toolRegion;
    QColor toolOverlay;

    QColor performanceCurve;
    QColor performanceRectangles;

private:
    ColorPalette() = default;

    static ColorPalette* createDark();
    static ColorPalette* createLight();
};

} // namespace QAccelPlot
```


