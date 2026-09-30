

# File BandMaterial.hpp

[**File List**](files.md) **>** [**materials**](dir_c94e933d4aa9b037e187c1fb93a79d8a.md) **>** [**BandMaterial.hpp**](BandMaterial_8hpp.md)

[Go to the documentation of this file](BandMaterial_8hpp.md)


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

class BandMaterial : public DataTextureMaterial {
public:
    struct Vertex {
        float id;   
        float edge; 
    };

    BandMaterial();

    QSGMaterialType* type() const override;
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    static const QSGGeometry::AttributeSet& vertexAttributes();

    float sampleCount{0.0f}; 

protected:
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
```


