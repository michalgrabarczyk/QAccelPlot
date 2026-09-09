//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "transitions/DrawTransition.hpp"

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

DrawTransition::DrawTransition(QObject* parent)
    : DataTransition(parent)
{
}

void DrawTransition::interpolate(const float easedProgress, const std::vector<float>& /*fromData*/, const int /*fromPointCount*/,
    const std::vector<float>& toData, const int toPointCount, std::vector<float>& outData, int& outPointCount)
{
    if (toPointCount <= 0) {
        outData.clear();
        outPointCount = 0;
        return;
    }

    const auto visibleCount = std::min(toPointCount, std::max(2, static_cast<int>(std::ceil(toPointCount * easedProgress))));
    outPointCount = visibleCount;
    outData.resize(visibleCount * 2);

    std::copy_n(toData.begin(), visibleCount * 2, outData.begin());
}

} // namespace QAccelPlot
