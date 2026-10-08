//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/data/Histogram.hpp"

#include "QAccelPlot/QAccelPlotLogging.hpp"

#include <QVariantMap>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <utility>

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();

bool hasSamples(const void* samples, const std::size_t sampleCount)
{
    if (sampleCount > 0 && !samples) {
        qCWarning(lcQAccelPlot) << "Histogram received a null sample pointer for" << sampleCount << "samples";
        return false;
    }
    return true;
}

bool isValidBinCount(const int binCount)
{
    if (binCount < 1) {
        qCWarning(lcQAccelPlot) << "Histogram bin count must be at least 1:" << binCount;
        return false;
    }
    return true;
}

bool isValidRange(const double min, const double max)
{
    if (!std::isfinite(min) || !std::isfinite(max) || min >= max) {
        qCWarning(lcQAccelPlot) << "Histogram range must be finite and increasing:" << min << "to" << max;
        return false;
    }
    return true;
}

bool isPositiveMinimum(const double min)
{
    if (min <= 0.0) {
        qCWarning(lcQAccelPlot) << "Histogram logarithmic edges need a positive minimum:" << min;
        return false;
    }
    return true;
}

// Returns mantissa × 10^exponent. Dividing for negative exponents gives the nearest double, e.g. exactly 0.3.
double decadeValue(const int mantissa, const int exponent)
{
    return exponent >= 0 ? mantissa * std::pow(10.0, exponent) : mantissa / std::pow(10.0, -exponent);
}

bool areValidEdges(const std::vector<double>& edges)
{
    const auto valid = edges.size() >= 2 && std::isfinite(edges.front()) && std::isfinite(edges.back())
        && std::adjacent_find(edges.cbegin(), edges.cend(), [](const double low, const double high) { return !(low < high); }) == edges.cend();
    if (!valid) {
        qCWarning(lcQAccelPlot) << "Histogram needs at least two finite, strictly increasing edges";
    }
    return valid;
}

bool areUniformEdges(const std::vector<double>& edges)
{
    const auto width = (edges.back() - edges.front()) / static_cast<double>(edges.size() - 1);
    // Edges far from zero are rounded by more than a fraction of a narrow bin.
    const auto rounding = 4.0 * std::numeric_limits<double>::epsilon() * std::max(std::abs(edges.front()), std::abs(edges.back()));
    const auto tolerance = 1e-9 * width + rounding;
    for (auto i = std::size_t{1}; i < edges.size(); ++i) {
        if (std::abs(edges[i] - edges[i - 1] - width) > tolerance) {
            return false;
        }
    }
    return true;
}

// Returns the range of the finite samples, widened when it is empty.
template <typename Sample> std::pair<double, double> finiteRange(const Sample* samples, const std::size_t sampleCount)
{
    auto min = std::numeric_limits<double>::infinity();
    auto max = -std::numeric_limits<double>::infinity();
    for (auto i = std::size_t{0}; i < sampleCount; ++i) {
        const auto sample = static_cast<double>(samples[i]);
        if (std::isfinite(sample)) {
            min = std::min(min, sample);
            max = std::max(max, sample);
        }
    }
    if (min > max) {
        return {0.0, 1.0};
    }
    if (min == max) {
        // Half a unit is below the precision of very large values.
        const auto margin = std::max(0.5, std::abs(min) * 1e-9);
        return {min - margin, max + margin};
    }
    return {min, max};
}

std::size_t uniformBin(const std::vector<double>& edges, const double sample, const double binsPerUnit)
{
    const auto lastBin = edges.size() - 2;
    auto bin = std::min(static_cast<std::size_t>((sample - edges.front()) * binsPerUnit), lastBin);
    // Rounding can place a sample near an edge in the neighboring bin; the edges decide.
    while (sample < edges[bin]) {
        --bin;
    }
    while (bin < lastBin && sample >= edges[bin + 1]) {
        ++bin;
    }
    return bin;
}

std::size_t searchedBin(const std::vector<double>& edges, const double sample)
{
    const auto upper = std::upper_bound(edges.cbegin(), edges.cend(), sample);
    return std::min(static_cast<std::size_t>(upper - edges.cbegin()) - 1, edges.size() - 2);
}

template <typename Sample>
std::vector<double> countSamples(const Sample* samples, const std::size_t sampleCount, const std::vector<double>& edges, const bool uniform)
{
    const auto first = edges.front();
    const auto last = edges.back();
    const auto binsPerUnit = static_cast<double>(edges.size() - 1) / (last - first);
    auto counts = std::vector<double>(edges.size() - 1, 0.0);
    for (auto i = std::size_t{0}; i < sampleCount; ++i) {
        const auto sample = static_cast<double>(samples[i]);
        // NaN fails both comparisons.
        if (sample >= first && sample <= last) {
            counts[uniform ? uniformBin(edges, sample, binsPerUnit) : searchedBin(edges, sample)] += 1.0;
        }
    }
    return counts;
}

}

Histogram Histogram::fromSamples(const double* samples, const std::size_t sampleCount, const int binCount)
{
    return spanning(samples, sampleCount, binCount);
}

Histogram Histogram::fromSamples(const double* samples, const std::size_t sampleCount, const int binCount, const double min, const double max)
{
    auto edges = linearEdges(min, max, binCount);
    return edges.empty() ? Histogram{} : binned(samples, sampleCount, std::move(edges));
}

Histogram Histogram::fromSamples(const double* samples, const std::size_t sampleCount, std::vector<double> edges)
{
    return binned(samples, sampleCount, std::move(edges));
}

Histogram Histogram::fromSamples(const float* samples, const std::size_t sampleCount, const int binCount)
{
    return spanning(samples, sampleCount, binCount);
}

Histogram Histogram::fromSamples(const float* samples, const std::size_t sampleCount, const int binCount, const double min, const double max)
{
    auto edges = linearEdges(min, max, binCount);
    return edges.empty() ? Histogram{} : binned(samples, sampleCount, std::move(edges));
}

Histogram Histogram::fromSamples(const float* samples, const std::size_t sampleCount, std::vector<double> edges)
{
    return binned(samples, sampleCount, std::move(edges));
}

Histogram Histogram::fromCounts(std::vector<double> edges, std::vector<double> counts)
{
    if (!areValidEdges(edges)) {
        return {};
    }
    if (counts.size() != edges.size() - 1) {
        qCWarning(lcQAccelPlot) << "Histogram received" << counts.size() << "counts for" << edges.size() - 1 << "bins";
        return {};
    }
    if (!std::all_of(counts.cbegin(), counts.cend(), [](const double count) { return std::isfinite(count) && count >= 0.0; })) {
        qCWarning(lcQAccelPlot) << "Histogram counts must be finite and non-negative";
        return {};
    }
    auto histogram = Histogram{};
    histogram.uniform_ = areUniformEdges(edges);
    histogram.total_ = std::accumulate(counts.cbegin(), counts.cend(), 0.0);
    histogram.edges_ = std::move(edges);
    histogram.values_ = std::move(counts);
    return histogram;
}

std::vector<double> Histogram::linearEdges(const double min, const double max, const int binCount)
{
    if (!isValidBinCount(binCount) || !isValidRange(min, max)) {
        return {};
    }
    auto edges = std::vector<double>(static_cast<std::size_t>(binCount) + 1);
    for (auto i = std::size_t{0}; i < edges.size(); ++i) {
        edges[i] = min + (max - min) * static_cast<double>(i) / static_cast<double>(binCount);
    }
    edges.back() = max;
    return edges;
}

std::vector<double> Histogram::logEdges(const double min, const double max, const int binCount)
{
    if (!isValidBinCount(binCount) || !isValidRange(min, max) || !isPositiveMinimum(min)) {
        return {};
    }
    const auto logMin = std::log10(min);
    const auto logMax = std::log10(max);
    auto edges = std::vector<double>(static_cast<std::size_t>(binCount) + 1);
    for (auto i = std::size_t{0}; i < edges.size(); ++i) {
        edges[i] = std::pow(10.0, logMin + (logMax - logMin) * static_cast<double>(i) / static_cast<double>(binCount));
    }
    edges.front() = min;
    edges.back() = max;
    return edges;
}

std::vector<double> Histogram::decadeEdges(const double min, const double max)
{
    if (!isValidRange(min, max) || !isPositiveMinimum(min)) {
        return {};
    }
    // An end of the range that equals a tick up to rounding, such as 3 * 0.1 for 0.3, is that tick.
    // Without the tolerance the two would become the edges of an extra bin of nearly zero width.
    constexpr auto kTolerance = 1e-9;
    const auto firstExponent = static_cast<int>(std::floor(std::log10(min)));
    const auto lastExponent = static_cast<int>(std::ceil(std::log10(max)));
    auto edges = std::vector<double>{min};
    for (auto exponent = firstExponent; exponent <= lastExponent; ++exponent) {
        for (auto mantissa = 1; mantissa <= 9; ++mantissa) {
            const auto edge = decadeValue(mantissa, exponent);
            if (edge > min * (1.0 + kTolerance) && edge < max * (1.0 - kTolerance)) {
                edges.push_back(edge);
            }
        }
    }
    edges.push_back(max);
    return edges;
}

int Histogram::binCount() const
{
    return static_cast<int>(values_.size());
}

const std::vector<double>& Histogram::edges() const
{
    return edges_;
}

const std::vector<double>& Histogram::values() const
{
    return values_;
}

double Histogram::total() const
{
    return total_;
}

bool Histogram::isUniform() const
{
    return uniform_;
}

double Histogram::binWidth() const
{
    return uniform_ ? (edges_.back() - edges_.front()) / static_cast<double>(values_.size()) : kNaN;
}

Histogram Histogram::density() const
{
    if (density_ || values_.empty()) {
        return *this;
    }
    auto result = *this;
    result.density_ = true;
    for (auto i = std::size_t{0}; i < values_.size(); ++i) {
        const auto area = total_ * (edges_[i + 1] - edges_[i]);
        result.values_[i] = total_ > 0.0 ? values_[i] / area : 0.0;
    }
    return result;
}

std::vector<double> Histogram::barData() const
{
    auto data = std::vector<double>(values_.size() * 3);
    for (auto i = std::size_t{0}; i < values_.size(); ++i) {
        data[i * 3] = edges_[i];
        data[i * 3 + 1] = edges_[i + 1];
        data[i * 3 + 2] = values_[i];
    }
    return data;
}

QVariantList Histogram::bars() const
{
    auto list = QVariantList{};
    list.reserve(static_cast<qsizetype>(values_.size()));
    for (auto i = std::size_t{0}; i < values_.size(); ++i) {
        list.append(QVariantMap{{QStringLiteral("from"), edges_[i]}, {QStringLiteral("to"), edges_[i + 1]}, {QStringLiteral("value"), values_[i]}});
    }
    return list;
}

template <typename Sample> Histogram Histogram::spanning(const Sample* samples, const std::size_t sampleCount, const int binCount)
{
    if (!hasSamples(samples, sampleCount)) {
        return {};
    }
    const auto [min, max] = finiteRange(samples, sampleCount);
    auto edges = linearEdges(min, max, binCount);
    return edges.empty() ? Histogram{} : binned(samples, sampleCount, std::move(edges));
}

template <typename Sample> Histogram Histogram::binned(const Sample* samples, const std::size_t sampleCount, std::vector<double> edges)
{
    if (!hasSamples(samples, sampleCount) || !areValidEdges(edges)) {
        return {};
    }
    auto histogram = Histogram{};
    histogram.uniform_ = areUniformEdges(edges);
    histogram.values_ = countSamples(samples, sampleCount, edges, histogram.uniform_);
    histogram.total_ = std::accumulate(histogram.values_.cbegin(), histogram.values_.cend(), 0.0);
    histogram.edges_ = std::move(edges);
    return histogram;
}

QList<qreal> Histogram::edgeList() const
{
    return QList<qreal>(edges_.cbegin(), edges_.cend());
}

QList<qreal> Histogram::valueList() const
{
    return QList<qreal>(values_.cbegin(), values_.cend());
}

} // namespace QAccelPlot
