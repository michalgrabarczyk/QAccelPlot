

# File PointMaterial.hpp

[**File List**](files.md) **>** [**materials**](dir_d8d9f4c155782c7a914c1dbe847fe73b.md) **>** [**PointMaterial.hpp**](PointMaterial_8hpp.md)

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

#include <QColor>
#include <QSGMaterial>
#include <QSGMaterialShader>
#include <QVector2D>

namespace QAccelPlot {

class PointMaterial : public QSGMaterial {
public:
    PointMaterial();
    ~PointMaterial() override = default;

    QSGMaterialType* type() const override;
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;
    int compare(const QSGMaterial* other) const override;

    QColor color{Qt::blue};                 
    QVector2D domainMin{0.0f, 0.0f};        
    QVector2D domainMax{1.0f, 1.0f};        
    QVector2D viewportSize{800.0f, 600.0f}; 
    float markerSize{4.0f};                 
    float logScaleX{0.0f};                  
    float logScaleY{0.0f};                  
    float useVertexColor{0.0f};             
    float antialiasingEnabled{1.0f};        
    float antialiasingFeather{1.0f};        
    int shapeType{0};                       
};

} // namespace QAccelPlot
```


