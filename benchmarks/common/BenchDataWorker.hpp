//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

/// \file BenchDataWorker.hpp
/// \brief Background thread that pre-generates data and builds the vertex cache
///        so that the render thread only calls setDataFNoRangeWithCache().

#include "renderers/LineCurveLineRenderer.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace QAccelPlot {

/// \brief Produces interleaved (x, y) float data with a pre-built vertex cache
///        on a dedicated background thread.
///
/// The render thread calls tryConsume() each frame. If a fresh batch is ready
/// the vectors are moved out (zero-copy hand-off); otherwise the previous frame
/// data is reused, which still produces a valid render.
struct BenchBatch {
    std::vector<float> data;
    std::vector<char>  vertexCache;
    int pointCount{0};
};

class BenchDataWorker {
public:
    /// \param pointCount  Number of (x, y) points per curve.
    /// \param curveCount  Number of independent phase offsets to produce.
    explicit BenchDataWorker(int pointCount, int curveCount = 1);
    ~BenchDataWorker();

    BenchDataWorker(const BenchDataWorker&) = delete;
    BenchDataWorker& operator=(const BenchDataWorker&) = delete;

    void start();
    void stop();

    /// \brief Try to take the latest ready batch.
    /// \returns true and moves data into \p out when a new batch is available.
    ///          Returns false when no new batch has been produced since the last call.
    bool tryConsume(BenchBatch& out);

private:
    void run();

    int pointCount_;
    int curveCount_;

    std::mutex              mutex_;
    std::condition_variable cv_;
    std::thread             thread_;

    BenchBatch              readyBatch_;
    bool                    dataReady_{false};

    std::atomic<bool>       running_{false};
    std::atomic<float>      phase_{0.0f};

    LineCurveLineRenderer   renderer_; // stateless — used only for buildVertexCache()
};

} // namespace QAccelPlot
