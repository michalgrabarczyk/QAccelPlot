

# File GradientLineMaterial.hpp

[**File List**](files.md) **>** [**materials**](dir_d8d9f4c155782c7a914c1dbe847fe73b.md) **>** [**GradientLineMaterial.hpp**](GradientLineMaterial_8hpp.md)

[Go to the documentation of this file](GradientLineMaterial_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "materials/GradientTexture.hpp"
#include "materials/LineMaterial.hpp"

namespace QAccelPlot {

class GradientLineMaterial : public LineMaterial {
public:
    QSGMaterialType* type() const override;
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    float gradientDirection{0.0f};
    float gradientValueMin{0.0f};
    float gradientValueMax{1.0f};
    GradientTexture gradientTexture;

protected:
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
```


