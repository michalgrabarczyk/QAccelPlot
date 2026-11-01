

# File PointMaterial.hpp

[**File List**](files.md) **>** [**materials**](dir_c94e933d4aa9b037e187c1fb93a79d8a.md) **>** [**PointMaterial.hpp**](PointMaterial_8hpp.md)

[Go to the documentation of this file](PointMaterial_8hpp.md)


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
#include "QAccelPlot/materials/GradientTexture.hpp"

namespace QAccelPlot {

class PointMaterial : public DataTextureMaterial {
public:
    PointMaterial();

    QSGMaterialType* type() const override;
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    float markerSize{4.0f};          
    float antialiasingEnabled{1.0f}; 
    float antialiasingFeather{1.0f}; 
    int shapeType{0};                
    float markerStrokeWidth{1.0f};   
    float markerFilled{1.0f};        
    float gradientDirection{0.0f};   
    float gradientValueMin{0.0f};    
    float gradientValueMax{1.0f};    
    GradientTexture gradientTexture; 

protected:
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
```


