

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

namespace QAccelPlot::Internal {

constexpr int kDataTextureWidth{8192};

constexpr int dataTextureHeight(const int floatCount)
{
    return (floatCount + kDataTextureWidth - 1) / kDataTextureWidth;
}

constexpr qint64 dataTextureItemCapacity(const int maxHeight, const int floatsPerItem)
{
    return static_cast<qint64>(kDataTextureWidth) * maxHeight / floatsPerItem;
}

} // namespace QAccelPlot::Internal
```


