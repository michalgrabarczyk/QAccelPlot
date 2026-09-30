

# File DataTexture.hpp

[**File List**](files.md) **>** [**materials**](dir_c94e933d4aa9b037e187c1fb93a79d8a.md) **>** [**DataTexture.hpp**](DataTexture_8hpp.md)

[Go to the documentation of this file](DataTexture_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QImage>
#include <QSGTexture>

#include <memory>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot {

class DataTexture {
public:
    DataTexture() = default;
    explicit DataTexture(std::unique_ptr<QSGTexture> texture);

    void upload(QQuickWindow* window, const float* data, int floatCount);

    QSGTexture* texture() const;

private:
    std::unique_ptr<QSGTexture> texture_;
    QImage imageBuffer_;
    bool warnedAboutTextureSize_{false};
};

} // namespace QAccelPlot
```


