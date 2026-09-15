//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "series/LineCurveGaps.hpp"

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
