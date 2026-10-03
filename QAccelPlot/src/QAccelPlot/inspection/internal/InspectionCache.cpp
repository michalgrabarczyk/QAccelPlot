//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/inspection/internal/InspectionCache.hpp"

#include <QRunnable>
#include <QThreadPool>

#include <algorithm>

namespace QAccelPlot {
namespace {

constexpr auto kPollIntervalMs = int{8};

// Owning copy of a series' records, taken on the series' thread and read by the worker.
struct Snapshot {
    std::vector<float> floats;
    std::vector<double> doubles;
    InspectionSource view;

    explicit Snapshot(const InspectionSource& source)
        : view(source)
    {
        const auto count = static_cast<std::size_t>(std::max(0, source.count));
        if (source.doubles) {
            doubles.assign(source.doubles, source.doubles + count * 2);
        } else if (source.floats) {
            floats.assign(source.floats, source.floats + count * static_cast<std::size_t>(source.floatStride));
        }
        view.values = nullptr;
    }

    // The buffers move with the snapshot, so the view is rebound where it is used.
    InspectionSource source() const
    {
        auto result = view;
        result.doubles = doubles.empty() ? nullptr : doubles.data();
        result.floats = floats.empty() ? nullptr : floats.data();
        return result;
    }
};

std::vector<InspectionIndex::Point> validPoints(const InspectionSource& source, const std::atomic_bool& cancelled)
{
    auto points = std::vector<InspectionIndex::Point>{};
    points.reserve(static_cast<std::size_t>(std::max(0, source.count)));
    for (auto i = 0; i < source.count; ++i) {
        if (i % 4096 == 0 && cancelled.load()) {
            break;
        }
        if (source.valid(i)) {
            points.push_back({source.x(i), source.y(i), i});
        }
    }
    return points;
}

} // namespace

InspectionCache::Host::~Host() = default;

InspectionCache::InspectionCache(Host& host)
    : host_(host)
{
    timer_.setSingleShot(true);
    connect(&timer_, &QTimer::timeout, this, &InspectionCache::tick);
}

InspectionCache::~InspectionCache()
{
    if (job_) {
        job_->cancelled.store(true);
    }
}

void InspectionCache::invalidate()
{
    ++generation_;
    index_.reset();
    if (job_) {
        job_->cancelled.store(true);
    }
    // The next query requests a new index, so data that no longer needs one is never indexed again.
    requested_ = false;
    timer_.stop();
}

void InspectionCache::request()
{
    requested_ = true;
    if (!index_ && !timer_.isActive()) {
        timer_.start(0);
    }
}

bool InspectionCache::requested() const
{
    return requested_;
}

const InspectionIndex* InspectionCache::index() const
{
    return index_.get();
}

void InspectionCache::tick()
{
    if (job_) {
        if (!job_->done.load()) {
            timer_.start(kPollIntervalMs);
            return;
        }
        if (job_->generation == generation_ && !job_->cancelled.load()) {
            index_ = std::move(job_->index);
        }
        job_.reset();
        if (index_) {
            host_.indexReady();
            return;
        }
    }
    if (requested_ && host_.indexSourceAvailable()) {
        launch();
    }
}

void InspectionCache::launch()
{
    job_ = std::make_shared<Job>();
    job_->generation = generation_;
    auto snapshot = std::make_shared<const Snapshot>(host_.indexSource());
    QThreadPool::globalInstance()->start(QRunnable::create([job = job_, snapshot = std::move(snapshot)] {
        const auto source = snapshot->source();
        auto index = std::make_unique<InspectionIndex>();
        if (index->build(validPoints(source, job->cancelled), source.logX, source.logY, job->cancelled)) {
            job->index = std::move(index);
        }
        job->done.store(true);
    }));
    timer_.start(kPollIntervalMs);
}

} // namespace QAccelPlot
