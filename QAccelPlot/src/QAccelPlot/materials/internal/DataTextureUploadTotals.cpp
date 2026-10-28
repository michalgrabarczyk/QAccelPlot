//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/internal/DataTextureUploadTotals.hpp"

#include <atomic>

namespace QAccelPlot::Internal {

namespace {

// Uploads run on the render thread, which can differ from the thread that reads the totals.
std::atomic<std::int64_t> uploadCount{0};
std::atomic<std::int64_t> uploadBytes{0};

} // namespace

void recordDataTextureUpload(const std::int64_t bytes)
{
    uploadCount.fetch_add(1, std::memory_order_relaxed);
    uploadBytes.fetch_add(bytes, std::memory_order_relaxed);
}

DataTextureUploadTotals dataTextureUploadTotals()
{
    return {uploadCount.load(std::memory_order_relaxed), uploadBytes.load(std::memory_order_relaxed)};
}

void resetDataTextureUploadTotals()
{
    uploadCount.store(0, std::memory_order_relaxed);
    uploadBytes.store(0, std::memory_order_relaxed);
}

} // namespace QAccelPlot::Internal
