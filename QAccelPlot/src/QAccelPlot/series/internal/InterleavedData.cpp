//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/series/internal/InterleavedData.hpp"

#include <limits>

namespace QAccelPlot::Internal {

namespace {

/// \brief Interleaves the first values of \a columns, as many as the shortest column holds, into data points of one value per column.
template <typename List> std::vector<double> interleaveColumns(const std::initializer_list<std::reference_wrapper<const List>> columns)
{
    const auto columnCount = columns.size();
    auto count = columnCount == 0 ? std::size_t{0} : std::numeric_limits<std::size_t>::max();
    for (const auto& column : columns) {
        count = std::min(count, static_cast<std::size_t>(column.get().size()));
    }
    auto data = std::vector<double>(count * columnCount);
    auto columnIndex = std::size_t{0};
    for (const auto& column : columns) {
        auto value = column.get().cbegin();
        for (auto i = columnIndex; i < data.size(); i += columnCount) {
            data[i] = *value++;
        }
        ++columnIndex;
    }
    return data;
}

} // namespace

std::vector<double> interleave(const std::initializer_list<std::reference_wrapper<const QList<qreal>>> columns)
{
    return interleaveColumns(columns);
}

std::vector<double> interleave(const std::initializer_list<std::reference_wrapper<const std::vector<double>>> columns)
{
    return interleaveColumns(columns);
}

} // namespace QAccelPlot::Internal
