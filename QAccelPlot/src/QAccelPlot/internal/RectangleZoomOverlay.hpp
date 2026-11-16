//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/internal/OutlinedRectangle.hpp"

namespace QAccelPlot {
class PlotRectangleZoom;
} // namespace QAccelPlot

namespace QAccelPlot::Internal {

class RectangleZoomOverlay final : public OutlinedRectangle {
public:
    RectangleZoomOverlay(QQuickItem* parent, PlotRectangleZoom* configuration);

private:
    void syncSelection();
    void syncStyle();

    PlotRectangleZoom* configuration_;
};

} // namespace QAccelPlot::Internal
