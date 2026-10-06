

# File HoverIndexBudget.hpp

[**File List**](files.md) **>** [**internal**](dir_70e6e0d61970c92b37b608a046280901.md) **>** [**HoverIndexBudget.hpp**](HoverIndexBudget_8hpp.md)

[Go to the documentation of this file](HoverIndexBudget_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <chrono>
#include <utility>

namespace QAccelPlot::Internal {

class HoverIndexBudget {
public:
    using CostPerRecord = std::chrono::duration<double, std::nano>;

    explicit HoverIndexBudget(CostPerRecord assumedBuildCostPerRecord);

    void reset();
    bool buildDue(int recordCount) const;
    void addScan(std::chrono::nanoseconds duration);
    void addBuild(std::chrono::nanoseconds duration, int recordCount);

    template <typename Scan> auto timeScan(Scan&& scan)
    {
        const auto start = std::chrono::steady_clock::now();
        auto result = std::forward<Scan>(scan)();
        addScan(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start));
        return result;
    }

    template <typename Build> void timeBuild(const int recordCount, Build&& build)
    {
        const auto start = std::chrono::steady_clock::now();
        std::forward<Build>(build)();
        addBuild(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start), recordCount);
    }

private:
    CostPerRecord buildCostPerRecord_;
    std::chrono::nanoseconds scanCost_{0};
};

} // namespace QAccelPlot::Internal
```


