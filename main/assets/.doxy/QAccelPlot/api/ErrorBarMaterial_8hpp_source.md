

# File ErrorBarMaterial.hpp

[**File List**](files.md) **>** [**materials**](dir_c94e933d4aa9b037e187c1fb93a79d8a.md) **>** [**ErrorBarMaterial.hpp**](ErrorBarMaterial_8hpp.md)

[Go to the documentation of this file](ErrorBarMaterial_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/materials/DataTextureMaterial.hpp"

#include <QSGGeometry>

namespace QAccelPlot {

class ErrorBarMaterial : public DataTextureMaterial {
public:
    static constexpr int kVerticesPerErrorBar{18};

    ErrorBarMaterial();

    QSGMaterialType* type() const override;
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    static const QSGGeometry::AttributeSet& attributeSet();

    QColor hoverColor{Qt::transparent}; 
    float hoveredIndex{-1.0f};          
    float lineWidth{1.5f};              
    float capLength{8.0f};              
    float offset{0.0f};                 
    float horizontal{0.0f};             
    float errorBarCount{0.0f};          

protected:
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
```


