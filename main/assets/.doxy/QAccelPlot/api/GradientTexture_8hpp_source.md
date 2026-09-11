

# File GradientTexture.hpp

[**File List**](files.md) **>** [**materials**](dir_d8d9f4c155782c7a914c1dbe847fe73b.md) **>** [**GradientTexture.hpp**](GradientTexture_8hpp.md)

[Go to the documentation of this file](GradientTexture_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "effects/GradientColorTypes.hpp"

#include <QImage>
#include <QSGMaterialShader>

#include <vector>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)
QT_FORWARD_DECLARE_CLASS(QSGTexture)

namespace QAccelPlot {

class GradientTexture {
public:
    GradientTexture() = default;
    ~GradientTexture();

    GradientTexture(const GradientTexture&) = delete;
    GradientTexture& operator=(const GradientTexture&) = delete;

    void upload(QQuickWindow* window, const std::vector<GradientStopData>& stops);
    void commit(QSGMaterialShader::RenderState& state, int binding, QSGTexture** texture);
    qint64 comparisonKey() const;

private:
    bool matches(const std::vector<GradientStopData>& stops) const;

    QImage image_;
    QSGTexture* texture_{nullptr};
    std::vector<GradientStopData> stops_;
};

} // namespace QAccelPlot
```


