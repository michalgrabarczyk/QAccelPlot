

# File ItemOpacityUniforms.hpp

[**File List**](files.md) **>** [**internal**](dir_3c3be61dbf90c69b9ad6cd32d23f3e24.md) **>** [**ItemOpacityUniforms.hpp**](ItemOpacityUniforms_8hpp.md)

[Go to the documentation of this file](ItemOpacityUniforms_8hpp.md)


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

namespace QAccelPlot::Internal {

struct ItemFadeGroupUniforms {
    float range[4];                  
    float timing[4];                 
    float curve[kFadeCurveSamples];  
    float starts[kFadeStartSamples]; 
};

static_assert(sizeof(ItemFadeGroupUniforms) == 224);

struct ItemOpacityUniforms {
    ItemFadeGroupUniforms groups[kFadeGroupCount]; 
    float trail[4];                                
    float trailEasing[kTrailEasingSamples];        
};

static_assert(sizeof(ItemOpacityUniforms) == 528);

ItemOpacityUniforms itemOpacityUniforms(const ItemFade& fade, const ItemTrail& trail);

} // namespace QAccelPlot::Internal
```


