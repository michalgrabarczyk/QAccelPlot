

# File HeatmapMaterial.hpp

[**File List**](files.md) **>** [**materials**](dir_c94e933d4aa9b037e187c1fb93a79d8a.md) **>** [**HeatmapMaterial.hpp**](HeatmapMaterial_8hpp.md)

[Go to the documentation of this file](HeatmapMaterial_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/materials/DataTexture.hpp"
#include "QAccelPlot/materials/GradientTexture.hpp"

#include <QSGMaterial>
#include <QVector2D>

namespace QAccelPlot {

class HeatmapMaterial : public QSGMaterial {
public:
    HeatmapMaterial();

    QSGMaterialType* type() const override;
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;
    int compare(const QSGMaterial* other) const override;

    QVector2D cellScale{1.0f, 1.0f};  
    QVector2D cellOffset{0.0f, 0.0f}; 
    QVector2D cellRate{0.0f, 0.0f};   
    QVector2D cellShift{0.0f, 0.0f};  
    float valueMin{0.0f};             
    float valueMax{1.0f};             
    float valueLogScale{0.0f};        
    int columns{0};                   
    int rows{0};                      
    int columnMapping{0};             
    int rowMapping{0};                
    DataTexture values;               
    GradientTexture colorMap;         
};

} // namespace QAccelPlot
```


