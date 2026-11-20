//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QList>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <vector>

namespace QAccelPlot::Internal {

/// \brief Returns the size of the shortest of \a lists.
template <typename... Lists> std::size_t shortestLength(const Lists&... lists)
{
    return std::min({static_cast<std::size_t>(lists.size())...});
}

/// \brief Interleaves the first values of \a columns, as many as the shortest column holds, into data points of one value per column.
std::vector<double> interleave(std::initializer_list<std::reference_wrapper<const QList<qreal>>> columns);
/// \brief \c std::vector overload of \c interleave(\a columns).
std::vector<double> interleave(std::initializer_list<std::reference_wrapper<const std::vector<double>>> columns);

} // namespace QAccelPlot::Internal
