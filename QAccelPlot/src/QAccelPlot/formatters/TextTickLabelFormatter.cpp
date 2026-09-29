//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/formatters/TextTickLabelFormatter.hpp"

#include "QAccelPlot/QAccelPlotLogging.hpp"

#include <cmath>

namespace QAccelPlot {

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

QString TextTickLabelFormatter::doFormat(const qreal value, [[maybe_unused]] const qreal tickStep) const
{
    if (!std::isfinite(value)) {
        return {};
    }
    const auto index = std::round(value);
    if (index < 0.0 || index >= static_cast<qreal>(labels_.size())) {
        qCDebug(lcQAccelPlot) << "index" << index << "out of range [0," << labels_.size() << "), returning empty string";
        return {};
    }
    return labels_.at(static_cast<qsizetype>(index));
}

} // namespace QAccelPlot
