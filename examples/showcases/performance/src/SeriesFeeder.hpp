//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "DataDeliveryMetrics.hpp"
#include "GenerationWorker.hpp"
#include "PageScene.hpp"

#include <QLatin1String>

#include <chrono>
#include <cstddef>
#include <memory>
#include <optional>

namespace QAccelPlotExample {

/// \brief Feeds one showcase page with generated data.
class PageFeeder {
public:
    virtual ~PageFeeder() = default;

    /// \brief Starts generating batches for \a timeSeconds.
    virtual void start(double timeSeconds) = 0;
    /// \brief Stops generating after the current batch.
    virtual void stop() = 0;
    /// \brief Applies the pending batch, if any, and requests the next one for \a timeSeconds.
    ///
    /// Returns \c true when the page's series received new data.
    virtual bool update(double timeSeconds) = 0;
    /// \brief Returns \c true until the series hold data generated for the current settings.
    virtual bool isLoading() const = 0;
};

/// \brief Feeds the series of one page with batches replayed by a \c GenerationWorker.
///
/// \a Feed names the page and its types: \c Generator, \c Series, \c kSettingsProperty,
/// \c kPlotName, <tt>parameters(const PageScene&, const CommonOptions&)</tt> returning the
/// generator parameters, and <tt>apply(Series&, Part&, const CommonOptions&)</tt> handing one
/// part of a batch to a series. A batch generated for other settings than the current ones,
/// such as another series count, is dropped. The feeder keeps the applied batch until the next
/// one arrives and then hands it back to the worker, which reuses whatever storage the series
/// did not take.
template <typename Feed> class SeriesFeeder final : public PageFeeder {
public:
    /// \brief Looks up the page's plot below \a root and reports copy times to \a metrics.
    ///
    /// With \a still, the worker replays one frame, so every batch holds the same data.
    SeriesFeeder(QObject* root, const std::shared_ptr<DataDeliveryMetrics>& metrics, const bool still)
        : scene_(root, Feed::kSettingsProperty, QLatin1String(Feed::kPlotName))
    {
        worker_.setCopyCallback([metrics](const std::chrono::nanoseconds duration) { metrics->batchCopied(duration.count()); });
        if (still) {
            worker_.setFrameInterval(0.0);
        }
    }

    void start(const double timeSeconds) override
    {
        configure(scene_.commonOptions(), timeSeconds);
        worker_.start();
    }

    void stop() override
    {
        worker_.requestStop();
        batch_ = Batch{};
        appliedParameters_.reset();
    }

    bool update(const double timeSeconds) override
    {
        const auto options = scene_.commonOptions();
        configure(options, timeSeconds);

        auto builtFor = Parameters{};
        if (!worker_.tryConsume(batch_, builtFor)) {
            return false;
        }
        const auto series = scene_.template series<typename Feed::Series>();
        if (series.empty() || series.size() != batch_.parts.size() || !hasPrecisionFor(batch_, options.ingestion)) {
            return false;
        }
        for (auto index = std::size_t{0}; index < series.size(); ++index) {
            if (series[index]->acceptHoverEvents() != options.hoverEnabled) {
                series[index]->setAcceptHoverEvents(options.hoverEnabled);
            }
            Feed::apply(*series[index], batch_.parts[index], options);
        }
        appliedParameters_ = builtFor;
        return true;
    }

    bool isLoading() const override
    {
        return !appliedParameters_ || !(*appliedParameters_ == requestedParameters_);
    }

private:
    using Batch = typename Feed::Generator::Batch;
    using Parameters = typename Feed::Generator::Parameters;

    static bool hasPrecisionFor(const Batch& batch, const Ingestion ingestion)
    {
        for (const auto& part : batch.parts) {
            if (!QAccelPlotExample::hasPrecisionFor(part, ingestion)) {
                return false;
            }
        }
        return true;
    }

    void configure(const CommonOptions& options, const double timeSeconds)
    {
        requestedParameters_ = Feed::parameters(scene_, options);
        worker_.setParameters(requestedParameters_);
        worker_.setTime(timeSeconds);
    }

    PageScene scene_;
    GenerationWorker<typename Feed::Generator> worker_;
    Batch batch_;
    Parameters requestedParameters_;
    std::optional<Parameters> appliedParameters_;
};

} // namespace QAccelPlotExample
