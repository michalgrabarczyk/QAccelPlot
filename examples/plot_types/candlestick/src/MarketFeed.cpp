//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "MarketFeed.hpp"

#include <QAccelPlot/series/BarSeries.hpp>
#include <QAccelPlot/series/CandlestickSeries.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace QAccelPlotExample {

namespace {

constexpr auto kTickIntervalMs = 150;
// Live price ticks per candle, so a new candle opens every three seconds.
constexpr auto kTicksPerCandle = 20;
// Price ticks simulated for each candle of the history.
constexpr auto kHistoryTicks = 12;
// Relative price move per tick.
constexpr auto kVolatility = 0.0015;
// Category values of the volume bars, as declared in main.qml.
constexpr auto kUpCategory = 0;
constexpr auto kDownCategory = 1;

// The volume bar category of a candle from its open and close.
int volumeCategory(const double open, const double close)
{
    return close >= open ? kUpCategory : kDownCategory;
}

} // namespace

MarketFeed::MarketFeed(QObject* parent)
    : QObject(parent)
{
    timer_.setInterval(kTickIntervalMs);
    connect(&timer_, &QTimer::timeout, this, &MarketFeed::tick);
}

bool MarketFeed::live() const
{
    return live_;
}

void MarketFeed::setLive(const bool live)
{
    if (live_ == live) {
        return;
    }
    live_ = live;
    if (live_ && candleSeries_) {
        timer_.start();
    } else {
        timer_.stop();
    }
    emit liveChanged();
}

void MarketFeed::start(QAccelPlot::CandlestickSeries* candles, QAccelPlot::BarSeries* volume, const int history)
{
    candleSeries_ = candles;
    volumeSeries_ = volume;
    candles_.clear();
    candles_.reserve(static_cast<std::size_t>(history));
    auto close = 100.0;
    for (auto i = 0; i < history; ++i) {
        candles_.push_back(simulateCandle(close, kHistoryTicks));
        close = candles_.back().close;
    }

    auto xs = std::vector<double>(candles_.size());
    auto opens = std::vector<double>(candles_.size());
    auto highs = std::vector<double>(candles_.size());
    auto lows = std::vector<double>(candles_.size());
    auto closes = std::vector<double>(candles_.size());
    for (auto i = std::size_t{0}; i < candles_.size(); ++i) {
        xs[i] = static_cast<double>(i);
        opens[i] = candles_[i].open;
        highs[i] = candles_[i].high;
        lows[i] = candles_[i].low;
        closes[i] = candles_[i].close;
    }
    if (candleSeries_) {
        candleSeries_->setData(xs, opens, highs, lows, closes);
    }
    uploadVolume();
    ticksInCandle_ = kHistoryTicks;
    if (live_) {
        timer_.start();
    }
}

QVariantMap MarketFeed::extent(const double first, const double last) const
{
    const auto count = static_cast<double>(candles_.size());
    const auto begin = static_cast<std::size_t>(std::clamp(std::floor(first), 0.0, count));
    const auto end = static_cast<std::size_t>(std::clamp(std::ceil(last) + 1.0, 0.0, count));
    if (begin >= end) {
        return {};
    }
    auto low = std::numeric_limits<double>::infinity();
    auto high = -std::numeric_limits<double>::infinity();
    auto volume = 0.0;
    for (auto i = begin; i < end; ++i) {
        low = std::min(low, candles_[i].low);
        high = std::max(high, candles_[i].high);
        volume = std::max(volume, candles_[i].volume);
    }
    return {{QStringLiteral("low"), low}, {QStringLiteral("high"), high}, {QStringLiteral("volume"), volume}};
}

MarketFeed::Candle MarketFeed::simulateCandle(const double open, const int ticks)
{
    auto candle = Candle{open, open, open, open, 0.0};
    // Some hours trade more than others, and busy hours move the price more.
    const auto activity = std::exp(0.5 * step_(random_));
    for (auto i = 0; i < ticks; ++i) {
        candle.close *= 1.0 + kVolatility * std::sqrt(activity) * step_(random_);
        candle.high = std::max(candle.high, candle.close);
        candle.low = std::min(candle.low, candle.close);
        candle.volume += 1000.0 * activity * (1.0 + std::abs(step_(random_)));
    }
    return candle;
}

void MarketFeed::tick()
{
    if (!candleSeries_ || candles_.empty()) {
        return;
    }
    if (ticksInCandle_ >= kTicksPerCandle) {
        appendCandle();
        return;
    }
    auto& last = candles_.back();
    const auto moved = simulateCandle(last.close, 1);
    last.close = moved.close;
    last.high = std::max(last.high, moved.high);
    last.low = std::min(last.low, moved.low);
    last.volume += moved.volume;
    ++ticksInCandle_;

    // Only the last candle changed: replace it in place instead of uploading every candle again.
    const auto index = static_cast<int>(candles_.size()) - 1;
    candleSeries_->replaceData(index, index, last.open, last.high, last.low, last.close);
    if (volumeSeries_) {
        volumeSeries_->replaceData(index, index, last.volume, volumeCategory(last.open, last.close));
    }
    emit lastCandleChanged();
}

void MarketFeed::appendCandle()
{
    const auto open = candles_.back().close;
    candles_.push_back(Candle{open, open, open, open, 0.0});
    ticksInCandle_ = 0;
    const auto index = static_cast<int>(candles_.size()) - 1;
    candleSeries_->appendData(index, open, open, open, open);
    uploadVolume();
    emit candleAppended();
}

void MarketFeed::uploadVolume()
{
    if (!volumeSeries_) {
        return;
    }
    // One double pair per candle.
    auto bars = std::vector<double>(candles_.size() * 2);
    auto categories = std::vector<int>(candles_.size());
    for (auto i = std::size_t{0}; i < candles_.size(); ++i) {
        bars[i * 2] = static_cast<double>(i);
        bars[i * 2 + 1] = candles_[i].volume;
        categories[i] = volumeCategory(candles_[i].open, candles_[i].close);
    }
    volumeSeries_->setData(std::move(bars), std::move(categories), static_cast<int>(candles_.size()));
}

} // namespace QAccelPlotExample
