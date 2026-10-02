//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/PlotDragRect.hpp"

#include <algorithm>

namespace QAccelPlot {

void PlotDragRect::begin(const QPointF& position)
{
    active_ = true;
    start_ = position;
    end_ = position;
}

void PlotDragRect::moveTo(const QPointF& position, const QRectF& bounds)
{
    end_ = {std::clamp(position.x(), bounds.left(), bounds.right()), std::clamp(position.y(), bounds.top(), bounds.bottom())};
}

void PlotDragRect::end()
{
    active_ = false;
}

bool PlotDragRect::active() const
{
    return active_;
}

QRectF PlotDragRect::rect() const
{
    return QRectF{start_, end_}.normalized();
}

bool PlotDragRect::modifiersMatch(const int pressed, const int required)
{
    return pressed == required;
}

bool PlotDragRect::meetsMinimum(const QRectF& rect, const qreal minimumSize, const bool checkWidth, const bool checkHeight)
{
    return (!checkWidth || rect.width() >= minimumSize) && (!checkHeight || rect.height() >= minimumSize);
}

} // namespace QAccelPlot
