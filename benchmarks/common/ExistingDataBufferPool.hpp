//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <cstddef>
#include <vector>

namespace QAccelPlot {

/// A bounded set of existing interleaved-float buffers used as benchmark input.
///
/// Buffer generation happens before timing begins. Live benchmarks alternate two
/// buffers, modelling the double-buffered handoff commonly used by applications
/// that already receive or calculate their data elsewhere. Static benchmarks use
/// one buffer. This deliberately measures ingestion and rendering, not synthetic
/// signal generation.
class ExistingDataBufferPool {
public:
    static constexpr int kMaxBufferCount = 3;

    explicit ExistingDataBufferPool(int pointCount = 1'000'000, int curveCount = 1, int bufferCount = 2);

    int pointCount() const;
    int curveCount() const;
    int bufferCount() const;
    std::size_t sourceBytes() const;

    const float* curveBufferData(int curveIndex, int bufferIndex) const;

    const std::vector<char>& vertexCache() const;

    void discardVertexCache();

private:
    void generate();

    int pointCount_{0};
    int curveCount_{1};
    int bufferCount_{2};
    std::size_t sourceBytes_{0};
    std::vector<std::vector<std::vector<float>>> buffers_;
    std::vector<char> vertexCache_;
};

} // namespace QAccelPlot
