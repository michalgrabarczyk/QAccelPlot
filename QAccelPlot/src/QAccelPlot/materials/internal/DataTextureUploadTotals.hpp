//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <cstdint>

namespace QAccelPlot::Internal {

/// \brief How many data textures were uploaded since the last reset, and how many bytes they held.
///
/// Tests read the totals to check how much data a frame uploads.
struct DataTextureUploadTotals {
    std::int64_t count{0}; ///< Number of uploads.
    std::int64_t bytes{0}; ///< Bytes of series data uploaded, without the padding of the last texture row.
};

/// \brief Adds one upload of \a bytes to the totals. Thread-safe.
void recordDataTextureUpload(std::int64_t bytes);

/// \brief Returns the totals since the last reset. Thread-safe.
DataTextureUploadTotals dataTextureUploadTotals();

/// \brief Sets the totals back to zero. Thread-safe.
void resetDataTextureUploadTotals();

} // namespace QAccelPlot::Internal
