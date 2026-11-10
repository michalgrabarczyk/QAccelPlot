//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

namespace QAccelPlotExample {

/// \brief A short loop of precomputed batches, played forward and then backward.
///
/// With the default interval the frames are one nominal display frame apart, so playback moves
/// at the generator's own speed and the turn at either end hides the loop's seam. \a Generator
/// is as for \c GenerationWorker, its \c Batch is copyable and has
/// <tt>std::size_t byteSize() const</tt>, and its \c Parameters compare with \c ==.
template <typename Generator> class ReplayLoop final {
public:
    using Batch = typename Generator::Batch;
    using Parameters = typename Generator::Parameters;

    /// \brief Memory the frames of one loop may take together, in bytes.
    static constexpr auto kMemoryBudgetBytes = std::size_t{1} << 30;
    /// \brief Most frames in one loop; a moving loop always gets at least two.
    static constexpr auto kMaximumFrameCount = 60;
    /// \brief Default animation time between two frames, in seconds.
    static constexpr auto kFrameIntervalSeconds = 1.0 / 60.0;

    /// \brief Returns how many frames of \a frameBytes bytes each fit the memory budget.
    static int frameCountFor(const std::size_t frameBytes)
    {
        const auto fitting = kMemoryBudgetBytes / std::max<std::size_t>(1, frameBytes);
        return static_cast<int>(std::clamp<std::size_t>(fitting, 2, kMaximumFrameCount));
    }

    /// \brief Returns \c true when the loop holds frames built for \a parameters.
    bool holds(const Parameters& parameters) const
    {
        return !frames_.empty() && parameters_ == parameters;
    }

    /// \brief Returns the parameters of the last complete build.
    const Parameters& parameters() const
    {
        return parameters_;
    }

    /// \brief Returns the number of frames in the loop.
    int frameCount() const
    {
        return static_cast<int>(frames_.size());
    }

    /// \brief Builds the frames for \a parameters with \a generator, \a intervalSeconds apart from \a startSeconds.
    ///
    /// An interval of 0 builds one still frame. Gives up and empties the loop as soon as
    /// \a stopRequested returns \c true between two frames; returns whether the loop is complete.
    template <typename StopRequested>
    bool build(Generator& generator, const Parameters& parameters, const double startSeconds, const double intervalSeconds, StopRequested&& stopRequested)
    {
        clear();
        frames_.emplace_back();
        generator.generate(frames_.front(), parameters, startSeconds);
        const auto count = intervalSeconds > 0.0 ? frameCountFor(frames_.front().byteSize()) : 1;
        frames_.reserve(static_cast<std::size_t>(count));
        for (auto frame = 1; frame < count; ++frame) {
            if (stopRequested()) {
                clear();
                return false;
            }
            frames_.emplace_back();
            generator.generate(frames_.back(), parameters, startSeconds + frame * intervalSeconds);
        }
        parameters_ = parameters;
        return true;
    }

    /// \brief Copies the current frame into \a batch, reusing its storage, and steps to the next one.
    void next(Batch& batch)
    {
        batch = frames_[static_cast<std::size_t>(index_)];
        const auto count = frameCount();
        if (count < 2) {
            return;
        }
        if (index_ + direction_ < 0 || index_ + direction_ >= count) {
            direction_ = -direction_;
        }
        index_ += direction_;
    }

    /// \brief Frees the frames.
    void clear()
    {
        frames_.clear();
        frames_.shrink_to_fit();
        index_ = 0;
        direction_ = 1;
    }

private:
    std::vector<Batch> frames_;
    Parameters parameters_{};
    int index_{0};
    int direction_{1};
};

} // namespace QAccelPlotExample
