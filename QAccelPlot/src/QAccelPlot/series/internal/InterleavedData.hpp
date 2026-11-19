//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

namespace QAccelPlot::Internal {

/// \brief Returns the size of the shortest of \a lists.
template <typename... Lists> std::size_t shortestLength(const Lists&... lists)
{
    return std::min({static_cast<std::size_t>(lists.size())...});
}

/// \brief Interleaves the first shortestLength() values of \a columns into data points of one value per column.
template <typename... Lists> std::vector<double> interleave(const Lists&... columns)
{
    constexpr auto kStride = sizeof...(Lists);
    const auto count = shortestLength(columns...);
    auto data = std::vector<double>(count * kStride);
    for (auto i = std::size_t{0}; i < count; ++i) {
        const auto dataPoint = std::array<double, kStride>{columns[i]...};
        std::copy(dataPoint.cbegin(), dataPoint.cend(), data.begin() + static_cast<std::ptrdiff_t>(i * kStride));
    }
    return data;
}

} // namespace QAccelPlot::Internal
