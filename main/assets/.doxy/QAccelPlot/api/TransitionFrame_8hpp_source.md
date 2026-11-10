

# File TransitionFrame.hpp

[**File List**](files.md) **>** [**internal**](dir_698dae9af7fc703eaceb77bfe4886cff.md) **>** [**TransitionFrame.hpp**](TransitionFrame_8hpp.md)

[Go to the documentation of this file](TransitionFrame_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/internal/ItemFade.hpp"
#include "QAccelPlot/transitions/internal/OpacityTimeline.hpp"

#include <QEasingCurve>

#include <array>
#include <limits>
#include <optional>
#include <vector>

namespace QAccelPlot::Internal {

enum class TransitionKind {
    Morph, 
    Draw,  
    Grow,  
};

struct TransitionLayout {
    int stride{2};                    
    std::vector<int> valueColumns{1}; 
    double baseline{0.0};             
    bool continuous{false};           
};

struct TransitionStep {
    TransitionKind kind{TransitionKind::Morph};                
    double progress{1.0};                                      
    double stagger{0.0};                                       
    QEasingCurve easing;                                       
    double baseline{std::numeric_limits<double>::quiet_NaN()}; 
    OpacityTimeline opacity;                                   
    double duration{0.0};                                      
};

struct ItemTiming {
    double motion{1.0}; 
    double length{1.0}; 
    double spread{0.0}; 
    bool front{false};  
};

ItemTiming itemTiming(const TransitionStep& step, int itemCount);

class FrontStarts {
public:
    explicit FrontStarts(const QEasingCurve& easing);

    [[nodiscard]] double operator()(double share) const;

private:
    static constexpr auto kSamples = 1025;
    std::array<double, kSamples> starts_{};
};

class ItemClock {
public:
    ItemClock(const TransitionStep& step, int itemCount, bool exit);

    [[nodiscard]] const ItemTiming& timing() const noexcept;
    [[nodiscard]] double start(int order) const;
    [[nodiscard]] double motion(int order) const;
    [[nodiscard]] const FrontStarts* frontStarts() const noexcept;

private:
    double time_;
    int count_;
    bool exit_;
    ItemTiming timing_;
    double startStep_;
    std::optional<FrontStarts> starts_;
};

struct TransitionItems {
    const double* values{nullptr}; 
    int count{0};                  
};

struct TransitionSlots {
    bool enter{false};  
    bool update{false}; 
    bool exit{false};   

    [[nodiscard]] bool any() const noexcept
    {
        return enter || update || exit;
    }
};

bool keptItemsChanged(const TransitionItems& from, const TransitionItems& to, int stride);

TransitionSlots plannedSlots(int fromCount, int toCount, bool keptChanged, std::optional<TransitionKind> enter, std::optional<TransitionKind> update,
    std::optional<TransitionKind> exit);

struct TransitionFrameInput {
    TransitionItems from;                 
    TransitionItems to;                   
    TransitionLayout layout;              
    std::optional<TransitionStep> enter;  
    std::optional<TransitionStep> update; 
    std::optional<TransitionStep> exit;   
};

int writeTransitionFrame(const TransitionFrameInput& input, std::vector<double>& out, ItemFade& fade);

} // namespace QAccelPlot::Internal
```


