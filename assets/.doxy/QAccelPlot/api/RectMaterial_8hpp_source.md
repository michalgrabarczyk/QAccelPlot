

# File RectMaterial.hpp

[**File List**](files.md) **>** [**materials**](dir_d8d9f4c155782c7a914c1dbe847fe73b.md) **>** [**RectMaterial.hpp**](RectMaterial_8hpp.md)

[Go to the documentation of this file](RectMaterial_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "materials/DataTextureMaterial.hpp"

namespace QAccelPlot {

class RectMaterial : public DataTextureMaterial {
public:
    RectMaterial();

    QSGMaterialType* type() const override;
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    float rectCount{0.0f}; 

protected:
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
```


