

# File CandlestickMaterial.hpp

[**File List**](files.md) **>** [**materials**](dir_c94e933d4aa9b037e187c1fb93a79d8a.md) **>** [**CandlestickMaterial.hpp**](CandlestickMaterial_8hpp.md)

[Go to the documentation of this file](CandlestickMaterial_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/materials/DataTextureMaterial.hpp"

#include <QSGGeometry>

namespace QAccelPlot {

class CandlestickMaterial : public DataTextureMaterial {
public:
    static constexpr int kVerticesPerCandle{18};

    CandlestickMaterial();

    QSGMaterialType* type() const override;
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode renderMode) const override;

    static const QSGGeometry::AttributeSet& attributeSet();

    QColor downColor{Qt::red};          
    QColor hoverColor{Qt::transparent}; 
    QColor glowDownColor;               
    float hoveredIndex{-1.0f};          
    float lineWidth{1.0f};              
    float halfWidth{0.4f};              
    float minimumWidth{1.0f};           
    float ohlc{0.0f};                   
    float candleCount{0.0f};            
    float antialiasing{1.0f};           

protected:
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
```


