//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QList>
#include <QObject>
#include <QVariantList>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <cstddef>
#include <vector>

namespace QAccelPlot {

/// \brief Samples counted into bins, convertible to \c BarSeries data.
///
/// A bin spans from its lower edge up to, but not including, its upper edge; the last bin also
/// includes its upper edge. NaN samples and samples outside the edges are not counted.
///
/// The functions are stateless, so a histogram can be built on a worker thread and its data
/// passed to \c postData().
///
/// \code
/// const auto histogram = Histogram::fromSamples(samples.data(), samples.size(), 40);
/// bars->setRangedData(histogram.barData(), histogram.binCount());
/// \endcode
///
/// In QML, histograms are created by the \c Histogram singleton, see HistogramFactory.
///
/// \sa HistogramFactory, BarSeries
class Histogram {
    Q_GADGET
    QML_ANONYMOUS

    /// \brief Read-only: number of bins; 0 for an invalid histogram.
    Q_PROPERTY(int binCount READ binCount CONSTANT)
    /// \brief Read-only: the \c binCount + 1 bin edges in ascending order.
    Q_PROPERTY(QList<qreal> edges READ edgeList CONSTANT)
    /// \brief Read-only: one value per bin, a count or, after \c density(), a density.
    Q_PROPERTY(QList<qreal> values READ valueList CONSTANT)
    /// \brief Read-only: sum of the bin counts, the number of samples inside the edges.
    Q_PROPERTY(qreal total READ total CONSTANT)
    /// \brief Read-only: true when all bins have the same width.
    Q_PROPERTY(bool uniform READ isUniform CONSTANT)
    /// \brief Read-only: the width of every bin, or NaN when the bins are not uniform.
    Q_PROPERTY(qreal binWidth READ binWidth CONSTANT)

public:
    /// \brief Counts \a sampleCount \a samples into \a binCount equal bins spanning the finite samples.
    ///
    /// The range is 0 to 1 without finite samples, and widens by 0.5 on each side when all samples are equal.
    static Histogram fromSamples(const double* samples, std::size_t sampleCount, int binCount);
    /// \brief Counts \a sampleCount \a samples into \a binCount equal bins from \a min to \a max.
    static Histogram fromSamples(const double* samples, std::size_t sampleCount, int binCount, double min, double max);
    /// \brief Counts \a sampleCount \a samples into the bins between \a edges, which must be finite and strictly increasing.
    static Histogram fromSamples(const double* samples, std::size_t sampleCount, std::vector<double> edges);
    /// \brief Float overload of \c fromSamples(\a samples, \a sampleCount, \a binCount).
    static Histogram fromSamples(const float* samples, std::size_t sampleCount, int binCount);
    /// \brief Float overload of \c fromSamples(\a samples, \a sampleCount, \a binCount, \a min, \a max).
    static Histogram fromSamples(const float* samples, std::size_t sampleCount, int binCount, double min, double max);
    /// \brief Float overload of \c fromSamples(\a samples, \a sampleCount, \a edges).
    static Histogram fromSamples(const float* samples, std::size_t sampleCount, std::vector<double> edges);
    /// \brief Wraps data that is already binned: \a counts has one finite, non-negative value per bin between \a edges.
    static Histogram fromCounts(std::vector<double> edges, std::vector<double> counts);

    /// \brief Returns \a binCount + 1 equally spaced edges from \a min to \a max, or an empty vector for invalid arguments.
    static std::vector<double> linearEdges(double min, double max, int binCount);
    /// \brief Returns \a binCount + 1 edges from \a min to \a max, equally spaced on a logarithmic axis.
    ///
    /// \a min must be positive. Returns an empty vector for invalid arguments.
    static std::vector<double> logEdges(double min, double max, int binCount);
    /// \brief Returns the edges from \a min to \a max at 1, 2, ..., 9 times each power of ten: the ticks of a logarithmic axis.
    ///
    /// \a min and \a max are the first and the last edge. \a min must be positive. Returns an
    /// empty vector for invalid arguments. The bins widen tenfold at each power of ten, so draw
    /// them as \c density().
    static std::vector<double> decadeEdges(double min, double max);

    /// \brief Returns the number of bins; 0 for an invalid histogram.
    int binCount() const;
    /// \brief Returns the \c binCount() + 1 bin edges in ascending order.
    const std::vector<double>& edges() const;
    /// \brief Returns one value per bin, a count or, after \c density(), a density.
    const std::vector<double>& values() const;
    /// \brief Returns the sum of the bin counts, the number of samples inside the edges.
    double total() const;
    /// \brief Returns true when all bins have the same width.
    bool isUniform() const;
    /// \brief Returns the width of every bin, or NaN when the bins are not uniform.
    double binWidth() const;

    /// \brief Returns a copy whose values are densities: count / (\c total() × bin width).
    ///
    /// The bin areas sum to 1, so bins of different widths and data sets of different sizes are
    /// comparable. Returns the histogram unchanged when it already holds densities.
    Q_INVOKABLE ::QAccelPlot::Histogram density() const;

    /// \brief Returns \c binCount() interleaved (from, to, value) bars, one per bin, for \c BarSeries::setRangedData().
    std::vector<double> barData() const;
    /// \brief Returns one object per bin with \c from, \c to, and \c value, for the QML \c BarSeries.setData().
    Q_INVOKABLE QVariantList bars() const;

private:
    template <typename Sample> static Histogram spanning(const Sample* samples, std::size_t sampleCount, int binCount);
    template <typename Sample> static Histogram binned(const Sample* samples, std::size_t sampleCount, std::vector<double> edges);

    QList<qreal> edgeList() const;
    QList<qreal> valueList() const;

    std::vector<double> edges_;
    std::vector<double> values_;
    double total_{0.0};
    bool uniform_{false};
    bool density_{false};
};

} // namespace QAccelPlot
