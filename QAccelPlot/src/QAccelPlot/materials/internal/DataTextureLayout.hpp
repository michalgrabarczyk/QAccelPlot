//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QtCore/qglobal.h>

#include <algorithm>

namespace QAccelPlot::Internal {

/// \brief Largest number of floats stored per data texture row, one float per RGBA8 texel.
constexpr int kMaxDataTextureWidth{8192};

/// \brief Returns the floats per data texture row on a GPU with texture limit \a maxTextureSize.
///
/// data_texture.glsl reads the width from the bound texture, so it follows this value.
constexpr int dataTextureWidth(const int maxTextureSize)
{
    return std::min(kMaxDataTextureWidth, maxTextureSize);
}

/// \brief Returns the number of rows of \a width floats needed to store \a floatCount floats.
constexpr int dataTextureHeight(const int floatCount, const int width)
{
    return (floatCount + width - 1) / width;
}

/// \brief Returns how many items of \a floatsPerItem floats fit in a data texture on a GPU with texture limit \a maxTextureSize.
constexpr qint64 dataTextureItemCapacity(const int maxTextureSize, const int floatsPerItem)
{
    return static_cast<qint64>(dataTextureWidth(maxTextureSize)) * maxTextureSize / floatsPerItem;
}

} // namespace QAccelPlot::Internal
