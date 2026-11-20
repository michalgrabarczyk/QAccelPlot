//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/series/internal/StoreTransition.hpp"

#include "QAccelPlot/series/DataPointStore.hpp"

#include <cstddef>
#include <limits>
#include <utility>

namespace QAccelPlot::Internal {

namespace {

// Copies the data points of \a view as doubles, for float and double data alike.
std::vector<double> doubleData(const DataPointStore::View& view, const int stride)
{
    auto data = std::vector<double>(static_cast<std::size_t>(view.count) * static_cast<std::size_t>(stride));
    for (auto index = 0; index < view.count; ++index) {
        for (auto column = 0; column < stride; ++column) {
            data[static_cast<std::size_t>(index) * static_cast<std::size_t>(stride) + static_cast<std::size_t>(column)] = view.value(index, column);
        }
    }
    return data;
}

} // namespace

StoreTransition::StoreTransition(TransitionRunner& runner, DataPointStore& store)
    : runner_(runner)
    , store_(store)
{
}

bool StoreTransition::start(std::vector<double>&& data, const int count, const TransitionRunner::Layout& layout)
{
    // Copied before the data changes: a run starts from the data points drawn now.
    const auto stride = store_.columnCount();
    const auto drawnView = store_.drawnView();
    const auto drawn = doubleData(drawnView, stride);
    store_.setPrecise(std::move(data), count);
    const auto stored = store_.view();
    return runner_.start(drawn, drawnView.count, doubleData(stored, stride), stored.count, layout);
}

bool StoreTransition::advance()
{
    auto shownCount = 0;
    runner_.advance(frameBuffer_, shownCount);
    if (!runner_.pending()) {
        return end();
    }
    // Data points the frame does not show, before they enter or after they exit, are not drawn.
    const auto frameCount = runner_.frameCapacity();
    frameBuffer_.resize(static_cast<std::size_t>(frameCount) * static_cast<std::size_t>(store_.columnCount()), std::numeric_limits<double>::quiet_NaN());
    frameBuffer_ = store_.setDrawn(std::move(frameBuffer_), frameCount);
    return true;
}

bool StoreTransition::end()
{
    if (!store_.hasDrawn()) {
        return false;
    }
    discard();
    return true;
}

void StoreTransition::discard()
{
    runner_.cancel();
    store_.clearDrawn();
    frameBuffer_ = std::vector<double>{};
}

} // namespace QAccelPlot::Internal
