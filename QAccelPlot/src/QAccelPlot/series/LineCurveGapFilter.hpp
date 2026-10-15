//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/renderers/CurveRendererParams.hpp"

#include <vector>

namespace QAccelPlot {

/// \brief Contiguous range of valid curve data points, used to break fills and hit tests at gaps.
struct SampleRun {
    int start; ///< \brief Index of the first valid data point in the run.
    int count; ///< \brief Number of consecutive valid data points.
};

/// \brief Stateless helpers implementing the invalid-data-point contract shared by LineCurve subsystems.
///
/// A data point is invalid when either coordinate is non-finite, or is not strictly positive on a
/// logarithmic dimension. See \c isValidSample() in \c MathUtils.hpp.
namespace LineCurveGapFilter {

/// \brief Returns \c true when both coordinates of data point \a index in \a data are valid.
[[nodiscard]] bool isValidPoint(const CurveDataView& data, int index, bool logScaleX, bool logScaleY);

/// \brief Returns the maximal runs of consecutive valid data points in \a data.
[[nodiscard]] std::vector<SampleRun> findValidRuns(const CurveDataView& data, int pointCount, bool logScaleX, bool logScaleY);

/// \brief Distributes a budget of \a maxSampledPoints across \a runs.
///
/// Returns one sampled count per run. Runs with fewer than two samples receive zero (they cannot
/// form an area). Every other run keeps at least its first and last data point, so the total may
/// exceed the budget by at most two data points per run when a curve contains very many gaps.
[[nodiscard]] std::vector<int> planRunSampling(const std::vector<SampleRun>& runs, int maxSampledPoints);

/// \brief Appends \a sampledCount source indices of \a run in \a data to \a indices, in index order.
///
/// Keeps every sample when \a sampledCount covers the run. Otherwise keeps the run's endpoints and
/// the lowest and highest Y data point of equally sized buckets, so the decimated curve keeps its peaks.
void appendEnvelopeSamples(const CurveDataView& data, const SampleRun& run, int sampledCount, std::vector<int>& indices);

} // namespace LineCurveGapFilter

} // namespace QAccelPlot
