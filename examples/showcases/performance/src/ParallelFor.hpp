//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <functional>

namespace QAccelPlotExample {

/// \brief Records per chunk below which \c parallelFor() keeps a dataset on one thread.
inline constexpr auto kParallelChunk = 1 << 16;

/// \brief Calls \a body for consecutive ranges <tt>[begin, end)</tt> that together cover <tt>[0, count)</tt>.
///
/// Splits the range into chunks of at least \a minimumChunk items, at most one per hardware
/// thread and no more than 8, and runs them in parallel; the calling thread runs the first one.
/// Returns after every chunk has finished. \a body must be safe to call concurrently for
/// disjoint ranges.
void parallelFor(int count, int minimumChunk, const std::function<void(int begin, int end)>& body);

} // namespace QAccelPlotExample
