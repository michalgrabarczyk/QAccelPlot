

# File ItemFade.hpp

[**File List**](files.md) **>** [**internal**](dir_78f1acf6f1aae410b1e370fefb18a5f4.md) **>** [**ItemFade.hpp**](ItemFade_8hpp.md)

[Go to the documentation of this file](ItemFade_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <array>
#include <vector>

namespace QAccelPlot::Internal {

inline constexpr int kFadeCurveSamples = 32;
inline constexpr int kFadeStartSamples = 16;
inline constexpr int kFadeGroupCount = 2;
inline constexpr int kTrailEasingSamples = 16;

struct ItemFadeGroup {
    int first{0};       
    int count{0};       
    bool exit{false};   
    bool front{false};  
    float time{0.0f};   
    float spread{0.0f}; 
    float length{0.0f}; 
    std::array<float, kFadeCurveSamples> curve{};
    std::array<float, kFadeStartSamples> starts{};

    [[nodiscard]] bool active() const noexcept
    {
        return count > 0;
    }
};

struct ItemFade {
    std::array<ItemFadeGroup, kFadeGroupCount> groups{}; 

    [[nodiscard]] bool active() const noexcept
    {
        return groups[0].active() || groups[1].active();
    }
};

[[nodiscard]] ItemFadeGroup compactedFadeGroup(const ItemFadeGroup& group, int first, const std::vector<int>& kept);

[[nodiscard]] int compareItemFades(const ItemFade& left, const ItemFade& right) noexcept;

struct ItemTrail {
    float length{0.0f};     
    float minOpacity{1.0f}; 
    int itemCount{0};       
    std::array<float, kTrailEasingSamples> easing{};

    [[nodiscard]] bool active() const noexcept
    {
        return length > 0.0f && minOpacity < 1.0f && itemCount > 1;
    }
};

[[nodiscard]] int compareItemTrails(const ItemTrail& left, const ItemTrail& right) noexcept;

} // namespace QAccelPlot::Internal
```


