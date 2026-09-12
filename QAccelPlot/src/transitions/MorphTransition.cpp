//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "transitions/MorphTransition.hpp"

#include <algorithm>

namespace QAccelPlot {

MorphTransition::MorphTransition(QObject* parent)
    : DataTransition(parent)
{
}

void MorphTransition::interpolate(const double easedProgress, const std::vector<double>& fromData, const int fromPointCount, const std::vector<double>& toData,
    const int toPointCount, std::vector<double>& outData, int& outPointCount)
{
    if (toPointCount <= 0) {
        outPointCount = 0;
        outData.clear();
        return;
    }

    if (fromPointCount <= 0) {
        outPointCount = toPointCount;
        outData.assign(toData.begin(), toData.end());
        return;
    }

    const auto maxCount = std::max(fromPointCount, toPointCount);
    outPointCount = maxCount;
    outData.resize(maxCount * 2);

    for (int i = 0; i < maxCount; ++i) {
        const auto fromIdx = std::min(i, fromPointCount - 1);
        const auto toIdx = std::min(i, toPointCount - 1);

        const auto fx = fromData[fromIdx * 2];
        const auto fy = fromData[fromIdx * 2 + 1];
        const auto tx = toData[toIdx * 2];
        const auto ty = toData[toIdx * 2 + 1];

        outData[i * 2] = fx + easedProgress * (tx - fx);
        outData[i * 2 + 1] = fy + easedProgress * (ty - fy);
    }
}

} // namespace QAccelPlot
