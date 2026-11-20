//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QVariantMap>

#include <random>
#include <vector>

namespace QAccelPlot {
class BarSeries;
class CandlestickSeries;
} // namespace QAccelPlot

namespace QAccelPlotExample {

/// \brief Simulated hourly prices: a random-walk history and a live last candle.
///
/// Fills a \c CandlestickSeries with the candles and a \c BarSeries with their volume, then updates
/// the last candle in place with \c replaceData() and appends a new one every few seconds.
class MarketFeed : public QObject {
    Q_OBJECT
    /// \brief Whether the last candle keeps updating. Default: \c true.
    Q_PROPERTY(bool live READ live WRITE setLive NOTIFY liveChanged)

public:
    /// \brief Constructs a feed with the given \a parent.
    explicit MarketFeed(QObject* parent = nullptr);

    /// \brief Returns whether the last candle keeps updating.
    bool live() const;
    /// \brief Sets whether the last candle keeps updating to \a live.
    void setLive(bool live);

    /// \brief Fills \a candles and \a volume with \a history candles and starts the live updates.
    Q_INVOKABLE void start(QAccelPlot::CandlestickSeries* candles, QAccelPlot::BarSeries* volume, int history);
    /// \brief Returns the lowest low, highest high, and largest volume of the candles from \a first to \a last as \c low, \c high, and \c volume.
    Q_INVOKABLE QVariantMap extent(double first, double last) const;

signals:
    /// \brief Emitted when the live property changes.
    void liveChanged();
    /// \brief Emitted after a new candle was appended.
    void candleAppended();
    /// \brief Emitted after the last candle changed.
    void lastCandleChanged();

private:
    struct Candle {
        double open;
        double high;
        double low;
        double close;
        double volume;
    };

    // Opens a candle at \a open and moves it through \a ticks price ticks.
    Candle simulateCandle(double open, int ticks);
    // Moves the last candle by one price tick.
    void tick();
    void appendCandle();
    void uploadVolume();

    std::vector<Candle> candles_;
    QPointer<QAccelPlot::CandlestickSeries> candleSeries_;
    QPointer<QAccelPlot::BarSeries> volumeSeries_;
    std::mt19937 random_{20260101};
    std::normal_distribution<double> step_{0.0, 1.0};
    QTimer timer_;
    bool live_{true};
    int ticksInCandle_{0};
};

} // namespace QAccelPlotExample
