

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

#include "QAccelPlot/series/PlotSeries.hpp"
#include "QAccelPlot/series/RectVertexCache.hpp"
#include "QAccelPlot/series/RectangleBorder.hpp"
#include "QAccelPlot/series/SpatialGrid.hpp"

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <array>
#include <vector>

namespace QAccelPlot {

class BarMaterial;

class BarSeries : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(BarSeries)

    
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(qreal barWidth READ barWidth WRITE setBarWidth NOTIFY barWidthChanged)
    Q_PROPERTY(qreal barOffset READ barOffset WRITE setBarOffset NOTIFY barOffsetChanged)
    Q_PROPERTY(qreal baselineValue READ baselineValue WRITE setBaselineValue NOTIFY baselineValueChanged)
    Q_PROPERTY(qreal minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumWidthChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QList<QColor> categoryColors READ categoryColors WRITE setCategoryColors NOTIFY categoryColorsChanged)
    Q_PROPERTY(RectangleBorder* border READ border CONSTANT)
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)

public:
    explicit BarSeries(QQuickItem* parent = nullptr);

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

    RectangleBorder* border() const;

    QColor hoverColor() const;
    void setHoverColor(const QColor& color);

    int count() const;
    int hoveredIndex() const;

    Q_INVOKABLE void setData(const QVariantList& bars);

    void setData(const double* data, int barCount) override;

    void setData(std::vector<double>&& data, int barCount) override;

    void setData(std::vector<double>&& data, std::vector<int>&& categories, int barCount);

    void setDataNoRange(const double* data, int barCount) override;
    void setDataNoRange(std::vector<double>&& data, int barCount) override;

    void setDataNoRange(std::vector<double>&& data, std::vector<int>&& categories, int barCount);

    void setDataF(const float* data, int barCount) override;

    void setDataF(std::vector<float>&& data, int barCount) override;

    void setDataF(std::vector<float>&& data, std::vector<int>&& categories, int barCount);

    void setDataFNoRange(const float* data, int barCount) override;
    void setDataFNoRange(std::vector<float>&& data, int barCount) override;

    void setDataFNoRange(std::vector<float>&& data, std::vector<int>&& categories, int barCount);

    void postData(std::vector<double>&& data, int barCount) override;

    void postData(std::vector<double>&& data, std::vector<int>&& categories, int barCount);

    void postData(std::vector<float>&& data, int barCount) override;

    void postData(std::vector<float>&& data, std::vector<int>&& categories, int barCount);

    Q_INVOKABLE void clearData() override;

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
    void hoverColorChanged();
    void countChanged();
    void hoveredIndexChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void onAxisScaleChanged() override;

private:
    bool validateRawDataArguments(const void* data, int barCount) const;
    bool validateDataArguments(std::size_t valueCount, std::size_t categoryCount, int barCount) const;
    void applyData(std::vector<double>&& data, std::vector<int>&& categories, int barCount, bool reportRanges);
    void applyFloatData(std::vector<float>&& data, std::vector<int>&& categories, int barCount, bool reportRanges);
    void setDataFFromArray(const float* data, int barCount, bool reportRanges);
    void finishDataChange(std::vector<int>&& categories, int barCount, bool reportRanges);
    // Invalidates what depends on the bar geometry after a barWidth, barOffset, baselineValue, or orientation change.
    void onGeometryChanged();
    bool isHorizontal() const;
    // True when the double setData() overloads supplied the data; false for the setDataF() overloads.
    bool hasPreciseData() const;
    double position(int index) const;
    double value(int index) const;
    // Returns bar \a index as data-space edges (x1, y1, x2, y2), all NaN when the bar is not drawn.
    std::array<double, 4> barRect(int index) const;
    bool hasCategories() const;
    QColor barColor(int index) const;
    // Tests bar \a index against item position \a position in pixels, widened like the shader draws it.
    bool containsInPixels(int index, const QPointF& position) const;
    void setHoveredIndex(int index);
    QSGNode* releaseNode(QSGNode* oldNode) const;
    void updateMaterial(BarMaterial& material) const;
    void ensureSpatialGrid() const;
    void buildVertexCache();
    void updateDataRanges();
    // Rebuilds renderData_ (origin-relative float coordinates) from the double-precision data_,
    // so the GPU upload stays accurate for large positions without double-precision textures.
    void rebuildRenderData(bool logScalePosition, bool logScaleValue);

    Qt::Orientation orientation_{Qt::Vertical};
    qreal barWidth_{0.8};
    qreal barOffset_{0.0};
    qreal baselineValue_{0.0};
    qreal minimumWidth_{1.0};
    QColor color_;
    QList<QColor> categoryColors_;
    RectangleBorder* border_{new RectangleBorder{this}};
    QColor hoverColor_;
    int hoveredIndex_{-1};
    // Data: 2 doubles per bar (position, value), full precision. Empty when setDataF() supplied the data.
    std::vector<double> data_;
    // One category per bar, or empty when no bar has one.
    std::vector<int> categories_;
    // Uploaded to the GPU: an origin-relative float mirror of data_, or the setDataF() data itself.
    std::vector<float> renderData_;
    qreal renderOriginPosition_{0.0};
    qreal renderOriginValue_{0.0};
    int barCount_{0};
    bool dataChanged_{false};
    // False after the NoRange overloads, so property changes don't overwrite application-managed ranges.
    bool reportRanges_{true};
    // Vertex colors are category colors when categories are set; otherwise \c color is a uniform.
    RectVertexCache vertexCache_;
    // Built on the first hit test after a data or geometry change, so streaming without hover skips it.
    mutable std::vector<double> hitRects_;
    mutable SpatialGrid spatialGrid_;
    mutable bool spatialGridValid_{false};
};

} // namespace QAccelPlot
```


