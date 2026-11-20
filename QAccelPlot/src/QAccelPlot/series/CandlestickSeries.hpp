//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/series/PlotSeries.hpp"
#include "QAccelPlot/transitions/TransitionRunner.hpp"

#include <QColor>
#include <QList>
#include <QVariantMap>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

namespace QAccelPlot {

class CandlestickMaterial;

namespace Internal {
class RecordHitIndex;
class StoreTransition;
} // namespace Internal

/// \brief A hardware-accelerated QML item that draws candles: the open, high, low, and close of each data point.
///
/// Each data point is an <tt>(x, open, high, low, close)</tt> quintuple. \c style draws a candle as
/// a body from its open to its close over a wick from its low to its high, or as an OHLC bar: a line
/// from its low to its high with the open as a tick to the left and the close as a tick to the right.
/// A candle whose close is at or above its open is drawn in \c upColor, any other in \c downColor.
///
/// A candle is \c candleWidth times \c spacing wide, at least \c minimumWidth pixels. \c spacing is
/// the smallest positive gap between consecutive x values unless set. To skip non-trading gaps, use
/// the bar index as x and map it to a date with a tick label formatter.
///
/// Data points are uploaded to the GPU as a float data texture once per data change; panning,
/// zooming, and changing \c style, the colors, or the widths only change shader uniforms. The
/// \c setData() overloads keep doubles and upload them relative to an origin near the viewport, so
/// large coordinates such as epoch timestamps stay precise. The \c setDataF() overloads store floats
/// and upload them without conversion.
///
/// \code
/// QAccelPlot.CandlestickSeries {
///     xAxis: plot.xAxis
///     yAxis: plot.yAxis
///     Component.onCompleted: setData([0, 1, 2], [10, 11, 10.5], [11.2, 11.4, 10.9], [9.8, 10.6, 9.9], [11, 10.5, 10.1])
/// }
/// \endcode
///
/// \par Invalid data points
/// A candle is not drawn or hovered when its \c x is NaN or ±Inf, or not strictly positive on a
/// log-scale axis, or when its open or close is NaN. A NaN high or low leaves out that half of the
/// wick or line, which then ends at the body. An infinite value, and a value that is not strictly
/// positive on a log-scale axis, extends past the plot edge. Inconsistent values, such as a high below
/// the body, are drawn as given. On a log-scale x axis, a candle whose left edge would be at or below
/// 0 starts at its \c x. Auto-ranging counts every valid coordinate and widens x by half a candle on
/// each side.
///
/// \par Hover
/// \c hoveredIndex is the candle whose body, wick, line, or ticks lie within \c hoverRadius pixels of
/// the cursor, the nearest one when several do, and the one with the highest index on a tie. A valid
/// \c hoverColor draws it. Only the topmost series under the cursor is hovered: set \c hoverEnabled to
/// \c false to leave hover to the series beneath.
///
/// \par Transitions
/// \c enterTransition animates added candles, \c updateTransition kept ones, and \c exitTransition
/// removed ones, matched by index. With a \c MorphTransition, kept candles move to their new x, open,
/// high, low, and close; a \c GrowTransition grows the high, low, and close from each candle's open
/// unless its \c baseline is set. \c count, \c candleAt(), the data ranges, \c spacing, and inspection
/// use the new data from the start of the animation, while hover picks the candles as drawn. While a
/// transition is enabled, the \c setDataF() overloads store doubles. \c appendData(), \c replaceData(),
/// and replacing, clearing, or cancelling a transition skip to the new data; \c clearData() runs the
/// exit transition.
///
/// \par Limits
/// At most 16,777,216 (2^24) candles are drawn, fewer on GPUs whose maximum texture size is below
/// 10240. Candles beyond the limit are not drawn or hovered, and a warning is logged once.
///
/// \sa BarSeries, ErrorBarSeries, MorphTransition, GrowTransition
class CandlestickSeries : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(CandlestickSeries)

    /// \brief How each candle is drawn. Default: \c CandlestickSeries.Candlestick.
    Q_PROPERTY(Style style READ style WRITE setStyle NOTIFY styleChanged)
    /// \brief Color of up candles, whose close is at or above their open. Default: \c Colors.dark.candleUp.
    Q_PROPERTY(QColor upColor READ upColor WRITE setUpColor NOTIFY upColorChanged)
    /// \brief Color of down candles, whose close is below their open. Default: \c Colors.dark.candleDown.
    Q_PROPERTY(QColor downColor READ downColor WRITE setDownColor NOTIFY downColorChanged)
    /// \brief Width of a candle as a fraction of \c spacing, clamped to at least 0. Default: 0.8.
    ///
    /// Each OHLC tick is half this width long.
    Q_PROPERTY(qreal candleWidth READ candleWidth WRITE setCandleWidth NOTIFY candleWidthChanged)
    /// \brief Distance between neighboring candles in x data units.
    ///
    /// Defaults to the smallest positive gap between consecutive x values in stored order, ignoring
    /// non-finite ones, or 1 without such a gap. Reading it returns the spacing in effect. Assign a
    /// positive value to pin it, for example on timestamps with gaps; assign \c undefined to return
    /// to the default. While the default applies, \c spacingChanged() is emitted on every data change.
    Q_PROPERTY(qreal spacing READ spacing WRITE setSpacing RESET resetSpacing NOTIFY spacingChanged)
    /// \brief Width in pixels of wicks, OHLC lines, and ticks, clamped to at least 0. Default: 1.
    ///
    /// A body is at least this tall, so a candle whose open equals its close stays visible.
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    /// \brief Smallest candle width in pixels, clamped to at least 0. Default: 1.
    Q_PROPERTY(qreal minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumWidthChanged)
    /// \brief Color of the hovered candle. Default: an invalid color, which draws it in its own color.
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    /// \brief Pick distance in pixels from a candle, clamped to at least 0. Default: 4.
    Q_PROPERTY(qreal hoverRadius READ hoverRadius WRITE setHoverRadius NOTIFY hoverRadiusChanged)
    /// \brief Transition of the candles a data change adds, or \c nullptr to show them at once.
    Q_PROPERTY(EnterExitTransition* enterTransition READ enterTransition WRITE setEnterTransition NOTIFY enterTransitionChanged)
    /// \brief Transition of the candles a data change keeps, or \c nullptr to move them at once.
    Q_PROPERTY(DataTransition* updateTransition READ updateTransition WRITE setUpdateTransition NOTIFY updateTransitionChanged)
    /// \brief Transition of the candles a data change removes, or \c nullptr to remove them at once.
    Q_PROPERTY(EnterExitTransition* exitTransition READ exitTransition WRITE setExitTransition NOTIFY exitTransitionChanged)
    /// \brief Read-only: index of the candle under the cursor, or -1 when none.
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)

public:
    /// \brief How a candle is drawn.
    enum class Style {
        Candlestick, ///< \brief A body from the open to the close over a wick from the low to the high.
        Ohlc,        ///< \brief A line from the low to the high, with the open as a tick to the left and the close as a tick to the right.
    };
    Q_ENUM(Style)

    /// \brief Constructs a CandlestickSeries with the given \a parent.
    explicit CandlestickSeries(QQuickItem* parent = nullptr);
    /// \brief Destroys the series, ending its animation on the assigned transition.
    ~CandlestickSeries() override;

    /// \brief Returns how each candle is drawn.
    Style style() const;
    /// \brief Sets how each candle is drawn to \a style.
    void setStyle(Style style);

    /// \brief Returns the color of up candles.
    QColor upColor() const;
    /// \brief Sets the color of up candles to \a color.
    void setUpColor(const QColor& color);

    /// \brief Returns the color of down candles.
    QColor downColor() const;
    /// \brief Sets the color of down candles to \a color.
    void setDownColor(const QColor& color);

    /// \brief Returns the candle width as a fraction of \c spacing.
    qreal candleWidth() const;
    /// \brief Sets the candle width to \a width times \c spacing. Negative values are clamped to 0.
    void setCandleWidth(qreal width);

    /// \brief Returns the spacing in effect, in x data units.
    qreal spacing() const;
    /// \brief Pins the spacing to \a spacing x data units. A value that is not finite and positive returns to the default.
    void setSpacing(qreal spacing);
    /// \brief Returns to the default spacing: the smallest positive gap between consecutive x values.
    void resetSpacing();

    /// \brief Returns the width of wicks, OHLC lines, and ticks in pixels.
    qreal lineWidth() const;
    /// \brief Sets the width of wicks, OHLC lines, and ticks to \a width pixels. Negative values are clamped to 0.
    void setLineWidth(qreal width);

    /// \brief Returns the smallest candle width in pixels.
    qreal minimumWidth() const;
    /// \brief Sets the smallest candle width to \a width pixels. Negative values are clamped to 0.
    void setMinimumWidth(qreal width);

    /// \brief Returns the color of the hovered candle, or an invalid color.
    QColor hoverColor() const;
    /// \brief Sets the color of the hovered candle to \a color; an invalid color draws it in its own color.
    void setHoverColor(const QColor& color);

    /// \brief Returns the hover pick distance in pixels.
    qreal hoverRadius() const;
    /// \brief Sets the hover pick distance to \a radius pixels. Negative values are clamped to 0.
    void setHoverRadius(qreal radius);

    /// \brief Returns the transition of added candles, or \c nullptr.
    EnterExitTransition* enterTransition() const;
    /// \brief Sets the transition of added candles to \a transition.
    void setEnterTransition(EnterExitTransition* transition);
    /// \brief Returns the transition of kept candles, or \c nullptr.
    DataTransition* updateTransition() const;
    /// \brief Sets the transition of kept candles to \a transition.
    void setUpdateTransition(DataTransition* transition);
    /// \brief Returns the transition of removed candles, or \c nullptr.
    EnterExitTransition* exitTransition() const;
    /// \brief Sets the transition of removed candles to \a transition.
    void setExitTransition(EnterExitTransition* transition);

    /// \brief Returns the index of the hovered candle, or -1 if none.
    int hoveredIndex() const;

    /// \brief Returns a legend entry with the \c Candle symbol in the up and down colors.
    std::optional<LegendEntry> legendEntry() const override;

    /// \brief Appends the data point (\a x, \a open, \a high, \a low, \a close). Triggers a redraw.
    Q_INVOKABLE void appendData(qreal x, qreal open, qreal high, qreal low, qreal close);
    /// \brief Replaces data point \a index with (\a x, \a open, \a high, \a low, \a close). Triggers a redraw.
    ///
    /// An \a index out of range logs a warning and changes nothing. The data ranges and the default
    /// \c spacing update in place, except that a new \a x rescans the x values for the default
    /// \c spacing. When the default \c spacing changes, or the candle no longer reaches an edge of the
    /// data ranges that it reached before, the next read of the data ranges rescans the candles.
    Q_INVOKABLE void replaceData(int index, qreal x, qreal open, qreal high, qreal low, qreal close);
    /// \brief Replaces the data points with \a xs, \a opens, \a highs, \a lows, and \a closes. If the sizes differ, the shortest length is used.
    Q_INVOKABLE void setData(
        const QList<qreal>& xs, const QList<qreal>& opens, const QList<qreal>& highs, const QList<qreal>& lows, const QList<qreal>& closes);
    /// \brief Replaces the data points with \a xs, \a opens, \a highs, \a lows, and \a closes. If the sizes differ, the shortest length is used.
    void setData(const std::vector<double>& xs, const std::vector<double>& opens, const std::vector<double>& highs, const std::vector<double>& lows,
        const std::vector<double>& closes);
    // The PlotSeries overloads take interleaved (x, open, high, low, close) quintuples, as doubles or floats.
    using PlotSeries::setData;
    using PlotSeries::setDataF;

    /// \brief Removes all data points.
    Q_INVOKABLE void clearData() override;

    /// \brief Returns data point \a index as an object with \c x, \c open, \c high, \c low, and \c close properties, or an empty object when out of range.
    Q_INVOKABLE QVariantMap candleAt(int index) const;
    /// \brief Returns the index of the candle within \c hoverRadius of item position \a position, or -1.
    ///
    /// The nearest candle wins; on a tie, the one with the highest index.
    Q_INVOKABLE int candleIndexAt(const QPointF& position) const;
    /// \brief Returns \c true when a candle lies within \c hoverRadius of item position \a point.
    ///
    /// Hover delivery uses this test, so stacked series underneath still receive hover events away from
    /// the candles.
    bool contains(const QPointF& point) const override;

signals:
    /// \brief Emitted when the style property changes.
    void styleChanged();
    /// \brief Emitted when the upColor property changes.
    void upColorChanged();
    /// \brief Emitted when the downColor property changes.
    void downColorChanged();
    /// \brief Emitted when the candleWidth property changes.
    void candleWidthChanged();
    /// \brief Emitted when the spacing in effect may have changed.
    void spacingChanged();
    /// \brief Emitted when the lineWidth property changes.
    void lineWidthChanged();
    /// \brief Emitted when the minimumWidth property changes.
    void minimumWidthChanged();
    /// \brief Emitted when the hoverColor property changes.
    void hoverColorChanged();
    /// \brief Emitted when the hoverRadius property changes.
    void hoverRadiusChanged();
    /// \brief Emitted when the enterTransition property changes.
    void enterTransitionChanged();
    /// \brief Emitted when the updateTransition property changes.
    void updateTransitionChanged();
    /// \brief Emitted when the exitTransition property changes.
    void exitTransitionChanged();
    /// \brief Emitted when the hovered candle changes.
    void hoveredIndexChanged();

protected:
    /// \cond INTERNAL
    bool recordInspectionSupported() const override;
    InspectionRecord inspectionRecord(int index) const override;
    InspectionRecord inspectionRecordAt(const QPointF& position) const override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    /// \endcond
    /// \brief Stores \a input, animating it from the drawn candles while a transition is enabled.
    void applyDataPoints(DataPointInput&& input) override;
    /// \brief Returns \c true for \c Glow.
    bool supportsEffect(const SeriesEffect& effect) const override;
    /// \brief Refreshes the hover lookup and repaints after a bound axis changes between linear and log scale.
    void onAxisScaleChanged() override;
    /// \brief Updates the default spacing; returns \c true when an appended or modified data point changes it.
    bool updateDataRangeInputs(const DataUpdate& dataUpdate) override;
    /// \brief Refreshes the hover lookup, and announces the default spacing when the change moved it.
    void onDataPointsChanged(const DataUpdate& dataUpdate) override;
    /// \brief Returns the extents of the valid coordinates, with x widened by half a candle on each side.
    DataRanges computeDataRanges() const override;
    /// \brief Returns the extents data point \a index adds to \c computeDataRanges().
    DataPointStore::Extents dataPointExtents(int index) const override;

private:
    // A pixel rectangle of a drawn candle: its wick or OHLC line, its body, or an OHLC tick.
    struct PixelBox {
        qreal left;
        qreal right;
        qreal top;
        qreal bottom;
    };

    // The pixel rectangles of one drawn candle.
    struct CandleBoxes {
        std::array<PixelBox, 3> boxes;
        int count{0};
    };

    // The default spacing, refreshed lazily and in O(1) on append.
    struct AutoSpacing {
        double minimumGap{std::numeric_limits<double>::infinity()};
        double lastX{std::numeric_limits<double>::quiet_NaN()};
        int count{0};
        bool valid{false};

        // Takes \a x as the next x value in stored order.
        void include(double x);
        // The spacing: the smallest gap, or 1 without one.
        double value() const;
    };

    bool automaticSpacing() const;
    void ensureAutoSpacing() const;
    double halfCandleWidth() const;
    void updateAutoSpacing(const DataUpdate& dataUpdate);
    // Returns the pixel boxes of drawn candle \a index, or nullopt when it is not drawn.
    std::optional<CandleBoxes> pixelBoxes(int index) const;
    // Pixel distance from \a position to drawn candle \a index, or nullopt when it is not drawn.
    std::optional<qreal> pixelDistance(int index, const QPointF& position) const;
    void applyInterleavedData(std::vector<double>&& data);
    void applyDoubleData(std::vector<double>&& data, int dataPointCount);
    // How the candles animate.
    TransitionRunner::Layout transitionLayout() const;
    // Draws the next frame of the running transition, or the data when it ends.
    void advanceTransition();
    // Shows the stored data points at once, when a transition was drawing others.
    void endTransition();
    void discardTransition();
    void setHoveredIndex(int index);
    void onGeometryChanged();
    void ensureSpatialGrid() const;
    void updateMaterial(CandlestickMaterial& material, int drawnCount) const;

    Style style_{Style::Candlestick};
    QColor upColor_;
    QColor downColor_;
    qreal candleWidth_{0.8};
    // NaN while the default spacing applies.
    qreal pinnedSpacing_{std::numeric_limits<qreal>::quiet_NaN()};
    mutable AutoSpacing autoSpacing_;
    // Set while a data change moved the default spacing in effect, until onDataPointsChanged() announces it.
    bool spacingChangePending_{false};
    qreal lineWidth_{1.0};
    qreal minimumWidth_{1.0};
    QColor hoverColor_;
    qreal hoverRadius_{4.0};
    int hoveredIndex_{-1};
    std::unique_ptr<TransitionRunner> transition_;
    // Shows the runs of transition_ as the drawn data points.
    std::unique_ptr<Internal::StoreTransition> storeTransition_;
    // Render revision of the data points last uploaded to the GPU.
    std::uint64_t uploadedRevision_{0};
    // Data-space rectangles of the candles that bound the hover candidates.
    std::unique_ptr<Internal::RecordHitIndex> hitIndex_;
};

} // namespace QAccelPlot
