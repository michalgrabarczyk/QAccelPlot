//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <algorithm>
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

    /// \brief Returns the bytes held by the records.
    std::size_t byteSize() const;
};

/// \brief Returns \c true when \a a and \a b describe the same dataset.
bool operator==(const DatasetParameters& a, const DatasetParameters& b);

/// \brief Returns the bytes held by every part of \a parts; \a Part has a \c byteSize() member.
template <typename Part> std::size_t partsByteSize(const std::vector<Part>& parts)
{
    auto bytes = std::size_t{0};
    for (const auto& part : parts) {
        bytes += part.byteSize();
    }
    return bytes;
}

/// \brief Calls \a body for every part of \a parts that holds records of the dataset range <tt>[begin, end)</tt>.
///
/// The parts hold consecutive records of one dataset. \a body receives the part, the range of the
/// part's own records <tt>[first, last)</tt>, and the dataset index of the part's first record.
template <typename Part, typename Body> void forEachPartSlice(std::vector<Part>& parts, const int begin, const int end, Body&& body)
{
    auto partBegin = 0;
    for (auto& part : parts) {
        const auto partEnd = partBegin + part.count;
        const auto first = std::max(begin, partBegin);
        const auto last = std::min(end, partEnd);
        if (first < last) {
            body(part, first - partBegin, last - partBegin, partBegin);
        }
        if (partEnd >= end) {
            return;
        }
        partBegin = partEnd;
    }
}

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
