//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/data/HistogramFactory.hpp"

#include <QJSValue>

#include <limits>
#include <vector>

namespace QAccelPlot {

namespace {

std::vector<double> toVector(const QList<qreal>& list)
{
    return std::vector<double>(list.cbegin(), list.cend());
}

QList<qreal> toList(const std::vector<double>& values)
{
    return QList<qreal>(values.cbegin(), values.cend());
}

bool isNumber(const QVariant& value)
{
    switch (value.typeId()) {
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Float:
    case QMetaType::Double:
        return true;
    default:
        return false;
    }
}

// Converts a JavaScript array or a list returned by one of the edge functions. An item that
// is not a number becomes NaN, which the histogram rejects as an edge.
std::vector<double> toEdges(const QVariant& value)
{
    const auto list = value.toList();
    auto edges = std::vector<double>{};
    edges.reserve(static_cast<std::size_t>(list.size()));
    for (const auto& item : list) {
        auto ok = false;
        const auto edge = item.toDouble(&ok);
        edges.push_back(ok ? edge : std::numeric_limits<double>::quiet_NaN());
    }
    return edges;
}

}

HistogramFactory::HistogramFactory(QObject* parent)
    : QObject(parent)
{
}

Histogram HistogramFactory::fromSamples(const QList<qreal>& samples, const QVariant& bins) const
{
    const auto sampleCount = static_cast<std::size_t>(samples.size());
    // Depending on the Qt version, a JavaScript array arrives as a list or wrapped in a QJSValue.
    const auto value = bins.metaType() == QMetaType::fromType<QJSValue>() ? bins.value<QJSValue>().toVariant() : bins;
    if (isNumber(value)) {
        return Histogram::fromSamples(samples.constData(), sampleCount, value.toInt());
    }
    return Histogram::fromSamples(samples.constData(), sampleCount, toEdges(value));
}

Histogram HistogramFactory::fromSamples(const QList<qreal>& samples, const int binCount, const qreal min, const qreal max) const
{
    return Histogram::fromSamples(samples.constData(), static_cast<std::size_t>(samples.size()), binCount, min, max);
}

Histogram HistogramFactory::fromCounts(const QList<qreal>& edges, const QList<qreal>& counts) const
{
    return Histogram::fromCounts(toVector(edges), toVector(counts));
}

QList<qreal> HistogramFactory::linearEdges(const qreal min, const qreal max, const int binCount) const
{
    return toList(Histogram::linearEdges(min, max, binCount));
}

QList<qreal> HistogramFactory::logEdges(const qreal min, const qreal max, const int binCount) const
{
    return toList(Histogram::logEdges(min, max, binCount));
}

QList<qreal> HistogramFactory::decadeEdges(const qreal min, const qreal max) const
{
    return toList(Histogram::decadeEdges(min, max));
}

} // namespace QAccelPlot
