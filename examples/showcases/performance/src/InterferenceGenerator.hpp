//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "DatasetParts.hpp"

#include <vector>

namespace QAccelPlotExample {

/// \brief One series' share of a BarSeries dataset: 2 floats per bar (position, value).
struct InterferencePart : SeriesPart {
    /// \brief One category per bar; empty when categories are off.
    std::vector<int> categories;
};

/// \brief One BarSeries dataset, split into consecutive runs of bars.
struct InterferenceBatch {
    std::vector<InterferencePart> parts;
};

/// \brief Settings of an InterferenceGenerator batch.
struct InterferenceParameters {
    DatasetParameters dataset;
    /// \brief Number of categories the value of a bar is binned into; 0 turns categories off.
    int categoryCount{0};
};

/// \brief Generates bars whose values follow three sine waves drifting at different speeds.
///
/// A bar's position is its index, which a float holds exactly for every bar count the showcase
/// offers. The waves span the dataset a fixed number of times whatever the bar count, and come
/// from \c SineLookupCursor, so no trigonometry runs per bar.
class InterferenceGenerator final {
public:
    using Batch = InterferenceBatch;
    using Parameters = InterferenceParameters;

    /// \brief Lowest value a bar can take.
    static constexpr auto kMinimumValue = 0.02f;
    /// \brief Highest value a bar can take.
    static constexpr auto kMaximumValue = 0.98f;

    /// \brief Fills \a batch with the bars described by \a parameters at \a timeSeconds.
    void generate(Batch& batch, const Parameters& parameters, double timeSeconds);

    /// \brief Returns the value of bar \a index out of \a barCount at \a timeSeconds.
    static float valueAt(int index, int barCount, double timeSeconds);
};

} // namespace QAccelPlotExample
