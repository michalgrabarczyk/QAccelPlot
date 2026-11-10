

# File OpacityTimeline.hpp

[**File List**](files.md) **>** [**internal**](dir_698dae9af7fc703eaceb77bfe4886cff.md) **>** [**OpacityTimeline.hpp**](OpacityTimeline_8hpp.md)

[Go to the documentation of this file](OpacityTimeline_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QEasingCurve>

#include <optional>
#include <vector>

namespace QAccelPlot::Internal {

struct OpacitySegment {
    double start{0.0};          
    double duration{0.0};       
    std::optional<double> from; 
    double to{1.0};             
    QEasingCurve easing;        
};

class OpacityTimeline {
public:
    OpacityTimeline() = default;
    OpacityTimeline(std::vector<OpacitySegment> segments, double length);

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] double length() const noexcept;
    [[nodiscard]] double valueAt(double time) const;

private:
    struct Resolved {
        double start;
        double duration;
        double from;
        double to;
        QEasingCurve easing;
    };

    [[nodiscard]] double valueAt(double time, std::size_t segmentCount) const;

    std::vector<Resolved> segments_;
    double length_{0.0};
};

} // namespace QAccelPlot::Internal
```


