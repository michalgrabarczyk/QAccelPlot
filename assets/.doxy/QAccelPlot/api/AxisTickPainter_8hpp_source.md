

# File AxisTickPainter.hpp

[**File List**](files.md) **>** [**axis**](dir_f07047c7c39e03c70b8bf3fe272880c9.md) **>** [**AxisTickPainter.hpp**](AxisTickPainter_8hpp.md)

[Go to the documentation of this file](AxisTickPainter_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "axis/Axis.hpp"
#include "axis/AxisTicker.hpp"
#include "axis/AxisTicks.hpp"

#include <QColor>
#include <QRectF>

#include <functional>

QT_FORWARD_DECLARE_CLASS(QPainter)

namespace QAccelPlot {

class AxisTickPainter {
public:
    struct Params {
        Axis::Orientation orientation{Axis::Horizontal}; 
        Axis::Side side{Axis::Left};                     
        bool hovered{false};                             
        const AxisTicker* ticker{nullptr};               
        QColor hoverColor;                               
        QColor defaultSubtickColor;                      
        bool clampEdgeLabels{false};                     
        qreal labelOverflow{0.0};                        
    };

    using MapToPosition = std::function<qreal(qreal value, qreal length)>;

    struct PaintContext {
        QPainter* painter; 
        QRectF rect;       
        qreal axisX;       
        qreal axisY;       
    };

    static AxisTicks computeTicks(qreal viewportMin, qreal viewportMax, bool logScale, const AxisTicker* ticker);

    static void paintTicks(
        QPainter* painter, const QRectF& rect, qreal axisX, qreal axisY, const Params& params, const AxisTicks& ticks, const MapToPosition& mapToPosition);

    static qreal computeNiceStep(qreal viewportMin, qreal viewportMax, int tickCount);

private:
    static AxisTicks computeLogScaleTicks(qreal viewportMin, qreal viewportMax, const AxisTicker& ticker);
    static AxisTicks computeLinearTicks(qreal viewportMin, qreal viewportMax, const AxisTicker& ticker);
    static void paintTick(const PaintContext& ctx, const AxisTick& tick, const Params& params, const MapToPosition& mapToPosition);
    static void paintSubtick(const PaintContext& ctx, qreal value, const Params& params, const MapToPosition& mapToPosition);
    static void drawTickLabel(QPainter* painter, const QRectF& labelRect, int alignment, const QString& label, qreal rotation);
};

} // namespace QAccelPlot
```


