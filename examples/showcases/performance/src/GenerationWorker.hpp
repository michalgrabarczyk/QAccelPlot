//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "ReplayLoop.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>

namespace QAccelPlotExample {

/// \brief Replays generated datasets from a background thread, keeping at most one completed batch pending.
///
/// The worker generates a \c ReplayLoop whenever the parameters change and copies one of its
/// frames into each batch. While a batch is pending, it copies the next one and then waits until
/// the pending batch is consumed, so it never prepares more than one dataset ahead of the display.
/// \a Generator provides default-constructible \c Batch and \c Parameters types,
/// <tt>void generate(Batch&, const Parameters&, double timeSeconds)</tt>, and what \c ReplayLoop
/// requires. \c start(), \c requestStop(), and \c stop() must be called from one thread.
template <typename Generator> class GenerationWorker final {
public:
    using Batch = typename Generator::Batch;
    using Parameters = typename Generator::Parameters;
    /// \brief Receives the time the worker spent copying one batch, on the worker thread.
    using CopyCallback = std::function<void(std::chrono::nanoseconds)>;

    /// \brief Constructs a stopped worker with default-constructed parameters.
    GenerationWorker() = default;

    ~GenerationWorker()
    {
        stop();
    }

    GenerationWorker(const GenerationWorker&) = delete;
    GenerationWorker& operator=(const GenerationWorker&) = delete;

    /// \brief Starts the background thread; does nothing when it is already running.
    ///
    /// Joins a thread that is still finishing after \c requestStop() first.
    void start()
    {
        if (isRunning()) {
            return;
        }
        if (workerThread_.joinable()) {
            workerThread_.join();
        }

        running_.store(true, std::memory_order_release);
        workerThread_ = std::thread([this]() { run(); });
    }

    /// \brief Asks the background thread to stop after its current batch, without waiting for it.
    void requestStop()
    {
        {
            // The worker checks running_ while holding the mutex, so clearing it under the mutex
            // keeps the notification from falling between that check and the worker's wait.
            const auto lock = std::lock_guard<std::mutex>{mutex_};
            running_.store(false, std::memory_order_release);
        }

        conditionVariable_.notify_all();
    }

    /// \brief Stops and joins the background thread. A batch that is still pending stays consumable.
    void stop()
    {
        requestStop();

        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }

    /// \brief Returns \c true while the background thread is running.
    bool isRunning() const
    {
        return running_.load(std::memory_order_acquire);
    }

    /// \brief Sets the animation time, in seconds, at which the next replay loop starts.
    void setTime(const double seconds)
    {
        timeSeconds_.store(seconds, std::memory_order_relaxed);
    }

    /// \brief Sets the generator parameters used for subsequent batches.
    void setParameters(const Parameters& parameters)
    {
        const auto lock = std::lock_guard<std::mutex>{mutex_};
        parameters_ = parameters;
    }

    /// \brief Sets the animation time between replayed frames. Call it before \c start().
    ///
    /// The default is \c ReplayLoop::kFrameIntervalSeconds; 0 replays one still frame.
    void setFrameInterval(const double seconds)
    {
        frameIntervalSeconds_ = seconds;
    }

    /// \brief Sets the function told how long each batch took to copy. Call it before \c start().
    void setCopyCallback(CopyCallback callback)
    {
        copyCallback_ = std::move(callback);
    }

    /// \brief Waits until a completed batch is pending or \a timeout elapses.
    bool waitForData(const std::chrono::milliseconds timeout)
    {
        auto lock = std::unique_lock<std::mutex>{mutex_};
        conditionVariable_.wait_for(lock, timeout, [this]() { return dataReady_ || !running_.load(std::memory_order_relaxed); });
        return dataReady_;
    }

    /// \brief Swaps the pending batch into \a batch and lets the worker publish the next one.
    ///
    /// The worker reuses the storage that \a batch held for a later batch, so a batch whose
    /// records were copied rather than moved out is not allocated again.
    bool tryConsume(Batch& batch)
    {
        auto builtFor = Parameters{};
        return tryConsume(batch, builtFor);
    }

    /// \brief Like \c tryConsume(Batch&), and also sets \a builtFor to the parameters the batch was generated for.
    ///
    /// The batch can stem from earlier parameters than the current ones, because the worker
    /// copies one batch ahead.
    bool tryConsume(Batch& batch, Parameters& builtFor)
    {
        const auto lock = std::lock_guard<std::mutex>{mutex_};
        if (!dataReady_) {
            return false;
        }

        std::swap(batch, readyBatch_);
        builtFor = readyParameters_;
        dataReady_ = false;
        conditionVariable_.notify_one();
        return true;
    }

private:
    void run()
    {
        auto workBatch = Batch{};
        auto loop = ReplayLoop<Generator>{};
        while (running_.load(std::memory_order_relaxed)) {
            if (!prepare(workBatch, loop) || !publish(workBatch, loop.parameters())) {
                return;
            }
        }
    }

    // Copies the next frame of the replay loop into batch, first building the loop for the current
    // parameters if needed. A build is abandoned as soon as the parameters change again. Returns
    // false when a stop interrupted the build.
    bool prepare(Batch& batch, ReplayLoop<Generator>& loop)
    {
        auto parameters = this->parameters();
        while (!loop.holds(parameters)) {
            const auto superseded = [this, &parameters]() { return !isRunning() || !(this->parameters() == parameters); };
            loop.build(generator_, parameters, timeSeconds_.load(std::memory_order_relaxed), frameIntervalSeconds_, superseded);
            if (!isRunning()) {
                return false;
            }
            parameters = this->parameters();
        }

        const auto start = std::chrono::steady_clock::now();
        loop.next(batch);
        if (copyCallback_) {
            copyCallback_(std::chrono::steady_clock::now() - start);
        }
        return true;
    }

    // Waits until no batch is pending, then makes batch, generated for parameters, the pending one
    // and takes back the storage of the batch consumed last. Returns false when stopped first.
    bool publish(Batch& batch, const Parameters& parameters)
    {
        {
            auto lock = std::unique_lock<std::mutex>{mutex_};
            conditionVariable_.wait(lock, [this]() { return !dataReady_ || !running_.load(std::memory_order_relaxed); });
            if (!running_.load(std::memory_order_relaxed)) {
                return false;
            }
            std::swap(readyBatch_, batch);
            readyParameters_ = parameters;
            dataReady_ = true;
        }
        conditionVariable_.notify_all();
        return true;
    }

    Parameters parameters()
    {
        const auto lock = std::lock_guard<std::mutex>{mutex_};
        return parameters_;
    }

    Generator generator_;
    std::mutex mutex_;
    std::condition_variable conditionVariable_;
    std::thread workerThread_;
    Batch readyBatch_;
    Parameters readyParameters_;
    bool dataReady_{false};
    std::atomic<bool> running_{false};
    std::atomic<double> timeSeconds_{0.0};
    Parameters parameters_;
    double frameIntervalSeconds_{ReplayLoop<Generator>::kFrameIntervalSeconds};
    CopyCallback copyCallback_;
};

} // namespace QAccelPlotExample
