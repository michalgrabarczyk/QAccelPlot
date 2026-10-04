//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <cstddef>
#include <vector>

namespace QAccelPlotExample {

/// \brief Size and layout of a generated dataset, shared by every generator.
struct DatasetParameters {
    /// \brief Total number of records across all series.
    int count{1};
    /// \brief Number of series the records are split across.
    int seriesCount{1};
    /// \brief Whether the records are delivered as doubles instead of floats.
    bool doublePrecision{false};
};

/// \brief One series' share of a generated dataset.
struct SeriesPart {
    /// \brief Interleaved records; empty when the part carries doubles.
    std::vector<float> floats;
    /// \brief The same records as doubles; empty unless double precision was requested.
    std::vector<double> doubles;
    /// \brief Number of records.
    int count{0};
};

/// \brief Returns how many of \a total records series \a index gets when they are split evenly across \a seriesCount series.
int partCount(int total, int seriesCount, int index);

/// \brief Moves the float records of \a part into its doubles.
void widenToDoubles(SeriesPart& part);

/// \brief Resizes \a parts to hold the dataset described by \a dataset, with \a floatsPerRecord floats per record.
///
/// \a Part derives from \c SeriesPart.
template <typename Part> void resizeParts(std::vector<Part>& parts, const DatasetParameters& dataset, const int floatsPerRecord)
{
    parts.resize(static_cast<std::size_t>(dataset.seriesCount));
    for (auto index = 0; index < dataset.seriesCount; ++index) {
        auto& part = parts[static_cast<std::size_t>(index)];
        part.count = partCount(dataset.count, dataset.seriesCount, index);
        part.floats.resize(static_cast<std::size_t>(part.count) * static_cast<std::size_t>(floatsPerRecord));
        part.doubles.clear();
    }
}

/// \brief Widens every part of \a parts when \a dataset asks for double precision.
template <typename Part> void applyPrecision(std::vector<Part>& parts, const DatasetParameters& dataset)
{
    if (!dataset.doublePrecision) {
        return;
    }
    for (auto& part : parts) {
        widenToDoubles(part);
    }
}

} // namespace QAccelPlotExample
