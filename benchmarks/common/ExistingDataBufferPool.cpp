//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExistingDataBufferPool.hpp"

#include "renderers/LineCurveLineRenderer.hpp"

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

ExistingDataBufferPool::ExistingDataBufferPool(const int pointCount, const int curveCount, const int bufferCount)
    : pointCount_(std::max(0, pointCount))
    , curveCount_(std::max(1, curveCount))
    , bufferCount_(std::clamp(bufferCount, 1, kMaxBufferCount))
{
    generate();
}

int ExistingDataBufferPool::pointCount() const
{
    return pointCount_;
}

int ExistingDataBufferPool::curveCount() const
{
    return curveCount_;
}

int ExistingDataBufferPool::bufferCount() const
{
    return bufferCount_;
}

std::size_t ExistingDataBufferPool::sourceBytes() const
{
    return sourceBytes_;
}

const float* ExistingDataBufferPool::curveBufferData(const int curveIndex, const int bufferIndex) const
{
    if (buffers_.empty()) {
        return nullptr;
    }
    const auto normalizedCurve = ((curveIndex % curveCount_) + curveCount_) % curveCount_;
    const auto normalizedBuffer = ((bufferIndex % bufferCount_) + bufferCount_) % bufferCount_;
    return buffers_[static_cast<std::size_t>(normalizedCurve)][static_cast<std::size_t>(normalizedBuffer)].data();
}

const std::vector<char>& ExistingDataBufferPool::vertexCache() const
{
    return vertexCache_;
}

void ExistingDataBufferPool::discardVertexCache()
{
    vertexCache_.clear();
    vertexCache_.shrink_to_fit();
}

void ExistingDataBufferPool::generate()
{
    buffers_.clear();
    vertexCache_.clear();
    sourceBytes_ = 0;
    if (pointCount_ == 0) {
        return;
    }

    buffers_.resize(static_cast<std::size_t>(curveCount_));
    constexpr auto kTwoPi = 6.28318530718f;
    constexpr auto kBufferPhaseStep = 0.05f;
    const auto curvePhaseStep = curveCount_ > 1 ? kTwoPi / static_cast<float>(curveCount_) : 0.0f;

    for (auto curveIndex = 0; curveIndex < curveCount_; ++curveIndex) {
        auto& curveBuffers = buffers_[static_cast<std::size_t>(curveIndex)];
        curveBuffers.resize(static_cast<std::size_t>(bufferCount_));
        const auto basePhase = static_cast<float>(curveIndex) * curvePhaseStep;
        for (auto bufferIndex = 0; bufferIndex < bufferCount_; ++bufferIndex) {
            auto& buffer = curveBuffers[static_cast<std::size_t>(bufferIndex)];
            buffer.resize(static_cast<std::size_t>(pointCount_) * 2);
            const auto phase = basePhase + static_cast<float>(bufferIndex) * kBufferPhaseStep;
            for (auto pointIndex = 0; pointIndex < pointCount_; ++pointIndex) {
                const auto offset = static_cast<std::size_t>(pointIndex) * 2;
                const auto x = static_cast<float>(pointIndex);
                buffer[offset] = x;
                buffer[offset + 1] = std::sin(x * 0.01f + phase);
            }
            sourceBytes_ += buffer.size() * sizeof(float);
        }
    }

    LineCurveLineRenderer{}.buildVertexCache(buffers_.front().front(), pointCount_, vertexCache_);
}

} // namespace QAccelPlot
