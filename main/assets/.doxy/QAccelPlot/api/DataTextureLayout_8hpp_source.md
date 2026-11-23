

# File DataTextureLayout.hpp

[**File List**](files.md) **>** [**internal**](dir_3c3be61dbf90c69b9ad6cd32d23f3e24.md) **>** [**DataTextureLayout.hpp**](DataTextureLayout_8hpp.md)

[Go to the documentation of this file](DataTextureLayout_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QtCore/qglobal.h>

#include <algorithm>

namespace QAccelPlot::Internal {

constexpr int kMaxDataTextureWidth{8192};

constexpr int dataTextureWidth(const int maxTextureSize)
{
    return std::min(kMaxDataTextureWidth, maxTextureSize);
}

constexpr int dataTextureHeight(const int floatCount, const int width)
{
    return (floatCount + width - 1) / width;
}

constexpr qint64 dataTextureItemCapacity(const int maxTextureSize, const int floatsPerItem)
{
    return static_cast<qint64>(dataTextureWidth(maxTextureSize)) * maxTextureSize / floatsPerItem;
}

} // namespace QAccelPlot::Internal
```


