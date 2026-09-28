//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QtCore/qglobal.h>

namespace QAccelPlot::Internal {

/// \brief Number of floats stored per data texture row, one float per RGBA8 texel.
///
/// data_texture.glsl reads the width from the bound texture, so this is the only definition.
constexpr int kDataTextureWidth{8192};

/// \brief Returns the number of data texture rows needed to store \a floatCount floats.
constexpr int dataTextureHeight(const int floatCount)
{
    return (floatCount + kDataTextureWidth - 1) / kDataTextureWidth;
}

/// \brief Returns how many items of \a floatsPerItem floats fit in a data texture at most \a maxHeight rows tall.
constexpr qint64 dataTextureItemCapacity(const int maxHeight, const int floatsPerItem)
{
    return static_cast<qint64>(kDataTextureWidth) * maxHeight / floatsPerItem;
}

} // namespace QAccelPlot::Internal
