//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/export/internal/GrabSize.hpp"

#include <QtMath>

#include <algorithm>
#include <cstdlib>

namespace QAccelPlot::Internal {

namespace {

#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
// Matches the rounding of QSize * qreal, which QQuickItemGrabResult applies to the target size.
int renderedLength(const int target, const qreal devicePixelRatio)
{
    return qRound(target * devicePixelRatio);
}

int targetLength(const int pixels, const qreal devicePixelRatio)
{
    const auto below = std::max(1, qFloor(pixels / devicePixelRatio));
    const auto above = below + 1;
    const auto belowError = std::abs(renderedLength(below, devicePixelRatio) - pixels);
    const auto aboveError = std::abs(renderedLength(above, devicePixelRatio) - pixels);
    return aboveError < belowError ? above : below;
}
#endif

} // namespace

GrabSize grabSizeFor(const QSize& pixels, [[maybe_unused]] const qreal devicePixelRatio)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
    return {pixels, pixels};
#else
    const auto target = QSize(targetLength(pixels.width(), devicePixelRatio), targetLength(pixels.height(), devicePixelRatio));
    return {target, QSize(renderedLength(target.width(), devicePixelRatio), renderedLength(target.height(), devicePixelRatio))};
#endif
}

} // namespace QAccelPlot::Internal
