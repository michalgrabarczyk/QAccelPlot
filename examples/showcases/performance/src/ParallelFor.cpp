//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ParallelFor.hpp"

#include <algorithm>
#include <cstddef>
#include <thread>
#include <vector>

namespace QAccelPlotExample {
namespace {

// Memory bandwidth, not arithmetic, limits the generators beyond a few threads.
constexpr auto kMaximumThreads = 8;

int chunkCount(const int count, const int minimumChunk)
{
    const auto hardwareThreads = static_cast<int>(std::max(1U, std::thread::hardware_concurrency()));
    return std::clamp(count / std::max(1, minimumChunk), 1, std::min(hardwareThreads, kMaximumThreads));
}

} // namespace

void parallelFor(const int count, const int minimumChunk, const std::function<void(int begin, int end)>& body)
{
    if (count <= 0) {
        return;
    }
    const auto chunks = chunkCount(count, minimumChunk);
    const auto chunkBegin = [count, chunks](const int chunk) { return static_cast<int>(static_cast<long long>(count) * chunk / chunks); };

    auto threads = std::vector<std::thread>{};
    threads.reserve(static_cast<std::size_t>(chunks - 1));
    for (auto chunk = 1; chunk < chunks; ++chunk) {
        threads.emplace_back(body, chunkBegin(chunk), chunkBegin(chunk + 1));
    }
    body(0, chunkBegin(1));
    for (auto& thread : threads) {
        thread.join();
    }
}

} // namespace QAccelPlotExample
