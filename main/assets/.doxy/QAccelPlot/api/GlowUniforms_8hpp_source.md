

# File GlowUniforms.hpp

[**File List**](files.md) **>** [**internal**](dir_3c3be61dbf90c69b9ad6cd32d23f3e24.md) **>** [**GlowUniforms.hpp**](GlowUniforms_8hpp.md)

[Go to the documentation of this file](GlowUniforms_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/internal/GlowStyle.hpp"

namespace QAccelPlot::Internal {

struct GlowUniforms {
    float color[4];  
    float params[4]; 
};

static_assert(sizeof(GlowUniforms) == 32);

GlowUniforms glowUniforms(const GlowStyle& style);

} // namespace QAccelPlot::Internal
```


