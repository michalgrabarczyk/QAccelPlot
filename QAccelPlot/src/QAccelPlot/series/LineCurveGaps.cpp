//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/series/LineCurveGaps.hpp"

namespace QAccelPlot {

LineCurveGaps::LineCurveGaps(QObject* parent)
    : QObject(parent)
{
}

NanGapMode LineCurveGaps::nanMode() const
{
    return nanMode_;
}

void LineCurveGaps::setNanMode(const NanGapMode mode)
{
    if (nanMode_ == mode) {
        return;
    }
    nanMode_ = mode;
    emit nanModeChanged();
}

} // namespace QAccelPlot
