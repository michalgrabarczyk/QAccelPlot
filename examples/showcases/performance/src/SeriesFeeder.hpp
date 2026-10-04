//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "GenerationWorker.hpp"
#include "PageScene.hpp"

#include <QLatin1String>

#include <cstddef>

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
};

/// \brief Feeds the series of one page from a generation worker.
///
/// \a Feed names the page and its types: \c Generator, \c Series, \c kSettingsProperty,
/// \c kPlotName, <tt>parameters(const PageScene&, const CommonOptions&)</tt> returning the
/// generator parameters, and <tt>apply(Series&, Part&, const CommonOptions&)</tt> handing one
/// part of a batch to a series.
template <typename Feed> class SeriesFeeder final : public PageFeeder {
public:
    /// \brief Looks up the page's plot below \a root.
    explicit SeriesFeeder(QObject* root)
        : scene_(root, Feed::kSettingsProperty, QLatin1String(Feed::kPlotName))
    {
    }

    void start(const double timeSeconds) override
    {
        configure(scene_.commonOptions(), timeSeconds);
        worker_.start();
    }

    void stop() override
    {
        worker_.requestStop();
    }

    bool update(const double timeSeconds) override
    {
        const auto options = scene_.commonOptions();
        configure(options, timeSeconds);

        auto batch = typename Feed::Generator::Batch{};
        if (!worker_.tryConsume(batch)) {
            return false;
        }
        const auto series = scene_.template series<typename Feed::Series>();
        if (series.empty() || series.size() != batch.parts.size()) {
            return false;
        }
        for (auto index = std::size_t{0}; index < series.size(); ++index) {
            Feed::apply(*series[index], batch.parts[index], options);
        }
        return true;
    }

private:
    void configure(const CommonOptions& options, const double timeSeconds)
    {
        worker_.setParameters(Feed::parameters(scene_, options));
        worker_.setTime(timeSeconds);
    }

    PageScene scene_;
    GenerationWorker<typename Feed::Generator> worker_;
};

} // namespace QAccelPlotExample
