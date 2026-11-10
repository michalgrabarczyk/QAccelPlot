

# File GlowStyle.hpp

[**File List**](files.md) **>** [**internal**](dir_78f1acf6f1aae410b1e370fefb18a5f4.md) **>** [**GlowStyle.hpp**](GlowStyle_8hpp.md)

[Go to the documentation of this file](GlowStyle_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>

namespace QAccelPlot::Internal {

struct GlowStyle {
    QColor color;          
    float radius{0.0f};    
    float intensity{0.0f}; 

    [[nodiscard]] bool active() const noexcept
    {
        return radius > 0.0f && intensity > 0.0f;
    }
};

[[nodiscard]] int compareGlowStyles(const GlowStyle& left, const GlowStyle& right) noexcept;

} // namespace QAccelPlot::Internal
```


