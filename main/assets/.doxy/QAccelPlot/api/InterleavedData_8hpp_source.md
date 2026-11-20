

# File InterleavedData.hpp

[**File List**](files.md) **>** [**internal**](dir_70e6e0d61970c92b37b608a046280901.md) **>** [**InterleavedData.hpp**](InterleavedData_8hpp.md)

[Go to the documentation of this file](InterleavedData_8hpp.md)


```C++
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

template <typename... Lists> std::size_t shortestLength(const Lists&... lists)
{
    return std::min({static_cast<std::size_t>(lists.size())...});
}

std::vector<double> interleave(std::initializer_list<std::reference_wrapper<const QList<qreal>>> columns);
std::vector<double> interleave(std::initializer_list<std::reference_wrapper<const std::vector<double>>> columns);

} // namespace QAccelPlot::Internal
```


