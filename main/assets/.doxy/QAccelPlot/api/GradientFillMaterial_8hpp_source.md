

# File GradientFillMaterial.hpp

[**File List**](files.md) **>** [**materials**](dir_c94e933d4aa9b037e187c1fb93a79d8a.md) **>** [**GradientFillMaterial.hpp**](GradientFillMaterial_8hpp.md)

[Go to the documentation of this file](GradientFillMaterial_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/materials/GradientTexture.hpp"

#include <QSGMaterial>

namespace QAccelPlot {

class GradientFillMaterial : public QSGMaterial {
public:
    GradientFillMaterial();

    QSGMaterialType* type() const override;
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;
    int compare(const QSGMaterial* other) const override;

    float opacity{1.0f};
    GradientTexture gradientTexture;
};

} // namespace QAccelPlot
```


