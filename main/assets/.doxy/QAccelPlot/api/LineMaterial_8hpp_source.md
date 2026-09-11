

# File LineMaterial.hpp

[**File List**](files.md) **>** [**materials**](dir_d8d9f4c155782c7a914c1dbe847fe73b.md) **>** [**LineMaterial.hpp**](LineMaterial_8hpp.md)

[Go to the documentation of this file](LineMaterial_8hpp.md)


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

class LineMaterial : public DataTextureMaterial {
public:
    LineMaterial();

    QSGMaterialType* type() const override;
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    float lineWidth{1.0f};           
    float pointCount{0.0f};          
    float antialiasingEnabled{1.0f}; 
    float antialiasingFeather{1.0f}; 

    // Dash line uniforms (std140 offsets 132–175 in the UBO).
    float dashPeriod{0.0f}; 
    float dashOffset{0.0f}; 
    int dashPatternSize{0}; 
    float dashPattern[8]{}; 

protected:
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
```


