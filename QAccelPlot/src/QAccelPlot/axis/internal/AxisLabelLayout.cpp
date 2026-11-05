//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/internal/AxisLabelLayout.hpp"

#include <QtMath>

#include <cmath>

namespace QAccelPlot::Internal {

QSizeF rotatedBoundingSize(const QSizeF& size, const qreal degrees)
{
    const auto radians = qDegreesToRadians(degrees);
    const auto cosine = std::abs(std::cos(radians));
    const auto sine = std::abs(std::sin(radians));
    return QSizeF{size.width() * cosine + size.height() * sine, size.width() * sine + size.height() * cosine};
}

QPointF axisLabelCenter(const AxisLabelLayoutParams& params)
{
    const auto box = rotatedBoundingSize(params.textSize, params.rotation);
    const auto& r = params.dataRect;
    auto center = QPointF{};
    switch (params.outward) {
    case Qt::BottomEdge:
        center.setY(params.baseline + params.padding + box.height() / 2.0);
        break;
    case Qt::TopEdge:
        center.setY(params.baseline - params.padding - box.height() / 2.0);
        break;
    case Qt::LeftEdge:
        center.setX(params.baseline - params.padding - box.width() / 2.0);
        break;
    case Qt::RightEdge:
        center.setX(params.baseline + params.padding + box.width() / 2.0);
        break;
    }
    if (params.outward == Qt::TopEdge || params.outward == Qt::BottomEdge) {
        center.setX(r.left() + box.width() / 2.0 + params.alongFraction * (r.width() - box.width()));
    } else {
        center.setY(r.bottom() - box.height() / 2.0 - params.alongFraction * (r.height() - box.height()));
    }
    return center + params.offset;
}

} // namespace QAccelPlot::Internal
