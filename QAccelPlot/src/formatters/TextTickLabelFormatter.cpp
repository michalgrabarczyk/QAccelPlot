//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "formatters/TextTickLabelFormatter.hpp"

#include "QAccelPlotLogging.hpp"

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

QString TextTickLabelFormatter::doFormat(const qreal value, const qreal tickStep) const
{
    const auto index = qRound(value);
    if (index < 0 || index >= labels_.size()) {
        qCDebug(lcQAccelPlot) << "index" << index << "out of range [0," << labels_.size() << "), returning empty string";
        return {};
    }
    return labels_.at(index);
}

} // namespace QAccelPlot
