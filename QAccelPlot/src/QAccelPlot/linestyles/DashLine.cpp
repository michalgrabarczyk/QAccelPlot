//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/linestyles/DashLine.hpp"

#include "QAccelPlot/QAccelPlotLogging.hpp"

#include <algorithm>

namespace QAccelPlot {

DashLine::DashLine(QObject* parent)
    : LineStyle(parent)
{
}

QList<qreal> DashLine::pattern() const
{
    return pattern_;
}

void DashLine::setPattern(const QList<qreal>& pattern)
{
    if (pattern_ == pattern) {
        return;
    }

    if (std::any_of(pattern.cbegin(), pattern.cend(), [](const qreal segment) { return segment < 0.0; })) {
        qCWarning(lcQAccelPlot) << "DashLine pattern segments cannot be negative, ignoring pattern" << pattern;
        return;
    }
    if (pattern.size() > kMaxDashPatternSize) {
        qCWarning(lcQAccelPlot) << "DashLine pattern has" << pattern.size() << "segments, but only the first" << kMaxDashPatternSize << "are rendered";
    }

    pattern_ = pattern;
    emit patternChanged();
    emit styleChanged();
}

DashParameters DashLine::dashParameters() const
{
    if (pattern_.isEmpty()) {
        return {};
    }

    auto params = DashParameters{};
    params.enabled = true;
    params.patternSize = std::min(static_cast<int>(pattern_.size()), kMaxDashPatternSize);

    auto period = 0.0f;
    for (auto i = 0; i < params.patternSize; ++i) {
        const auto seg = static_cast<float>(pattern_[i]);
        params.pattern[i] = seg;
        period += seg;
    }
    params.period = period > 0.0f ? period : 1.0f;
    params.offset = 0.0f;
    return params;
}

} // namespace QAccelPlot
