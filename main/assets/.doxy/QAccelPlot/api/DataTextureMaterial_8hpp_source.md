

# File DataTextureMaterial.hpp

[**File List**](files.md) **>** [**materials**](dir_d8d9f4c155782c7a914c1dbe847fe73b.md) **>** [**DataTextureMaterial.hpp**](DataTextureMaterial_8hpp.md)

[Go to the documentation of this file](DataTextureMaterial_8hpp.md)


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
#include <QImage>
#include <QMatrix4x4>
#include <QQuickWindow>
#include <QSGMaterial>
#include <QSGTexture>
#include <QVector2D>

#include <memory>

namespace QAccelPlot {

class DataTextureMaterial : public QSGMaterial {
public:
    DataTextureMaterial();
    ~DataTextureMaterial() override = default;

    int compare(const QSGMaterial* other) const override;

    static bool writeCommonUniforms(char* buf, int bufSize, const QMatrix4x4& matrix, const DataTextureMaterial* mat);

    void uploadTexture(std::unique_ptr<QSGTexture>& texture, QQuickWindow* window, const float* data, int floatCount);

    static void commitTexture(QSGMaterialShader::RenderState& state, int binding, QSGTexture** texture, QSGTexture* dataTexture);

    static const QSGGeometry::AttributeSet& attributeSet();

protected:
    virtual int compareExtra(const QSGMaterial* other) const;

public:
    QColor color{Qt::blue};                  
    QVector2D domainMin{0.0f, 0.0f};         
    QVector2D domainMax{1.0f, 1.0f};         
    QVector2D viewportSize{800.0f, 600.0f};  
    float logScaleX{0.0f};                   
    float logScaleY{0.0f};                   
    float useVertexColor{0.0f};              
    std::unique_ptr<QSGTexture> dataTexture; 

private:
    QImage imageBuffer_;
};

} // namespace QAccelPlot
```


