

# File BarSeries.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**BarSeries.hpp**](BarSeries_8hpp.md)

[Go to the documentation of this file](BarSeries_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/Border.hpp"
#include "QAccelPlot/series/ColormappedSeries.hpp"
#include "QAccelPlot/series/RectVertexCache.hpp"
#include "QAccelPlot/series/SpatialGrid.hpp"
#include "QAccelPlot/transitions/DataTransition.hpp"

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <array>
#include <memory>
#include <vector>

namespace QAccelPlot {

class BarMaterial;
class TransitionRunner;

class BarSeries : public ColormappedSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(BarSeries)

    
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(qreal barWidth READ barWidth WRITE setBarWidth NOTIFY barWidthChanged)
    Q_PROPERTY(qreal barOffset READ barOffset WRITE setBarOffset NOTIFY barOffsetChanged)
    Q_PROPERTY(qreal baselineValue READ baselineValue WRITE setBaselineValue NOTIFY baselineValueChanged)
    Q_PROPERTY(qreal minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumWidthChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QList<QColor> categoryColors READ categoryColors WRITE setCategoryColors NOTIFY categoryColorsChanged)
    Q_PROPERTY(ColorSource colorSource READ colorSource WRITE setColorSource NOTIFY colorSourceChanged)
    Q_PROPERTY(Border* border READ border CONSTANT)
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    Q_PROPERTY(DataTransition* transition READ transition WRITE setTransition NOTIFY transitionChanged)
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)

public:
    enum class ColorSource {
        Value,   
        Position 
    };
    Q_ENUM(ColorSource)

    
    explicit BarSeries(QQuickItem* parent = nullptr);
    ~BarSeries() override;

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation orientation);

    qreal barWidth() const;
    void setBarWidth(qreal width);

    qreal barOffset() const;
    void setBarOffset(qreal offset);

    qreal baselineValue() const;
    void setBaselineValue(qreal baselineValue);

    qreal minimumWidth() const;
    void setMinimumWidth(qreal width);

    QColor color() const;
    void setColor(const QColor& color);

    QList<QColor> categoryColors() const;
    void setCategoryColors(const QList<QColor>& colors);

    ColorSource colorSource() const;
    void setColorSource(ColorSource source);

    Border* border() const;

    QColor hoverColor() const;
    void setHoverColor(const QColor& color);

    DataTransition* transition() const;
    void setTransition(DataTransition* transition);

    int hoveredIndex() const;

    Q_INVOKABLE void setData(const QVariantList& bars);

    void setData(std::vector<double>&& data, std::vector<int>&& categories, int barCount);

    void setDataF(std::vector<float>&& data, std::vector<int>&& categories, int barCount);

    using PlotSeries::postData;
    using PlotSeries::setData;
    using PlotSeries::setDataF;

    void postData(std::vector<double>&& data, std::vector<int>&& categories, int barCount);

    void postData(std::vector<float>&& data, std::vector<int>&& categories, int barCount);

    void setRangedData(std::vector<double>&& data, int barCount);
    void setRangedData(std::vector<double>&& data, std::vector<int>&& categories, int barCount);
    void postRangedData(std::vector<double>&& data, int barCount);

    Q_INVOKABLE void clearData() override;
    std::optional<LegendEntry> legendEntry() const override;

    Q_INVOKABLE void setCategories(const QList<int>& categories);

    Q_INVOKABLE QVariantMap barAt(int index) const;

    int barIndexAt(const QPointF& position) const;
    bool contains(const QPointF& point) const override;

signals:
    void orientationChanged();
    void barWidthChanged();
    void barOffsetChanged();
    void baselineValueChanged();
    void minimumWidthChanged();
    void colorChanged();
    void categoryColorsChanged();
    void colorSourceChanged();
    void hoverColorChanged();
    void transitionChanged();
    void hoveredIndexChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
    bool recordInspectionSupported() const override;
    InspectionRecord inspectionRecord(int index) const override;
    InspectionRecord inspectionRecordAt(const QPointF& position) const override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void applyDataPoints(DataPointInput&& input) override;
    int dataPointValueCount() const override;
    void onAxisScaleChanged() override;
    DataRanges computeDataRanges() const override;
    void onDataPointsChanged(DataChange change, int previousCount) override;
    std::optional<ValueSpan> scanValueRange() const override;

private:
    // A set of bars: count bars of stride values each, with one category per bar or none.
    struct Bars {
        std::vector<double> values;
        std::vector<int> categories;
        int count{0};
        int stride{2};
    };

    bool validateDataArguments(std::size_t valueCount, std::size_t categoryCount, int barCount, int barValueCount = 2) const;
    // Switches between (position, value) and (from, to, value) bars; dropping the bars when it changes.
    void setRanged(bool ranged);
    void applyData(std::vector<double>&& data, std::vector<int>&& categories, int barCount);
    void applyRangedData(std::vector<double>&& data, std::vector<int>&& categories, int barCount);
    // Replaces the bars, animating from the drawn ones when \a animate is set.
    void applyBars(std::vector<double>&& data, bool ranged, std::vector<int>&& categories, int barCount, bool animate);
    void applyFloatData(std::vector<float>&& data, std::vector<int>&& categories, int barCount);
    // Replaces the bars with float data, without a transition.
    void applyFloatBars(std::vector<float>&& data, std::vector<int>&& categories, int barCount);
    void setDataFFromArray(const float* data, int barCount);
    // True while transition bars are drawn instead of the data.
    bool animating() const;
    // Returns the stored bars as doubles.
    std::vector<double> storedBars() const;
    // Returns the bars drawn now: the transition bars, or the data.
    Bars drawnBars() const;
    int drawnBarCount() const;
    const std::vector<int>& drawnCategories() const;
    // Returns one category per transition bar: those of the data, then those of \a previous for the bars
    // that shrink away. Empty when no bar has a category.
    std::vector<int> transitionCategories(const std::vector<int>& previous, int transitionBarCount) const;
    // Animates from \a drawn to the data. A bar without a counterpart grows from, or shrinks to, the baseline.
    void startTransition(Bars&& drawn);
    void advanceTransition();
    // Drops the transition bars, so the data is drawn.
    void endTransition();
    // Invalidates what depends on the bar geometry after a barWidth, barOffset, baselineValue, or orientation change.
    void onGeometryChanged();
    bool isHorizontal() const;
    // Number of values per bar: (position, value), or (from, to, value) for ranged bars.
    int valuesPerBar() const;
    // Returns value \a offset of bar \a index, counted within the bar.
    double component(int index, int offset) const;
    double value(int index) const;
    // Returns the extent of bar \a index along the position axis, both NaN when the bar has none.
    std::array<double, 2> positionSpan(int index) const;
    // Returns bar \a index as data-space edges (x1, y1, x2, y2), all NaN when the bar is not drawn.
    std::array<double, 4> barRect(int index) const;
    bool hasCategories() const;
    QColor barColor(int index) const;
    // Tests bar \a index against item position \a position in pixels, widened like the shader draws it.
    bool containsInPixels(int index, const QPointF& position) const;
    void setHoveredIndex(int index);
    QSGNode* releaseNode(QSGNode* oldNode) const;
    // Sets the uniforms for drawing the first \a renderCount of the drawn bars.
    void updateMaterial(BarMaterial& material, int renderCount) const;
    // Uploads the colormap ramp and sets the uniforms that place each bar on it.
    void updateColormapUniforms(BarMaterial& material, QQuickWindow* window) const;
    // True when the colormap colors the bars instead of color and categoryColors.
    bool colormapped() const;
    void ensureSpatialGrid() const;
    // Builds the vertices of the first \a barCount of the drawn bars.
    void buildVertexCache(int barCount);

    Qt::Orientation orientation_{Qt::Vertical};
    qreal barWidth_{0.8};
    qreal barOffset_{0.0};
    qreal baselineValue_{0.0};
    qreal minimumWidth_{1.0};
    QColor color_;
    QList<QColor> categoryColors_;
    ColorSource colorSource_{ColorSource::Value};
    Border* border_{new Border{Qt::black, 0.0, this}};
    QColor hoverColor_;
    int hoveredIndex_{-1};
    std::unique_ptr<TransitionRunner> transition_;
    // Categories of the bars drawn while a transition runs: the new bars, then the removed ones
    // that shrink away. Empty when no bar has one.
    std::vector<int> transitionCategories_;
    // The previous transition frame, reused for the next one.
    std::vector<double> frameBuffer_;
    // Render revision of the bars last uploaded to the GPU.
    std::uint64_t uploadedRevision_{0};
    // Vertex colors are category colors when categories are set; otherwise \c color is a uniform.
    RectVertexCache vertexCache_;
    // Whether the vertex cache holds category colors.
    bool vertexCacheHasCategories_{false};
    // Built on the first hit test after a data or geometry change, so streaming without hover skips it.
    mutable std::vector<double> hitRects_;
    mutable SpatialGrid spatialGrid_;
    mutable bool spatialGridValid_{false};
};

} // namespace QAccelPlot
```


