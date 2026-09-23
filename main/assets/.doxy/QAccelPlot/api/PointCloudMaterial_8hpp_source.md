

# File PointCloudMaterial.hpp

[**File List**](files.md) **>** [**materials**](dir_c94e933d4aa9b037e187c1fb93a79d8a.md) **>** [**PointCloudMaterial.hpp**](PointCloudMaterial_8hpp.md)

[Go to the documentation of this file](PointCloudMaterial_8hpp.md)


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

class PointCloudMaterial : public DataTextureMaterial {
public:
    PointCloudMaterial();

    QSGMaterialType* type() const override;
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    static const QSGGeometry::AttributeSet& attributeSet();

    float markerSize{3.0f};          
    float antialiasingEnabled{1.0f}; 
    float antialiasingFeather{1.0f}; 
    float valueMin{0.0f};            
    float valueMax{1.0f};            
    float stride{2.0f};              
    int shapeType{0};                
    float markerStrokeWidth{1.0f};   
    float markerFilled{1.0f};        
    float valueLogScale{0.0f};       
    GradientTexture colorMap;        

protected:
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
```


