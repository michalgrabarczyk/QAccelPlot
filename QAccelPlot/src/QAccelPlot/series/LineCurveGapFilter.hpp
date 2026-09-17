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

/// \brief Contiguous range of valid curve samples, used to break fills and hit tests at gaps.
struct SampleRun {
    int start; ///< \brief Index of the first valid sample in the run.
    int count; ///< \brief Number of consecutive valid samples.
};

/// \brief Stateless helpers implementing the invalid-sample contract shared by LineCurve subsystems.
///
/// A sample is invalid when either coordinate is non-finite, or is not strictly positive on a
/// logarithmic dimension. See \c isValidSample() in \c MathUtils.hpp.
namespace LineCurveGapFilter {

/// \brief Returns \c true when both coordinates of sample \a index in \a data are valid.
[[nodiscard]] bool isValidPoint(const CurveDataView& data, int index, bool logScaleX, bool logScaleY);

/// \brief Returns the number of invalid samples among the first \a pointCount samples of \a data.
[[nodiscard]] int countInvalidPoints(const CurveDataView& data, int pointCount, bool logScaleX, bool logScaleY);

/// \brief Returns the maximal runs of consecutive valid samples in \a data.
[[nodiscard]] std::vector<SampleRun> findValidRuns(const CurveDataView& data, int pointCount, bool logScaleX, bool logScaleY);

/// \brief Distributes a budget of \a maxSampledPoints across \a runs.
///
/// Returns one sampled count per run. Runs with fewer than two samples receive zero (they cannot
/// form an area). Every other run keeps at least its first and last sample, so the total may
/// exceed the budget by at most two samples per run when a curve contains very many gaps.
[[nodiscard]] std::vector<int> planRunSampling(const std::vector<SampleRun>& runs, int maxSampledPoints);

/// \brief Maps a sampled position \a sampleIndex of \a sampledCount onto a source index within \a run.
[[nodiscard]] int sampledSourceIndex(const SampleRun& run, int sampleIndex, int sampledCount);

/// \brief Copies the valid samples of interleaved float \a data into \a output.
///
/// Returns the number of samples written. \a output is resized to exactly that many XY pairs.
int compactValidPoints(const std::vector<float>& data, int pointCount, bool logScaleX, bool logScaleY, std::vector<float>& output);

/// \brief Copies the valid samples of interleaved double \a data into \a output and the matching
/// entries of the parallel float \a renderData into \a renderOutput.
///
/// Validity is decided from the double values. Returns the number of samples written.
int compactValidPoints(const std::vector<double>& data, const std::vector<float>& renderData, int pointCount, bool logScaleX, bool logScaleY,
    std::vector<double>& output, std::vector<float>& renderOutput);

} // namespace LineCurveGapFilter

} // namespace QAccelPlot
