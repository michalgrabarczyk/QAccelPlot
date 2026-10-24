//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/formatters/TextTickLabelFormatter.hpp"

#include "QAccelPlot/QAccelPlotLogging.hpp"

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

namespace {

/// \brief Returns how far a tick value may sit from an integer and still be labeled, given the \a tickStep.
///
/// Tick values carry rounding error, so an exact comparison would drop labels. The tolerance shrinks with
/// steps below one so that a tick one step away from an index stays unlabeled on a deeply zoomed axis.
qreal indexTolerance(const qreal tickStep)
{
    constexpr static auto kRelativeTolerance = qreal{1e-6};
    if (!std::isfinite(tickStep) || tickStep <= 0.0) {
        return kRelativeTolerance;
    }
    return kRelativeTolerance * std::min(tickStep, qreal{1.0});
}

} // namespace

TextTickLabelFormatter::TextTickLabelFormatter(QObject* parent)
    : TickLabelFormatter(parent)
{
}

QStringList TextTickLabelFormatter::labels() const
{
    return labels_;
}

void TextTickLabelFormatter::setLabels(const QStringList& labels)
{
    if (labels_ == labels) {
        return;
    }
    labels_ = labels;
    emit labelsChanged();
    emit formatChanged();
}

qreal TextTickLabelFormatter::minimumTickStep() const
{
    return 1.0;
}

QString TextTickLabelFormatter::doFormat(const qreal value, const qreal tickStep) const
{
    if (!std::isfinite(value)) {
        return {};
    }
    const auto index = std::round(value);
    if (std::abs(value - index) > indexTolerance(tickStep)) {
        return {};
    }
    if (index < 0.0 || index >= static_cast<qreal>(labels_.size())) {
        qCDebug(lcQAccelPlot) << "index" << index << "out of range [0," << labels_.size() << "), returning empty string";
        return {};
    }
    return labels_.at(static_cast<qsizetype>(index));
}

} // namespace QAccelPlot
