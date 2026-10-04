//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <utility>

namespace QAccelPlotExample {

/// \brief Runs a dataset generator on a background thread, keeping at most one completed batch pending.
///
/// The worker waits until the pending batch is consumed before generating another, so it never
/// builds datasets the display cannot show. \a Generator provides default-constructible
/// \c Batch and \c Parameters types and
/// <tt>void generate(Batch&, const Parameters&, double timeSeconds)</tt>.
/// \c start(), \c requestStop(), and \c stop() must be called from one thread.
template <typename Generator> class GenerationWorker final {
public:
    using Batch = typename Generator::Batch;
    using Parameters = typename Generator::Parameters;

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

    /// \brief Sets the animation time, in seconds, used for subsequent batches.
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

    /// \brief Waits until a completed batch is pending or \a timeout elapses.
    bool waitForData(const std::chrono::milliseconds timeout)
    {
        auto lock = std::unique_lock<std::mutex>{mutex_};
        conditionVariable_.wait_for(lock, timeout, [this]() { return dataReady_ || !running_.load(std::memory_order_relaxed); });
        return dataReady_;
    }

    /// \brief Moves the pending batch into \a batch and lets the worker generate the next one.
    bool tryConsume(Batch& batch)
    {
        const auto lock = std::lock_guard<std::mutex>{mutex_};
        if (!dataReady_) {
            return false;
        }

        batch = std::move(readyBatch_);
        readyBatch_ = Batch{};
        dataReady_ = false;
        conditionVariable_.notify_one();
        return true;
    }

private:
    void run()
    {
        auto workBatch = Batch{};
        while (running_.load(std::memory_order_relaxed)) {
            generator_.generate(workBatch, parameters(), timeSeconds_.load(std::memory_order_relaxed));

            {
                const auto lock = std::lock_guard<std::mutex>{mutex_};
                std::swap(readyBatch_, workBatch);
                dataReady_ = true;
            }
            conditionVariable_.notify_all();

            auto lock = std::unique_lock<std::mutex>{mutex_};
            conditionVariable_.wait(lock, [this]() { return !dataReady_ || !running_.load(std::memory_order_relaxed); });
        }
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
    bool dataReady_{false};
    std::atomic<bool> running_{false};
    std::atomic<double> timeSeconds_{0.0};
    Parameters parameters_;
};

} // namespace QAccelPlotExample
