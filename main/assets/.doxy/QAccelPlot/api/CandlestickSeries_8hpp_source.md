

# File CandlestickSeries.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**CandlestickSeries.hpp**](CandlestickSeries_8hpp.md)

[Go to the documentation of this file](CandlestickSeries_8hpp.md)


```C++
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

class CandlestickSeries : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(CandlestickSeries)

    
    Q_PROPERTY(Style style READ style WRITE setStyle NOTIFY styleChanged)
    Q_PROPERTY(QColor upColor READ upColor WRITE setUpColor NOTIFY upColorChanged)
    Q_PROPERTY(QColor downColor READ downColor WRITE setDownColor NOTIFY downColorChanged)
    Q_PROPERTY(qreal candleWidth READ candleWidth WRITE setCandleWidth NOTIFY candleWidthChanged)
    Q_PROPERTY(qreal spacing READ spacing WRITE setSpacing RESET resetSpacing NOTIFY spacingChanged)
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    Q_PROPERTY(qreal minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumWidthChanged)
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    Q_PROPERTY(qreal hoverRadius READ hoverRadius WRITE setHoverRadius NOTIFY hoverRadiusChanged)
    Q_PROPERTY(EnterExitTransition* enterTransition READ enterTransition WRITE setEnterTransition NOTIFY enterTransitionChanged)
    Q_PROPERTY(DataTransition* updateTransition READ updateTransition WRITE setUpdateTransition NOTIFY updateTransitionChanged)
    Q_PROPERTY(EnterExitTransition* exitTransition READ exitTransition WRITE setExitTransition NOTIFY exitTransitionChanged)
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)

public:
    enum class Style {
        Candlestick, 
        Ohlc,        
    };
    Q_ENUM(Style)

    
    explicit CandlestickSeries(QQuickItem* parent = nullptr);
    ~CandlestickSeries() override;

    Style style() const;
    void setStyle(Style style);

    QColor upColor() const;
    void setUpColor(const QColor& color);

    QColor downColor() const;
    void setDownColor(const QColor& color);

    qreal candleWidth() const;
    void setCandleWidth(qreal width);

    qreal spacing() const;
    void setSpacing(qreal spacing);
    void resetSpacing();

    qreal lineWidth() const;
    void setLineWidth(qreal width);

    qreal minimumWidth() const;
    void setMinimumWidth(qreal width);

    QColor hoverColor() const;
    void setHoverColor(const QColor& color);

    qreal hoverRadius() const;
    void setHoverRadius(qreal radius);

    EnterExitTransition* enterTransition() const;
    void setEnterTransition(EnterExitTransition* transition);
    DataTransition* updateTransition() const;
    void setUpdateTransition(DataTransition* transition);
    EnterExitTransition* exitTransition() const;
    void setExitTransition(EnterExitTransition* transition);

    int hoveredIndex() const;

    std::optional<LegendEntry> legendEntry() const override;

    Q_INVOKABLE void appendData(qreal x, qreal open, qreal high, qreal low, qreal close);
    Q_INVOKABLE void replaceData(int index, qreal x, qreal open, qreal high, qreal low, qreal close);
    Q_INVOKABLE void setData(
        const QList<qreal>& xs, const QList<qreal>& opens, const QList<qreal>& highs, const QList<qreal>& lows, const QList<qreal>& closes);
    void setData(const std::vector<double>& xs, const std::vector<double>& opens, const std::vector<double>& highs, const std::vector<double>& lows,
        const std::vector<double>& closes);
    // The PlotSeries overloads take interleaved (x, open, high, low, close) quintuples, as doubles or floats.
    using PlotSeries::setData;
    using PlotSeries::setDataF;

    Q_INVOKABLE void clearData() override;

    Q_INVOKABLE QVariantMap candleAt(int index) const;
    Q_INVOKABLE int candleIndexAt(const QPointF& position) const;
    bool contains(const QPointF& point) const override;

signals:
    void styleChanged();
    void upColorChanged();
    void downColorChanged();
    void candleWidthChanged();
    void spacingChanged();
    void lineWidthChanged();
    void minimumWidthChanged();
    void hoverColorChanged();
    void hoverRadiusChanged();
    void enterTransitionChanged();
    void updateTransitionChanged();
    void exitTransitionChanged();
    void hoveredIndexChanged();

protected:
    bool recordInspectionSupported() const override;
    InspectionRecord inspectionRecord(int index) const override;
    InspectionRecord inspectionRecordAt(const QPointF& position) const override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void applyDataPoints(DataPointInput&& input) override;
    bool supportsEffect(const SeriesEffect& effect) const override;
    void onAxisScaleChanged() override;
    bool updateDataRangeInputs(const DataUpdate& dataUpdate) override;
    void onDataPointsChanged(const DataUpdate& dataUpdate) override;
    DataRanges computeDataRanges() const override;
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
```


